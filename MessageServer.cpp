#include "MessageServer.h"
#include "Session.h"
#include "errors.h"
#include "protocol.h"
#include "UserSocket.h"
#include <boost/thread.hpp>
#include <string>
#include <iostream>

MessageServer::MessageServer(uint16_t _listening_port)
    : server_sock(_listening_port) {}

void MessageServer::run() {
  std::cout << "started message server on port " << server_sock.get_port()
            << std::endl;
  while (true) {
    boost::thread t(&MessageServer::handle_client, this,
                    server_sock.accept_client());
    t.detach();
  }
}

void MessageServer::handle_client(int client_fd) {
  // connected
  UserSocket user_sock(client_fd);
  while (true) {
    try {
      std::string buf = user_sock.recv_len(sizeof(tcp_hdr_t));
      tcp_hdr_t *tcp_hdr = reinterpret_cast<tcp_hdr_t *>(buf.data());
      std::string data = user_sock.recv_len(tcp_hdr->data_len);

      std::string sess_name = tcp_hdr->session_name;
      std::string user_name = tcp_hdr->user_name;

      switch (tcp_hdr->method) {
      case tcp_method::U_NAME:
        user_sock.set_name(user_name);
        insert_user(user_name, user_sock);
        // send user_name response
        break;
      case tcp_method::WHERE:
        break;

      case tcp_method::JOIN: {
        std::shared_ptr<Session> sess = get_session(sess_name);
        sess->add_user(std::make_shared<UserSocket>(user_sock));
        // send join response
        break;
      }
      case tcp_method::CREATE: {
        std::shared_ptr<Session> sess = insert_session(sess_name);
        sess->add_user(std::make_shared<UserSocket>(user_sock));
        // send create response
        break;
      }
      case tcp_method::CHAT: {
        std::shared_ptr<Session> sess = get_session(sess_name);
        Message msg = {user_name, data.data()};
        sess->queue_msg(msg);
      }
      case tcp_method::LEAVE:
        break;

      default:
        break;
      }
    } catch (...) {
    }
  }
}

template <typename K, typename V, typename E>
V MessageServer::safe_get(std::unordered_map<K, V> &map, const K &key,
                          boost::shared_mutex &mtx) {
  boost::shared_lock<boost::shared_mutex> lock(mtx);
  auto it = map.find(key);
  if (it != map.end()) {
    return it->second;
  }
  throw E(key);
}

template <typename K, typename V, typename E>
V MessageServer::safe_insert(std::unordered_map<K, V> &map, const K &key,
                             const V &value, boost::shared_mutex &mtx) {
  boost::unique_lock<boost::shared_mutex> lock(mtx);
  if (!map.contains(key)) {
    map.emplace(key, value);
    return value;
  }

  throw E(key);
}

std::shared_ptr<Session> MessageServer::get_session(std::string &name) {
  return safe_get<std::string, std::shared_ptr<Session>, SessionNotFound>(
      sessions, name, sess_mtx);
}

std::shared_ptr<Session> MessageServer::insert_session(std::string &name) {
  return safe_insert<std::string, std::shared_ptr<Session>, DuplicateSession>(
      sessions, name, std::make_shared<Session>(name), sess_mtx);
}

UserSocket MessageServer::get_user(std::string &user_name) {
  return safe_get<std::string, UserSocket, UserNotFound>(users, user_name,
                                                         users_mtx);
}

UserSocket MessageServer::insert_user(std::string &user_name,
                                      UserSocket &user_socket) {
  return safe_insert<std::string, UserSocket, DuplicateUser>(
      users, user_name, user_socket, users_mtx);
}

int main(int argc, char *argv[]) {
  MessageServer server(8080);
  server.run();
  return 0;
}
