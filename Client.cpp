#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <sstream>
// #include <termkey.h>
#include <unistd.h>
#include <ncurses.h>

#include "Client.h"
#include "graphics.h"
#include "protocol.h"

Client::Client(std::string &_server_ip, int _server_port)
    : req_handler(_server_ip, _server_port) {}

Client::~Client() {}

void Client::print_login_screen() {
  get_terminal_size(term_rows, term_cols);

  print_header();

  for (int i = 0; i < term_rows - 16; ++i) {
    std::cout << std::endl;
  }

  std::cout << "Enter your username: ";
  std::getline(std::cin, username);

  // TODO: add validation checking on username
  req_handler.send_username(username);
}

void Client::print_home_screen() {
  std::cout << std::endl << std::endl;
  std::cout << "Welcome, " << username << "!\n";

  print_header();
  std::cout << std::endl << std::endl;
  display_help_screen(term_rows, term_cols);
}

void Client::msg_update_listener() {
  while (true) {
    boost::unique_lock<boost::mutex> msg_lock(msg_mtx);

    while (!update_msgs) {
      msg_cv.wait(msg_lock);
    }

    print_messages();
    update_msgs = false;
  }
}

void Client::queue_chat(Message msg) {
  boost::unique_lock<boost::mutex> msg_lock(msg_mtx);
  messages.push_back(msg);
  update_msgs = true;
  msg_cv.notify_one();
}

void Client::print_session_screen() {
  initscr();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  curs_set(1);

  // Seperate the messages and the input window so
  // new messages coming in do not interrupt the input space
  int height;
  int width;
  getmaxyx(stdscr, height, width);

  messages_win = newwin(height - 3, width, 0, 0);
  input_win = newwin(3, width, height - 3, 0);

  scrollok(messages_win, TRUE);
  
  wrefresh(messages_win);
  wrefresh(input_win);

  werase(messages_win);
  box(messages_win, 0, 0);
  wrefresh(messages_win);

  std::string session_title = "\033[1;36m" + curr_sess + "\033[0m";
  int msg_width = getmaxx(messages_win);
  mvwprintw(messages_win, 1, (msg_width - session_title.length()) / 2, "%s", session_title.c_str());
  wrefresh(messages_win);

  std::string client_message;
  while (true) {
    werase(input_win);
    box(input_win, 0, 0);
    mvwprintw(input_win, 1, 2, "> ");
    wrefresh(input_win);

    echo();
    
    wmove(input_win, 1, 4);

    char input_buffer[1024];
    wgetnstr(input_win, input_buffer, sizeof(input_buffer) - 1);
    client_message = std::string(input_buffer);
    
    noecho();

    if (client_message == ":leave") {
      boost::unique_lock<boost::mutex> sess_lock(sess_mtx);
      curr_sess.clear();
      sess_cv.notify_all();
      print_home_screen();

      // ncurses cleanup
      delwin(messages_win);
      delwin(input_win);
      endwin();
      return;
    }

    Message msg = {message_type::CHAT, username, client_message};
    req_handler.send_message(username, curr_sess, msg);
    queue_chat(msg);
  }
}

void Client::message_listener() {
  while (true) {
    std::string res_hdr_str =
        req_handler.client_sock.recv_len(sizeof(tcp_hdr_t));
    tcp_hdr_t *res_hdr = reinterpret_cast<tcp_hdr_t *>(res_hdr_str.data());

    std::string data = req_handler.client_sock.recv_len(res_hdr->data_len);

    if (res_hdr->method == tcp_method::MESSAGE) {
      const std::vector<char> recv_msg =
          std::vector<char>(data.begin(), data.end());
      Message msg;
      msg = Message::deserialize_message(recv_msg);

      queue_chat(msg);
    } else {
      req_handler.queue_res(*res_hdr, data);
    }
  }
}

void Client::print_messages() {
  boost::unique_lock<boost::mutex> lock(cout_mtx);

  werase(messages_win);
  box(messages_win, 0, 0);

  int msg_width = getmaxx(messages_win);
  mvwprintw(messages_win, 1, (msg_width - curr_sess.length()) / 2, "%s", curr_sess.c_str());

  int y = 3;
  for (const auto& msg : messages) {
    if (y >= getmaxy(messages_win) - 1) { // Prevent writing outside the window
      break;
    }
    mvwprintw(messages_win, y, 2, "%s: ", msg.username.c_str());
    mvwprintw(messages_win, y, 2 + msg.username.length() + 2, "%s", msg.text.c_str());
    y++;
  }

  // Refresh the messages window to show updates
  wrefresh(messages_win);
}

void Client::run() {
  boost::thread updt_listener(&Client::msg_update_listener, this);
  boost::thread msg_listener(&Client::message_listener, this);

  print_login_screen();
  print_home_screen();

  std::string line, command;
  while (true) {
    get_terminal_size(term_rows, term_cols);

    for (int i = 0; i < term_rows - 28; ++i) {
      std::cout << std::endl;
    }
    std::cout << "> ";
    std::getline(std::cin, line);
    boost::trim(line);

    std::istringstream stream(line);

    stream >> command;
    std::string pattern =
        "^\\s*" + command + "\\s+" + "[\\x20-\\x7E]{1," +
        std::to_string(command == "where" ? MAX_USERNAME - 1
                                          : MAX_SESSION_NAME - 1) +
        "}$";

    if (command == "join" || command == "create" || command == "where") {
      // valid pattern
      if (!boost::regex_match(line, boost::regex(pattern))) {
        std::cout << "Error: Invalid command. See help for proper format."
                  << std::endl;
        continue;
      }

      // get session_name/username argument
      std::string arg;
      stream >> arg;

      // send join/create/where
      if (command == "join") {
        req_handler.send_join(username, arg);
        {
          boost::unique_lock<boost::mutex> sess_lock(sess_mtx);
          curr_sess = arg;
          sess_cv.notify_all();
        }
        print_session_screen();

      } else if (command == "create") {
        req_handler.send_create(username, arg);
        {
          boost::unique_lock<boost::mutex> sess_lock(sess_mtx);
          curr_sess = arg;
          sess_cv.notify_all();
        }
        print_session_screen();

      } else if (command == "where") {
        req_handler.send_where(username, arg);
      }
      // recv success
      // set session_name/username

    } else if (line == "help") {

    } else if (line == "exit") {
      std::cout << "Exiting application. Goodbye!\n";
      break;
    } else {
      std::cout << "Unknown command" << std::endl;
    }
  }
  updt_listener.join();
  msg_listener.join();
}

int main(int argc, char *argv[]) {
  std::string server_ip = "127.0.0.1";
  int server_port = 8080;
  Client c(server_ip, server_port);
  c.run();
  return 0;
}
