#include "MessageServer.h"
#include "Session.h"
#include "UserSocket.h"
#include "errors.h"
#include "protocol.h"
#include <boost/thread.hpp>
#include <exception>
#include <iostream>
#include <string>

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
  UserSocket user_sock(client_fd);
  while (true) {
    try {
      std::string buf = user_sock.recv_len(sizeof(tcp_hdr_t));
      tcp_hdr_t *tcp_hdr = reinterpret_cast<tcp_hdr_t *>(buf.data());
      std::string data = user_sock.recv_len(tcp_hdr->data_len);

      std::string sess_name = tcp_hdr->session_name;
      std::string username = tcp_hdr->username;

      switch (tcp_hdr->method) {
      case tcp_method::U_NAME:
        user_sock.set_name(username);
        insert_user(username,
                    std::make_shared<UserSocket>(std::move(user_sock)));
        std::cout << "user inserted: " << username << std::endl;
        res_handler.send_username_res(user_sock, tcp_status::SUCCESS);
        break;
      case tcp_method::WHERE:
        break;

      case tcp_method::JOIN: {
        std::shared_ptr<Session> sess = get_session(sess_name);
        sess->add_user(std::make_shared<UserSocket>(std::move(user_sock)));
        std::cout << "join session request: " << sess_name << std::endl;
        res_handler.send_join_res(user_sock, tcp_status::SUCCESS);
        // send join response
        break;
      }
      case tcp_method::CREATE: {
        std::shared_ptr<Session> sess = insert_session(sess_name);
        sess->add_user(std::make_shared<UserSocket>(std::move(user_sock)));
        std::cout << "create session request: " << sess_name << std::endl;
        boost::thread t(&Session::handle_session, sess.get());
        // send create response
        res_handler.send_create_res(user_sock, tcp_status::SUCCESS);
        break;
      }
      case tcp_method::MESSAGE: {
        std::shared_ptr<Session> sess = get_session(sess_name);

        Message msg = Message::deserialize_message(
            std::vector<char>(data.begin(), data.end()));

        sess->queue_msg(msg);
        break;
      }
      case tcp_method::LEAVE:
        break;

      default:
        break;
      }
    } catch (TCPError &e) {
      res_handler.send_err_res(user_sock, e);
    } catch (std::exception &e) {
      std::cout << e.what() << std::endl;
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
    auto [it, inserted] = map.emplace(key, value);
    return it->second;
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

std::shared_ptr<UserSocket> MessageServer::get_user(std::string &username) {
  return safe_get<std::string, std::shared_ptr<UserSocket>, UserNotFound>(
      users, username, users_mtx);
}

std::shared_ptr<UserSocket>
MessageServer::insert_user(std::string &username,
                           std::shared_ptr<UserSocket> user_socket) {
  return safe_insert<std::string, std::shared_ptr<UserSocket>, DuplicateUser>(
      users, username, user_socket, users_mtx);
}

int main(int argc, char *argv[]) {
  MessageServer server(8080);
  server.run();
  return 0;
}
