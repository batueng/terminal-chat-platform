#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <ncurses.h>
#include <csignal>
#include <sstream>
#include <unistd.h>
#include <fstream>

#include "Client.h"
#include "graphics.h"
#include "protocol.h"


Client::Client(std::string &_server_ip, int _server_port, std::string debug_file)
    : req_handler(_server_ip, _server_port), fout(debug_file) {
  initscr();
  use_default_colors();
  start_color();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  curs_set(1);

  for (uint8_t i = static_cast<uint8_t>(color::RED);
       i < static_cast<uint8_t>(color::END); ++i) {
    init_pair(i, i, -1);
  }
 }

Client::~Client() {
  endwin();
}

void Client::print_login_screen() {
  int height, width;
  getmaxyx(stdscr, height, width);

  login_win = newwin(height, width, 0, 0);
  keypad(login_win, TRUE);
  print_header(login_win);
  mvwprintw(login_win, height - 1, 1, "Enter your username: ");
  wrefresh(login_win);

  cbreak();
  noecho();
  curs_set(1);

  int prompt_x = 22;
  std::string username;
  int ch;
  bool valid_username = false;

  while (!valid_username) {
    username.clear();
    redraw_prompt(login_win, height, prompt_x, username);

    while (true) {
      ch = wgetch(login_win);

      if (ch == KEY_RESIZE) {
        handle_resize(login_win, height, width);
        print_header(login_win);
        redraw_prompt(login_win, height, prompt_x, username);
        continue;
      }

      if (ch == '\n')
        break;
      else if (ch == KEY_BACKSPACE || ch == 127 || ch == '\b') {
        if (!username.empty()) {
          username.pop_back();
          redraw_prompt(login_win, height, prompt_x, username);
        }
      }
      else if (ch >= 32 && ch <= 126) {
        username.push_back(ch);
        redraw_prompt(login_win, height, prompt_x, username);
      }
    }

    std::string err_msg;
    if (req_handler.send_username(username, err_msg) == tcp_status::SUCCESS)
      valid_username = true;
    else {
      mvwprintw(login_win, height - 2, 1, "%s", err_msg.c_str());
      wclrtoeol(login_win);
      wrefresh(login_win);
      wgetch(login_win);
      mvwprintw(login_win, height - 2, 1, "%*s", width - 2, " ");
      redraw_prompt(login_win, height, prompt_x, "");
    }
  }

  noecho();
  curs_set(0);
  delwin(login_win);
}

