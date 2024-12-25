/*
 * ServerSocket class represents the servers main socket
 * which accepts connections from clients
 *
 * Implements RAII by opening, binding and listening on construction
 * Closes socket on destruction
 *
 * */
#include <cstdint>
class ServerSocket {
public:
  // Able to construct with specified port or without and let os choose
  ServerSocket(uint16_t port);
  ServerSocket();

  // Destructor closes socket
  ~ServerSocket();

  // Return port of socket
  uint16_t get_port();

  // Accept connection from one client and return fd of new connection
  int accept_client();

private:
  // Setup socket to listen
  void setup();

  // Close socket
  void cleanup();
  uint16_t port;
  int fd;
};
