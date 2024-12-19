// server.h
#ifndef server_h
#define server_h

#include <chrono>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

class MessageServer {
public:
  MessageServer(int _listening_port);

  void run();

private:
  // while true for spinning client threads, client threads spawn session
  // threads and get held up by cv wait, when their user class is in_session
  void handle_client(int client_fd);
};

#endif
