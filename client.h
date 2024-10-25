// client.h
#ifndef client_h
#define client_h

#include <string>
#include <signal.h>

const size_t BUFFER_SIZE = 20;

class Client {
public:

  Client(std::string& server_ip, int server_port, std::string& log_file);

  ~Client();

  int start_client(int listening_port);

private:

  std::ofstream fout;
  int server_fd;
  std::string server_ip;
  int server_port;
  int client_fd;

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

  int connect_to_server();

};

#endif