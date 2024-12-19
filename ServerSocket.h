/*
 * ServerSocket class represents the servers main socket
 * which accepts connections from clients
 *
 * Implements RAII by opening, binding and listening on construction
 * Closes socket on destruction
 *
 * */
class ServerSocket {
public:
  // Able to construct with specified port or without and let os choose
  ServerSocket(int port);
  ServerSocket();

  // Destructor closes socket
  ~ServerSocket();

  // Return port of socket
  int get_port();

  // Accept connection from one client and return fd of new connection
  int accept_client();

private:
  // Setup socket to listen
  void setup();

  // Close socket
  void cleanup();
  int port;
  int fd;
};
