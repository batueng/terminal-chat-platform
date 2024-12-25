// MessageServer.h
#ifndef MessageServer_h
#define MessageServer_h

#include "Session.h"
#include <ServerSocket.h>
#include <arpa/inet.h>
#include <chrono>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <netdb.h>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class MessageServer {
public:
  MessageServer(uint16_t _listening_port);

  // run select or create client per thread (handle_client)
  void run();

private:
  ServerSocket server_sock;

  // open socket UserSocket
  // if (WHERE/JOIN/CREATE/CHAT);
  // JOIN/CREATE spin up a new session thread
  // CHAT adds a chat message to the session id
  void handle_client(int client_fd);

  void handle_create(std::string name);

  boost::shared_mutex sessions_mtx;
  std::unordered_map<std::string, Session> sessions; // name -> Session

  boost::shared_mutex users_mtx;
  std::unordered_map<std::string, UserSocket> users; // name -> UserSocket

  boost::shared_mutex where_mtx;
  std::unordered_map<std::string, std::shared_ptr<Session>>
      user_sessions; // name -> Session ptr;  used for WHERE
};

#endif
