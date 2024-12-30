// MessageServer.h
#ifndef MessageServer_h
#define MessageServer_h

#include "Session.h"
#include "ResponseHandler.h"
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

#include "ServerSocket.h"
#include "Session.h"

class MessageServer {
public:
  MessageServer(uint16_t _listening_port);

  void run();

private:
  ServerSocket server_sock;
  ResponseHandler response_handler;

  void handle_client(int client_fd);

  std::shared_ptr<Session> get_session(std::string &name);
  std::shared_ptr<Session> insert_session(std::string &name);

  UserSocket get_user(std::string &user_name);
  UserSocket insert_user(std::string &user_name, UserSocket &user_socket);

  template <typename K, typename V, typename E>
  V safe_get(std::unordered_map<K, V> &map, const K &key,
             boost::shared_mutex &mtx);

  template <typename K, typename V, typename E>
  V safe_insert(std::unordered_map<K, V> &map, const K &key, const V &value,
                boost::shared_mutex &mtx);

  boost::shared_mutex sess_mtx;
  std::unordered_map<std::string, std::shared_ptr<Session>> sessions;

  boost::shared_mutex users_mtx;
  std::unordered_map<std::string, UserSocket> users; // username -> UserSocket

  boost::shared_mutex where_mtx;
  std::unordered_map<std::string, std::string>
      user_sessions; // username -> session_name;  used for WHERE
};

#endif
