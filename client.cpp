#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <sstream>
#include <ncurses.h>
#include <signal.h>

#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

#include "client.h"

volatile sig_atomic_t Client::resized = false;

Client::Client(int listenting_port_in) : listening_port(listenting_port_in) {}

int Client::start_client(std::string log_file) {
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
  }
  return 0;
}

Client::~Client() {
  endwin();
}

int main() {
  Client client(1600);
  client.start_client("log.txt");

  return 0;
}