void Client::print_home_screen() {
  int height, width;
  getmaxyx(stdscr, height, width);

  home_win = newwin(height, width, 0, 0);

  std::string welcome_text = "Welcome, " + username + "!";
  mvwprintw(home_win, 1, 1, welcome_text.c_str());
  wrefresh(home_win);

  print_header(home_win);

  int header_height = 15;
  WINDOW* help_win = derwin(home_win, height - header_height, width, header_height, 0);
  display_help_screen(help_win);
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
  int height, width;
  getmaxyx(stdscr, height, width);

  // Create the windows without borders.
  messages_win = newwin(height - 2, width, 0, 0);
  input_win = newwin(2, width, height - 2, 0);

  scrollok(messages_win, TRUE);

  werase(messages_win);
  wrefresh(messages_win);
  werase(input_win);
  wrefresh(input_win);

  // Print the session title centered on row 1.
  std::string session_title = curr_sess;
  int msg_width = getmaxx(messages_win);
  wattron(messages_win, COLOR_PAIR(1) | A_BOLD);
  mvwprintw(messages_win, 1, (msg_width - curr_sess.size()) / 2, "%s",
            curr_sess.c_str());
  wattroff(messages_win, COLOR_PAIR(1) | A_BOLD);
  wrefresh(messages_win);

  std::string client_message;
  while (true) {
    werase(input_win);
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

      // Clean up the windows.
      delwin(messages_win);
      delwin(input_win);
      return;
    }

    Message msg = {message_type::CHAT, username, c, client_message};
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

  int win_width = getmaxx(messages_win);
  int win_height = getmaxy(messages_win);
  int interiorWidth = win_width;

  wattron(messages_win, COLOR_PAIR(1) | A_BOLD);
  mvwprintw(messages_win, 1, (win_width - curr_sess.size()) / 2, "%s",
            curr_sess.c_str());
  wattroff(messages_win, COLOR_PAIR(1) | A_BOLD);

  int y = 3;

  std::string prev_sender = "";
  for (const auto &msg : messages) {
    if (y >= win_height)
      break;

    if (msg.username == username) {
      std::string text = msg.text;
      int available_self = interiorWidth - 1;

      if ((int)text.size() > available_self)
        text = text.substr(text.size() - available_self);

      int textLen = text.size();
      int x = (interiorWidth - 1) - textLen;

      mvwprintw(messages_win, y, x, "%s", text.c_str());
      y++;
      prev_sender = username;

    } else {
      int available_rec = interiorWidth - 1;

      if (msg.username != prev_sender) {
        wattron(messages_win, COLOR_PAIR(static_cast<uint8_t>(msg.color)));
        mvwprintw(messages_win, y, 1, "%s", msg.username.c_str());
        wattroff(messages_win, COLOR_PAIR(static_cast<uint8_t>(msg.color)));

        mvwprintw(messages_win, y, 1 + msg.username.size(), ": %s",
                  msg.text.c_str());

        y++;
        prev_sender = msg.username;

      } else {
        std::string text = msg.text;
        if ((int)text.size() > available_rec)
          text = text.substr(0, available_rec);

        mvwprintw(messages_win, y, 1, "%s", text.c_str());
        y++;
      }
    }
  }

  wrefresh(messages_win);
  wmove(input_win, 1, 4);
  wrefresh(input_win);
}

void Client::run() {
  boost::thread updt_listener(&Client::msg_update_listener, this);
  boost::thread msg_listener(&Client::message_listener, this);

  print_login_screen();
  print_home_screen();

  std::string line, command;
  while (true) {
    int height, width;
    getmaxyx(stdscr, height, width);

    mvwprintw(home_win, height - 1, 1, "> ");
    wrefresh(home_win);

    echo();

    char line_in[1024];
    wgetnstr(home_win, line_in, 1024 - 1);

    noecho();

    line = std::string(line_in);

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
        mvwprintw(home_win, height - 2, 1, "Error: Invalid command. See help for proper format.");
        wrefresh(home_win);
        continue;
      }

      // get session_name/username argument
      std::string arg;
      stream >> arg;

      // send join/create/where
      if (command == "join") {
        c = req_handler.send_join(username, arg);
        {
          boost::unique_lock<boost::mutex> sess_lock(sess_mtx);
          curr_sess = arg;
          sess_cv.notify_all();
        }
        delwin(home_win);
        print_session_screen();

      } else if (command == "create") {
        c = req_handler.send_create(username, arg);
        {
          boost::unique_lock<boost::mutex> sess_lock(sess_mtx);
          curr_sess = arg;
          sess_cv.notify_all();
        }
        delwin(home_win);
        print_session_screen();

      } else if (command == "where") {
        req_handler.send_where(username, arg);
      }
      // recv success
      // set session_name/username

    } else if (line == "help") {

    } else if (line == "exit") {
      mvwprintw(home_win, height - 2, 1, "Exiting application. Goodbye!");
      break;
    } else {
      mvwprintw(home_win, height - 2, 1, "Unknown command");
    }
  }
  updt_listener.join();
  msg_listener.join();
}

int main(int argc, char *argv[]) {
  std::string server_ip = "127.0.0.1";
  int server_port = 8080;
  Client c(server_ip, server_port, "out.txt");
  c.run();
  return 0;
}
