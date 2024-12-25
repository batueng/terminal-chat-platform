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

void MessageServer::handle_create(std::string name) {
  if (sessions.contains(name)) {
    throw DuplicateSessionError(name);
  }
}

void MessageServer::handle_client(int client_fd) {
  UserSocket user_sock(client_fd);
  while (true) {
    try {
      std::string buf = user_sock.recv_len(sizeof(tcp_hdr_t));
      tcp_hdr_t *tcp_hdr = reinterpret_cast<tcp_hdr_t *>(buf.data());
      std::string data = user_sock.recv_len(tcp_hdr->data_len);

      switch (tcp_hdr->method) {
      case tcp_method::WHERE:
        break;

      case tcp_method::JOIN:
        break;

      case tcp_method::CREATE: {
        // data is the session name for create
        boost::unique_lock<boost::shared_mutex> sess_lock(sessions_mtx);
        sessions.emplace(tcp_hdr->session_name, Session(tcp_hdr->session_name));
        sessions[tcp_hdr->session_name].add_user(
            std::make_shared<UserSocket>(user_sock));
        break;
      }
      case tcp_method::CHAT: {
        boost::unique_lock<boost::shared_mutex> sess_lock(sessions_mtx);
        Message message{tcp_hdr->user_name, data};
        sessions[tcp_hdr->session_name].queue_msg(message);
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
