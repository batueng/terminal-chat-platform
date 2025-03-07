#include <atomic>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <csignal>
#include <fstream>
#include <iostream>
#include <ncurses.h>
#include <sstream>
#include <unistd.h>

#include "Client.h"
#include "graphics.h"
#include "protocol.h"

std::atomic<bool> g_running{true};
Client *Client::instance = nullptr;

Client::Client(std::string &_server_ip, int _server_port,
               std::string debug_file)
    : req_handler(_server_ip, _server_port),
      fout(std::to_string((unsigned long long)(void **)this) + "_" +
           debug_file) {
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

Client::~Client() { endwin(); }

void Client::handle_signal(int signum) {
  if (instance) {
    instance->on_signal(signum);
  }
}

void Client::on_signal(int signum) {
  g_running = false;

  boost::unique_lock<boost::mutex> lock(sess_mtx);
  if (curr_sess != "") {
    req_handler.send_leave(username, curr_sess);
  }
  req_handler.send_shutdown(username);

  msg_cv.notify_all();

  int fd = req_handler.client_sock.fd;
  if (fd != -1) {
    close(fd);
  }

  flushinp();
  ungetch('\n');
}

void Client::print_login_screen() {
  int height, width;
  getmaxyx(stdscr, height, width);

  login_win = newwin(height, width, 0, 0);
  keypad(login_win, TRUE);
  wtimeout(login_win, 100);
  print_header(login_win);
  mvwprintw(login_win, height - 1, 1, "Enter your username: ");
  wrefresh(login_win);

  cbreak();
  noecho();
  curs_set(1);

  int prompt_x = 22;
  int ch;
  bool valid_username = false;

  while (g_running && !valid_username) {
    username.clear();
    redraw_prompt(login_win, height, prompt_x, username);

    while (g_running) {
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
      } else if (ch >= 32 && ch <= 126) {
        username.push_back(ch);
        redraw_prompt(login_win, height, prompt_x, username);
      }
    }

    if (!g_running)
      break;

    std::string err_msg;
    if (req_handler.send_username(username, err_msg) == tcp_status::SUCCESS)
      valid_username = true;
    else {
      print_error_message(login_win, height, width, err_msg);
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
  keypad(home_win, TRUE);
  wtimeout(home_win, 100);
  cbreak();
  noecho();
  curs_set(1);

  int header_height = 15;
  redraw_home_screen(home_win, height, width, header_height, username);

  int prompt_x = 3;
  std::string line, command, arg;
  int ch;

  while (g_running) {
    mvwprintw(home_win, height - 2, 1, "%*s", width - 2, " ");
    wrefresh(home_win);

    line.clear();

    while (g_running) {
      mvwprintw(home_win, height - 1, 1, "> ");
      wmove(home_win, height - 1, prompt_x);
      wclrtoeol(home_win);
      wrefresh(home_win);
      mvwprintw(home_win, height - 1, prompt_x, "%s", line.c_str());
      wmove(home_win, height - 1, prompt_x + static_cast<int>(line.size()));
      wrefresh(home_win);

      ch = wgetch(home_win);
      if (ch == KEY_RESIZE) {
        handle_resize(home_win, height, width);
        redraw_home_screen(home_win, height, width, header_height, username);
        mvwprintw(home_win, height - 2, 1, "%*s", width - 2, " ");
        continue;
      }
      if (ch == KEY_BACKSPACE || ch == 127 || ch == '\b') {
        if (!line.empty())
          line.pop_back();
      } else if (ch == '\n') {
        break;
      } else if (ch >= 32 && ch <= 126) {
        line.push_back(ch);
      }
    }

    if (!g_running)
      break;

    boost::trim(line);
    if (line.empty())
      continue;

    std::istringstream stream(line);
    stream >> command;

    if (command == "join" || command == "create" || command == "where") {
      std::string pattern =
          "^\\s*" + command + "\\s+" + "[\\x20-\\x7E]{1," +
          std::to_string(command == "where" ? MAX_USERNAME - 1
                                            : MAX_SESSION_NAME - 1) +
          "}$";
      if (!boost::regex_match(line, boost::regex(pattern))) {
        mvwprintw(home_win, height - 2, 1,
                  "Error: Invalid command. See help for proper format.");
        wrefresh(home_win);
        continue;
      }
      stream >> arg;
      if (command == "join") {
        std::string err_msg;
        auto [_c, status] = req_handler.send_join(username, arg, err_msg);
        if (status != tcp_status::SUCCESS) {
          print_error_message(home_win, height, width, err_msg);
        } else {
          {
            boost::unique_lock<boost::mutex> lock(sess_mtx);
            c = _c;
            curr_sess = arg;
          }

          delwin(home_win);
          clear();
          refresh();
          print_session_screen();
        }
      } else if (command == "create") {
        std::string err_msg;
        auto [_c, status] = req_handler.send_create(username, arg, err_msg);
        if (status != tcp_status::SUCCESS) {
          print_error_message(home_win, height, width, err_msg);
        } else {
          {
            boost::unique_lock<boost::mutex> lock(sess_mtx);
            c = _c;
            curr_sess = arg;
            fout << curr_sess << std::endl;
          }

          delwin(home_win);
          clear();
          refresh();
          print_session_screen();
        }
      } else if (command == "where") {
        std::string err_msg;
        auto [user_loc, status] =
            req_handler.send_where(username, arg, err_msg);
        if (status != tcp_status::SUCCESS) {
          print_error_message(home_win, height, width, err_msg);
        } else {
          std::string loc_msg = arg + " is in session " + user_loc;
          print_error_message(home_win, height, width, loc_msg);
        }
      }
    } else if (command == "help") {
    } else if (command == "exit") {
    } else {
      std::string err_msg = "Unknown command.";
      print_error_message(home_win, height, width, err_msg);
    }
  }
  delwin(home_win);
}

void Client::print_error_message(WINDOW *win, int height, int width,
                                 std::string &err_msg) {
  mvwprintw(win, height - 2, 1, "%s", err_msg.c_str());
  wclrtoeol(win);
  wrefresh(win);
  wgetch(win);
  mvwprintw(win, height - 2, 1, "%*s", width - 2, " ");
}

void Client::msg_update_listener() {
  while (g_running) {
    boost::unique_lock<boost::mutex> msg_lock(msg_mtx);

    while (g_running && !update_msgs) {
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

  messages_win = newwin(height - 2, width, 0, 0);
  input_win = newwin(2, width, height - 2, 0);

  keypad(input_win, TRUE);
  wtimeout(input_win, 100);

  scrollok(messages_win, TRUE);

  werase(messages_win);
  wrefresh(messages_win);

  werase(input_win);
  wrefresh(input_win);

  int msg_width = getmaxx(messages_win);

  std::string user_string = "User: ";
  mvwprintw(messages_win, 0, 1, user_string.c_str());

  wattron(messages_win, COLOR_PAIR(c));
  mvwprintw(messages_win, 0, user_string.size() + 1, username.c_str());
  wattroff(messages_win, COLOR_PAIR(c));

  wattron(messages_win, COLOR_PAIR(1) | A_BOLD);

  int start_col = 0;
  if (msg_width > static_cast<int>(curr_sess.size())) {
    start_col = (msg_width - curr_sess.size()) / 2;
  }
  mvwprintw(messages_win, 1, start_col, "%s", curr_sess.c_str());

  wattroff(messages_win, COLOR_PAIR(1) | A_BOLD);
  wrefresh(messages_win);

  std::string client_message;
  int prompt_x = 3;
  while (g_running) {
    {
      boost::unique_lock<boost::mutex> lock(win_mtx);
      werase(input_win);
      redraw_session_prompt(input_win, prompt_x, "");
    }
    std::string line;
    int ch;

    while (g_running) {
      {
        boost::unique_lock<boost::mutex> lock(win_mtx);
        redraw_session_prompt(input_win, prompt_x, line);
      }
      ch = wgetch(input_win);

      if (ch == KEY_RESIZE) {
        {
          boost::unique_lock<boost::mutex> lock(win_mtx);
          handle_session_resize(messages_win, input_win, height, width, 15,
                                curr_sess, username, c);
          print_messages();
        }
        continue;
      }

      if (ch == KEY_BACKSPACE || ch == 127 || ch == '\b') {
        if (!line.empty())
          line.pop_back();
      } else if (ch == '\n') {
        break;
      } else if (ch >= 32 && ch <= 126) {
        line.push_back(ch);
      }
    }

    client_message = line;

    if (client_message == ":leave") {
      req_handler.send_leave(username, curr_sess);
      {
        boost::unique_lock<boost::mutex> msg_lock(msg_mtx);
        messages.clear();
        curr_sess.clear();
      }

      {
        boost::unique_lock<boost::mutex> lock(win_mtx);
        print_home_screen();
        delwin(messages_win);
        delwin(input_win);
      }
      return;
    }

    Message msg = {msg_type::CHAT, username, c, client_message};

    req_handler.send_message(username, curr_sess, msg);
    queue_chat(msg);
  }
}

void Client::message_listener() {
  while (g_running) {
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
  if (!messages_win)
    return;

  boost::unique_lock<boost::mutex> lock(win_mtx);

  int in_y = 0, in_x = 0;
  if (input_win) {
    getyx(input_win, in_y, in_x);
  }

  werase(messages_win);

  int win_width = getmaxx(messages_win);
  int win_height = getmaxy(messages_win);
  int max_msg_width = win_width / 2;

  std::string user_string = "User: ";
  mvwprintw(messages_win, 0, 1, user_string.c_str());

  wattron(messages_win, COLOR_PAIR(c));
  mvwprintw(messages_win, 0, user_string.size() + 1, username.c_str());
  wattroff(messages_win, COLOR_PAIR(c));

  wattron(messages_win, COLOR_PAIR(1) | A_BOLD);
  mvwprintw(messages_win, 1, (win_width - curr_sess.size()) / 2, "%s",
            curr_sess.c_str());
  wattroff(messages_win, COLOR_PAIR(1) | A_BOLD);

  int y = 3;
  std::string prev_sender = "";

  for (const auto &msg : messages) {
    if (y >= win_height)
      break;

    if (msg.msg_t == msg_type::CHAT) {
      bool is_self = (msg.username == username);
      color msg_color = is_self ? c : msg.color;

      if (msg.username != prev_sender) {
        ++y;
        wattron(messages_win, COLOR_PAIR(msg_color));

        if (is_self) {
          int name_x = win_width - msg.username.size() - 2;
          mvwprintw(messages_win, y++, name_x, "%s", msg.username.c_str());
        } else {
        }

        wattroff(messages_win, COLOR_PAIR(msg_color));
        prev_sender = msg.username;
      }

      std::string text = msg.text;
      std::vector<std::string> lines;
      std::string word, current_line;
      std::istringstream stream(text);

      while (stream >> word) {
        if (current_line.size() + word.size() +
                (current_line.empty() ? 0 : 1) <=
            max_msg_width) {
          if (!current_line.empty())
            current_line += " ";
          current_line += word;
        } else {
          lines.push_back(current_line);
          current_line = word;
        }
      }
      if (!current_line.empty())
        lines.push_back(current_line);

      for (const auto &line : lines) {
        if (y >= win_height)
          break;

        if (is_self) {
          int line_x = win_width - line.size() - 2;
          mvwprintw(messages_win, y++, line_x, "%s", line.c_str());
        } else {
          mvwprintw(messages_win, y++, 1, "%s", line.c_str());
        }
      }

    } else {
      ++y;
      print_centered(messages_win, y++, win_width, msg.text);
    }
  }

  wrefresh(messages_win);

  if (input_win) {
    wmove(input_win, in_y, in_x);
    wrefresh(input_win);
  }
}

void Client::run() {
  boost::thread updt_listener(&Client::msg_update_listener, this);
  boost::thread msg_listener(&Client::message_listener, this);

  print_login_screen();
  print_home_screen();

  updt_listener.join();
  msg_listener.join();
}

int main(int argc, char *argv[]) {

  std::string server_ip = "127.0.0.1";
  int server_port = 8080;
  Client c(server_ip, server_port, "out.txt");

  Client::instance = &c;
  std::signal(SIGINT, Client::handle_signal);

  c.run();
  return 0;
}
