// Client.h
#ifndef client_h
#define client_h

#include "ClientSocket.h"
#include "RequestHandler.h"
#include "Session.h"

#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include <cstddef>
#include <cstdlib>
#include <deque>
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

  std::string username;

  std::string curr_sess;

  boost::mutex msg_mtx;

  std::deque<Message> messages;

  RequestHandler req_handler;

  int term_rows, term_cols;

  void print_login_screen();

  void print_home_screen();

  void print_session_screen(std::string &session_name);

  void print_messages();

  void messages_listen();
};

#endif
