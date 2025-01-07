#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <sstream>

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

  // TODO: add error checking on username
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
  // Get the current terminal size
  get_terminal_size(term_rows, term_cols);

  // Clear the screen
  clear_screen();

  // Print the session name at the top (centered or just left-aligned)
  // Here, I'm using a centered print:
  print_centered("\033[1;36m" + curr_sess + "\033[0m", term_cols);

  // Leave the rest of the screen mostly empty
  // You can tweak the `term_rows - 4` part to adjust spacing
  for (int i = 0; i < term_rows - 4; ++i) {
    std::cout << std::endl;
  }
  std::string client_message;
  while (true) {
    // Print the prompt at the bottom
    std::cout << "> ";
    std::flush(std::cout);
    std::getline(std::cin, client_message);

    if (client_message == ":leave") {
      boost::unique_lock<boost::mutex> sess_lock(sess_mtx);
      curr_sess.clear();
      sess_cv.notify_all();
      print_home_screen();
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
      std::cout << "hello" << std::endl;
      req_handler.queue_res(*res_hdr, data);
    }
  }
}

void Client::print_messages() {
  std::cout << "\0337";
  std::cout << "\033[H";

  int available_lines = term_rows - 2;

  int messages_size = messages.size();
  int start_index = 0;
  if (messages_size > available_lines) {
    start_index = messages_size - available_lines;
  }

  for (int i = start_index; i < messages_size; ++i) {
    const Message &msg = messages[i];
    std::cout << "\033[K";
    std::cout << msg.username << ": " << msg.text << std::endl;
  }

  int printed_lines = messages_size - start_index;
  for (int i = printed_lines; i < available_lines; ++i) {
    std::cout << "\033[K" << std::endl;
  }

  std::cout << "\0338";
  std::cout.flush();
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
