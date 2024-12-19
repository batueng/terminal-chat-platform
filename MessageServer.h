// server.h
#ifndef server_h
#define server_h

#include <Session.h>
#include <arpa/inet.h>
#include <chrono>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
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
  MessageServer(int _listening_port);

  // run select or create client per thread (handle_client)
  void run();

private:
  // open socket UserSocket
  // if (WHERE/JOIN/CREATE/CHAT);
  // JOIN/CREATE spin up a new session thread
  // CHAT adds a chat message to the session id
  void handle_client(int client_fd);

  std::unordered_map<int, Session> sessions;

  std::unordered_map<std::string, UserSocket> users;

  std::unordered_map<std::string, std::shared_ptr<Session>>
      user_sessions; // used for WHERE
};

#endif
