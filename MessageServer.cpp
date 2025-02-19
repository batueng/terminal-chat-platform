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
      case tcp_method::U_NAME: {
        user_sock.set_name(username);

        std::shared_ptr<UserSocket> user_ptr =
            std::make_shared<UserSocket>(std::move(user_sock));

        users.emplace<DuplicateUser>(username, user_ptr);

        std::cout << "user inserted: " << username << std::endl;

        res_handler.send_username_res(user_ptr, tcp_status::SUCCESS);
        break;
      }
      case tcp_method::WHERE: {
        std::shared_ptr<UserSocket> user_ptr =
            users.find<UserNotFound>(username);

        std::string user_loc = user_sessions.find<UserNotFound>(data);

        std::cout << "found user: " << data << " in session: " << user_loc
                  << std::endl;

        res_handler.send_where_res(user_ptr, tcp_status::SUCCESS, user_loc);
        break;
      }
      case tcp_method::JOIN: {
        std::shared_ptr<Session> sess =
            sessions.find<SessionNotFound>(sess_name);

        std::shared_ptr<UserSocket> user_ptr =
            users.find<UserNotFound>(username);

        user_sessions.emplace<DuplicateUser>(user_ptr->get_name(),
                                             sess->get_name());

        sess->add_user(user_ptr);

        std::cout << "join session request: " << sess_name << std::endl;

        res_handler.send_join_res(user_ptr, tcp_status::SUCCESS);
        break;
      }
      case tcp_method::CREATE: {
        std::shared_ptr<Session> sess = sessions.emplace<DuplicateSession>(
            sess_name, std::make_shared<Session>(sess_name));

        std::shared_ptr<UserSocket> user_ptr =
            users.find<UserNotFound>(username);

        user_sessions.emplace<DuplicateUser>(user_ptr->get_name(),
                                             sess->get_name());

        sess->add_user(user_ptr);

        std::cout << "create session request: " << sess_name << std::endl;

        boost::thread t(&Session::handle_session, sess.get());

        res_handler.send_create_res(user_ptr, tcp_status::SUCCESS);
        break;
      }
      case tcp_method::MESSAGE: {
        std::shared_ptr<Session> sess =
            sessions.find<SessionNotFound>(sess_name);

        Message msg = Message::deserialize_message(
            std::vector<char>(data.begin(), data.end()));

        sess->queue_msg(msg);
        break;
      }
      case tcp_method::LEAVE: {
        user_sessions.erase(username);

        std::shared_ptr<Session> sess =
            sessions.find<SessionNotFound>(sess_name);

        std::shared_ptr<UserSocket> user_ptr =
            users.find<UserNotFound>(username);

        sess->remove_user(user_ptr);

        if (sess->num_users() == 0) {
          sessions.erase(sess_name);
        }

        res_handler.send_leave_res(user_ptr, tcp_status::SUCCESS);

        break;
      }

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

int main(int argc, char *argv[]) {
  MessageServer server(8080);
  server.run();
  return 0;
}
