#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <sstream>
#include <ncurses.h>
#include <signal.h>
#include <fstream>

#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

#include "client.h"

volatile sig_atomic_t Client::resized = false;

Client::Client(std::string& server_ip_in, int server_port_in, std::string& log_file) : server_ip(server_ip_in), server_port(server_port_in) {
  fout = std::ofstream(log_file);
}

int Client::start_client(int listening_port) {
    char buffer[BUFFER_SIZE];
    initiate_ui();
    while (true) {
        memset(buffer, '\0', BUFFER_SIZE);
        handle_ui(buffer);

        if (handle_user_input(buffer) == -1) {
          return -1;
        }
    }
}

int Client::connect_to_server() {
  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  
  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(server_port);

  if (inet_pton(AF_INET, server_ip.data(), &server_addr.sin_addr) < 0) {
    fout << "Invalid server IP address" << std::endl;
    close(server_fd);
    exit(1);
  }

  if (connect(server_fd, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
    fout << "Error connecting to server" << std::endl;
    exit(1);
  }

  fout << "Connected to the server" << std::endl;
  return 0;
}

void Client::handle_winch(int sig) {
    resized = true;
}

void Client::initiate_ui() {
  // Initialize the screen
  initscr();
  raw(); // No signals from control keys
  curs_set(1);
  keypad(stdscr, TRUE); // Enable keypad keys to signal

  // Set up the signal handler for window resizing
  sa.sa_handler = handle_winch;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART; // Restart functions if interrupted by handler
  sigaction(SIGWINCH, &sa, NULL);

  // Initial window size
  getmaxyx(stdscr, term_height, term_width);
  move(term_height - 1, 0);
  refresh();
}

void Client::handle_ui(char * buffer) {
  if (resized) {
    resized = false;

    // Handle the resize
    endwin();
    refresh();

    // Get the new window size
    getmaxyx(stdscr, term_width, term_height);
  }

  // Move the cursor to the new bottom-left corner
  move(term_height - 1, 0);

  // Only get input up to buffer size
  getnstr(buffer, BUFFER_SIZE-1);

  clrtoeol();
  refresh();
}

int Client::handle_user_input(char * buffer) {
  if (!strcmp(buffer, ":q")) {
    return -1;
  } else if (!strcmp(buffer, ":start")) {
    connect_to_server();
  }
  return 0;
}

Client::~Client() {
  endwin();
}

int main() {
  std::string log_file = "log.txt";
  std::string server_ip = "127.0.0.1";
  Client client(server_ip, 1600, log_file);
  client.start_client(1800);

  return 0;
}