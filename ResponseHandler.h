#ifndef RESPONSE_HANDLER
#define RESPONSE_HANDLER

#include "UserSocket.h"
#include "errors.h"
#include "protocol.h"
#include <string>

class ResponseHandler {
public:
  ResponseHandler() {};

  void send_username_res(std::shared_ptr<UserSocket> user_ptr,
                         tcp_status status);

  void send_where_res(std::shared_ptr<UserSocket> user_ptr, tcp_status status,
                      std::string session);

  void send_join_res(std::shared_ptr<UserSocket> user_ptr, tcp_status status);

  void send_create_res(std::shared_ptr<UserSocket> user_ptr, tcp_status status);

  void send_leave_res(std::shared_ptr<UserSocket> user_ptr, tcp_status status);

  void send_err_res(UserSocket &user_sock, TCPError &e);

private:
};

#endif // !RESPONSE_HANDLER
