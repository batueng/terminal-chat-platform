/*
 * ClientSocket class represents the client's main socket 
 * which connects to a server
 *
 * Implements RAII by opening, and connecting on construction
 * Closes socket on destruction
 *
 * */

#include <cstdint>
#include <string>
#include <netinet/in.h>
class ClientSocket {
public:
  // Able to construct with specified ip and port
  ClientSocket(std::string& ip, uint16_t port);
  ClientSocket();

  // Destructor closes socket
  ~ClientSocket();

  void send_len(const void* buf, int n);

  std::string recv_len(int n);

private:
  // Setup socket
  void setup();

  // Connect to server
  void connect_to_server();

  // Close socket
  void cleanup();
  std::string ip;
  uint16_t port;
  int fd;
  struct sockaddr_in addr;
};
