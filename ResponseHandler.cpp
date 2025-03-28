#include "ResponseHandler.h"
#include "UserSocket.h"
#include "errors.h"
#include "protocol.h"

void ResponseHandler::send_username_res(std::shared_ptr<UserSocket> user_ptr,
                                        tcp_status status) {
  tcp_hdr_t uname_hdr(tcp_method::U_NAME, status, color::DEFAULT, 0,
                      user_ptr->get_name(), "");

  user_ptr->send_len(&uname_hdr, sizeof(tcp_hdr_t));
}

void ResponseHandler::send_where_res(std::shared_ptr<UserSocket> user_ptr,
                                     tcp_status status, std::string session) {
  tcp_hdr_t where_hdr(tcp_method::WHERE, status, color::DEFAULT, session.size(),
                      user_ptr->get_name(), "");

  user_ptr->send_len(&where_hdr, sizeof(where_hdr));
  user_ptr->send_len(session.c_str(), session.size());
}

void ResponseHandler::send_join_res(std::shared_ptr<UserSocket> user_ptr,
                                    tcp_status status) {
  tcp_hdr_t join_hdr(tcp_method::WHERE, status, user_ptr->get_color(), 0,
                     user_ptr->get_name(), "");

  user_ptr->send_len(&join_hdr, sizeof(join_hdr));
}

void ResponseHandler::send_create_res(std::shared_ptr<UserSocket> user_ptr,
                                      tcp_status status) {
  tcp_hdr_t create_hdr(tcp_method::CREATE, status, user_ptr->get_color(), 0,
                       user_ptr->get_name(), "");

  user_ptr->send_len(&create_hdr, sizeof(create_hdr));
}

void ResponseHandler::send_leave_res(std::shared_ptr<UserSocket> user_ptr,
                                     tcp_status status) {
  tcp_hdr_t leave_hdr(tcp_method::LEAVE, status, user_ptr->get_color(), 0,
                      user_ptr->get_name(), "");

  user_ptr->send_len(&leave_hdr, sizeof(leave_hdr));
}

void ResponseHandler::send_err_res(std::shared_ptr<UserSocket> user_ptr,
                                   TCPError &e) {
  const char *err_msg = e.what();
  size_t err_len = strlen(err_msg);

  tcp_hdr_t err_hdr(tcp_method::ERROR, e.get_status(), color::DEFAULT, err_len,
                    user_ptr->get_name(), "");

  user_ptr->send_len(&err_hdr, sizeof(err_hdr));
  user_ptr->send_len(err_msg, err_len);
}
