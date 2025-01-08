#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <sstream>

#include "Client.h"
#include "graphics.h"
#include "ncurses.h"
#include "protocol.h"

void handle_resize(int sig) {
  endwin();  // End current ncurses session
  refresh(); // Refresh screen
  clear();   // Clear screen
}

Client::Client(std::string &_server_ip, int _server_port)
    : req_handler(_server_ip, _server_port) {
  initscr();            // Initialize ncurses
  cbreak();             // Disable line buffering
  noecho();             // Disable echoing typed characters
  keypad(stdscr, TRUE); // Enable special keys

  if (has_colors()) {
    use_default_colors();
    start_color();

    init_pair(1, COLOR_BLUE, -1);
    init_pair(2, COLOR_CYAN, -1);
    init_pair(3, COLOR_GREEN, -1);
    init_pair(4, COLOR_YELLOW, -1);

    clear();
    refresh();

  } else {
    endwin();
    throw std::runtime_error("Your terminal does not support colors.");
  }

  signal(SIGWINCH, handle_resize); // Handle terminal resize
}

Client::~Client() { endwin(); }

void Client::print_login_screen() {
  getmaxyx(stdscr, term_rows, term_cols); // Get terminal dimensions

  // Print the header at the top
  print_header();

  // Prompt for username at the bottom, left-aligned
  attron(COLOR_PAIR(3)); // Use green for the input prompt
  std::string uname_prompt = "Enter your username: ";
  mvprintw(term_rows - 1, 0, uname_prompt.c_str());
  attroff(COLOR_PAIR(3));

  refresh();

  // Enable echo for username input and get the input
  echo();
  char username_buf[MAX_USERNAME];
  mvgetstr(term_rows - 1, uname_prompt.size(),
           username_buf); // Place input directly below prompt
  noecho();

  username =
      std::string(username_buf); // Store the input in the username variable

  // Send the username to the server
  req_handler.send_username(username);

  refresh();
}

void Client::print_home_screen() {
  clear_screen();

  getmaxyx(stdscr, term_rows, term_cols);

  print_header();
  mvprintw(9, 0, "Debug: Header printed");
  refresh();

  int help_start_row = 10;
  display_help_screen(help_start_row, term_cols);

  mvprintw(9, 0, "Debug: Header printed");
  refresh();
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
  clear_screen();

  // Get the current terminal size
  getmaxyx(stdscr, term_rows, term_cols);

  WINDOW *msg_win = newwin(term_rows - 3, term_cols, 0, 0);   // Message area
  WINDOW *input_win = newwin(3, term_cols, term_rows - 3, 0); // Input area
  scrollok(msg_win, TRUE);
  wrefresh(msg_win);

  std::string client_message;

  while (true) {
    dynamic_multi_line_input(input_win, client_message,
                             3); // Allow dynamic input

    if (client_message == ":leave") {
      delwin(msg_win);
      delwin(input_win);
      boost::unique_lock<boost::mutex> sess_lock(sess_mtx);
      curr_sess.clear();
      sess_cv.notify_all();
      print_home_screen();
      return;
    }

    Message msg = {message_type::CHAT, username, client_message};
    req_handler.send_message(username, curr_sess, msg);
    queue_chat(msg);

    // Print the new message in the message window
    wprintw(msg_win, "%s: %s\n", username.c_str(), client_message.c_str());
    wrefresh(msg_win);
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
  getmaxyx(stdscr, term_rows, term_cols);

  WINDOW *msg_win = newwin(term_rows - 3, term_cols, 0, 0); // Message area
  box(msg_win, 0, 0);
  scrollok(msg_win, TRUE);

  int start_index =
      messages.size() > term_rows - 4 ? messages.size() - (term_rows - 4) : 0;
  for (int i = start_index; i < messages.size(); ++i) {
    const Message &msg = messages[i];
    wprintw(msg_win, "%s: %s\n", msg.username.c_str(), msg.text.c_str());
  }

  wrefresh(msg_win);
  delwin(msg_win);
}

void Client::run() {
  boost::thread updt_listener(&Client::msg_update_listener, this);
  boost::thread msg_listener(&Client::message_listener, this);

  print_login_screen();
  print_home_screen();

  int rows, cols;
  getmaxyx(stdscr, rows, cols);

  WINDOW *main_win = newwin(rows - 3, cols, 0, 0);  // Main display area
  WINDOW *input_win = newwin(3, cols, rows - 3, 0); // Input area
  scrollok(main_win, TRUE);
  wrefresh(main_win);

  std::string line, command;
  while (true) {
    getmaxyx(stdscr, term_rows, term_cols);

    dynamic_multi_line_input(input_win, line, 3); // Capture input

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
      print_home_screen();
    } else if (line == "exit") {
      wprintw(main_win, "Exiting application. Goodbye!\n");
      wrefresh(main_win);
      break;
    } else {
      wprintw(main_win,
              "Unknown command. Please use help for proper TCP usage.");
      wrefresh(main_win);
    }
    line.clear();
  }

  updt_listener.join();
  msg_listener.join();
  delwin(main_win);
  delwin(input_win);
}

int main(int argc, char *argv[]) {
  std::string server_ip = "127.0.0.1";
  int server_port = 8080;
  Client c(server_ip, server_port);
  c.run();
  return 0;
}
