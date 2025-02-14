// MessageServer.h
#ifndef MessageServer_h
#define MessageServer_h

#include <arpa/inet.h>
#include <cstring>
#include <ctime>
#include <memory>
#include <netdb.h>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

#include "ResponseHandler.h"
#include "ServerSocket.h"
#include "Session.h"
#include "unordered_map_ts.hpp"

class MessageServer {
public:
  MessageServer(uint16_t _listening_port);

  void run();

private:
  ServerSocket server_sock;
  ResponseHandler res_handler;

  void handle_client(int client_fd);

  unordered_map_ts<std::string, std::shared_ptr<Session>> sessions;

  unordered_map_ts<std::string, std::shared_ptr<UserSocket>> users;

  unordered_map_ts<std::string, std::string>
      user_sessions; // username -> session_name;  used for WHERE
};

#endif
