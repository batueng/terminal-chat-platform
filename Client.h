// Client.h
#ifndef client_h
#define client_h

#include "RequestHandler.h"

#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <cstddef>
#include <cstdlib>
#include <deque>
#include <ncurses.h>
#include <string>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

class Client {
public:
  Client(std::string &server_ip, int server_port);

  ~Client();

  void run();

private:
  friend class RequestHandler;

  // should implement this to allow terminal resizing
  // // Flag to indicate window has been resized
  // static volatile sig_atomic_t resized;
  // struct sigaction sa;
  //
  // // Signal handler for window size
  // static void handle_winch(int sig);

  // ncurses windows to manage messages and input space
  WINDOW *messages_win;

  WINDOW *input_win;

  std::string username;

  color c;

  std::string curr_sess;

  boost::mutex sess_mtx;

  boost::condition_variable sess_cv;

  boost::mutex msg_mtx;

  boost::condition_variable msg_cv;

  std::deque<Message> messages;

  bool update_msgs = false;

  RequestHandler req_handler;

  int term_rows, term_cols;

  void print_login_screen();

  void print_home_screen();

  void print_session_screen();

  void print_messages();

  void msg_update_listener();

  void message_listener();

  void queue_chat(Message msg);
};

#endif
