#include "ResponseHandler.h"
#include "UserSocket.h"
#include "errors.h"
#include "protocol.h"

void ResponseHandler::send_username_res(UserSocket &user_sock,
                                        tcp_status status) {
  tcp_hdr_t uname_hdr = {tcp_method::U_NAME, status, 0};
  std::string username = user_sock.get_name();
  std::memcpy(uname_hdr.username, username.c_str(), username.size());
  uname_hdr.username[username.size()] = '\0';

  user_sock.send_len(&uname_hdr, sizeof(tcp_hdr_t));
}

void ResponseHandler::send_where_res(UserSocket &user_sock, tcp_status status,
                                     std::string session) {
  tcp_hdr_t where_hdr = {tcp_method::WHERE, status, session.size()};
  std::string username = user_sock.get_name();
  std::memcpy(where_hdr.username, username.c_str(), username.size());
  where_hdr.username[username.size()] = '\0';

  user_sock.send_len(&where_hdr, sizeof(where_hdr));
  user_sock.send_len(session.c_str(), session.size());
}

void ResponseHandler::send_join_res(UserSocket &user_sock, tcp_status status) {
  tcp_hdr_t join_hdr = {tcp_method::JOIN, status, 0};
  std::string username = user_sock.get_name();
  std::memcpy(join_hdr.username, username.c_str(), username.size());
  join_hdr.username[username.size()] = '\0';

  user_sock.send_len(&join_hdr, sizeof(join_hdr));
}

void ResponseHandler::send_create_res(UserSocket &user_sock,
                                      tcp_status status) {
  tcp_hdr_t create_hdr = {tcp_method::CREATE, status, 0};
  std::string username = user_sock.get_name();
  std::memcpy(create_hdr.username, username.c_str(), username.size());
  create_hdr.username[username.size()] = '\0';

  user_sock.send_len(&create_hdr, sizeof(create_hdr));
}

void ResponseHandler::send_leave_res(UserSocket &user_sock, tcp_status status) {
  tcp_hdr_t leave_hdr = {tcp_method::LEAVE, status, 0};
  std::string username = user_sock.get_name();
  std::memcpy(leave_hdr.username, username.c_str(), username.size());
  leave_hdr.username[username.size()] = '\0';

  user_sock.send_len(&leave_hdr, sizeof(leave_hdr));
}

void ResponseHandler::send_err_res(UserSocket &user_sock, TCPError &e) {
  const char *err_msg = e.what();
  size_t err_len = strlen(err_msg);

  tcp_hdr_t err_hdr = {tcp_method::ERROR, e.get_status(), err_len};
  std::string username = user_sock.get_name();
  std::memcpy(err_hdr.username, username.c_str(), username.size());
  err_hdr.username[username.size()] = '\0';

  user_sock.send_len(&err_hdr, sizeof(err_hdr));
  user_sock.send_len(err_msg, err_len);
}
