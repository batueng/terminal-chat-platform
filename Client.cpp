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

void Client::print_session_screen(std::string &session_name) {
  // Get the current terminal size
  get_terminal_size(term_rows, term_cols);

  // Clear the screen
  clear_screen();

  // Print the session name at the top (centered or just left-aligned)
  // Here, I'm using a centered print:
  print_centered("\033[1;36m" + session_name + "\033[0m", term_cols);

  // Leave the rest of the screen mostly empty
  // You can tweak the `term_rows - 4` part to adjust spacing
  for (int i = 0; i < term_rows - 4; ++i) {
    std::cout << std::endl;
  }

  // Print the prompt at the bottom
  std::cout << "> ";
  std::flush(std::cout);
  while (true) {
    messages_mtx.lock();
  }
}

void Client::run() {
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

    if (curr_sess == "") {

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
          req_handler.send_join(arg);
          // Check if success
          print_session_screen(arg); 
        } else if (command == "create") {
          req_handler.send_create(arg);
          // Check if success
          print_session_screen(arg);
        } else if (command == "where") {
          req_handler.send_where(arg);
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
    } else {
      // send chats or guard for leave and exit
      if (line == "leave") {

      } else if (line == "exit") {

      } else {
      }
    }
  }
}

int main(int argc, char *argv[]) {
  std::string server_ip = "127.0.0.1";
  int server_port = 8080;
  Client c(server_ip, server_port);
  c.run();
  return 0;
}
