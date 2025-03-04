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
#include <fstream>
#include <ncurses.h>
#include <string>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

class Client {
public:
  Client(std::string &server_ip, int server_port, std::string debug_file);

  ~Client();

  void run();

  static void handle_signal(int signum);

  static Client *instance;

private:
  friend class RequestHandler;

  WINDOW *login_win;

  WINDOW *home_win;

  WINDOW *messages_win;

  WINDOW *input_win;

  std::string username;

  color c;

  std::string curr_sess;

  boost::mutex sess_mtx;

  boost::mutex msg_mtx;

  boost::condition_variable msg_cv;

  std::deque<Message> messages;

  bool update_msgs = false;

  RequestHandler req_handler;

  int term_rows, term_cols;

  // Use for debugging
  std::ofstream fout;

  void print_login_screen();

  void print_home_screen();

  void print_session_screen();

  void print_messages();

  void msg_update_listener();

  void message_listener();

  void queue_chat(Message msg);

  void on_signal(int signum);
};

#endif
