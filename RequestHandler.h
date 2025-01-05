#ifndef RequestHandler_h
#define RequestHandler_h

#include <string>

#include "ClientSocket.h"
#include "protocol.h"

class RequestHandler {
public:
  RequestHandler(std::string &_ip, uint16_t _port);

  void send_username(std::string username);
  
  void send_create(std::string& session_name);

  void send_join(std::string& session_name);

  void send_where(std::string& username);

private:
  std::string ip;

  uint16_t port;

  ClientSocket client_sock;
};

#endif
