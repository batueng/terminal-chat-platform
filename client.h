// client.h
#ifndef client_h
#define client_h

#include <string>
#include <signal.h>

const size_t BUFFER_SIZE = 20;

class Client {
public:

  Client(int listening_port_in);

  ~Client();

  int start_client(std::string log_file);

private:

  int client_fd;
  int listening_port;

  int term_width;
  int term_height;

  // Flag to indicate window has been resized
  static volatile sig_atomic_t resized;
  struct sigaction sa;

  // Signal handler for window size
  static void handle_winch(int sig);

  void initiate_ui();
  void handle_ui(char * buffer);

  int handle_user_input(char * buffer);
};

#endif