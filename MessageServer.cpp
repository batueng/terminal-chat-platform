#include <arpa/inet.h>
#include <cstring>
#include <ctime>
#include <netdb.h>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

#include "MessageServer.h"
#include "errors.h"
#include "protocol.h"

MessageServer::MessageServer(uint16_t _listening_port)
    : server_sock(_listening_port) {}

void MessageServer::run() {
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
      std::string user_name = tcp_hdr->user_name;

      switch (tcp_hdr->method) {
      case tcp_method::U_NAME:

        break;
      case tcp_method::WHERE:
        break;

      case tcp_method::JOIN: {
        std::shared_ptr<Session> sess = get_session(sess_name);
        sess->add_user(std::make_shared<UserSocket>(user_sock));

        break;
      }
      case tcp_method::CREATE: {
        std::shared_ptr<Session> sess = insert_session(sess_name);
        sess->add_user(std::make_shared<UserSocket>(user_sock));
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

UserSocket MessageServer::get_user(std::string &user_name) {
  boost::shared_lock<boost::shared_mutex> sess_lock(users_mtx);
  return UserSocket{0};
}

std::shared_ptr<Session> MessageServer::get_session(std::string &name) {
  boost::shared_lock<boost::shared_mutex> sess_lock(sess_mtx);
  auto sess = sessions.find(name);
  if (sess != sessions.end())
    return sess->second;

  throw SessionNotFound(name);
}

std::shared_ptr<Session> MessageServer::insert_session(std::string &name) {
  boost::unique_lock<boost::shared_mutex> sess_lock(sess_mtx);
  if (!sessions.contains(name)) {
    sessions[name] = std::make_shared<Session>(Session(name));
    return sessions[name];
  }

  throw DuplicateSessionError(name);
}
