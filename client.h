// client.h
#ifndef client_h
#define client_h

#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <signal.h>
#include <string>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

const size_t BUFFER_SIZE = 20;

class Client {
public:
  Client(std::string &server_ip, int server_port);

  ~Client();

  void run();

private:
  // should implement this to allow terminal resizing
  // // Flag to indicate window has been resized
  // static volatile sig_atomic_t resized;
  // struct sigaction sa;
  //
  // // Signal handler for window size
  // static void handle_winch(int sig);

  std::string &server_ip;

  int server_port;

  std::string username;

  int term_rows, term_cols;

  void print_login_screen();

  void print_home_screen();
};

#endif
