#include "Client.h"
#include "graphics.h"
#include "protocol.h"

Client::Client(std::string &_server_ip, int _server_port)
    : server_ip(_server_ip), server_port(_server_port),
      client_sock(server_ip, server_port) {}

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
  tcp_hdr_t uname_hdr = {tcp_method::U_NAME, 0};
  std::memcpy(uname_hdr.user_name, username.c_str(), username.size());
  uname_hdr.user_name[username.size()] = '\0';
  client_sock.send_len(&uname_hdr, sizeof(tcp_hdr_t));
}

void Client::print_home_screen() {
  std::cout << std::endl << std::endl;
  std::cout << "Welcome, " << username << "!\n";

  print_header();
  std::cout << std::endl << std::endl;
  display_help_screen(term_rows, term_cols);
}

void Client::run() {
  print_login_screen();
  print_home_screen();

  std::string command;
  while (true) {
    get_terminal_size(term_rows, term_cols);

    for (int i = 0; i < term_rows - 28; ++i) {
      std::cout << std::endl;
    }
    std::cout << ">";
    std::getline(std::cin, command);

    if (command == "exit") {
      std::cout << "Exiting application. Goodbye!\n";
      break;
    } else {
      std::cout << "Command received: " << command << "\n";
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
