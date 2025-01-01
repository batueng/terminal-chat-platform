#ifndef RESPONSE_HANDLER
#define RESPONSE_HANDLER

#include "UserSocket.h"
#include "protocol.h"
#include <string>

class ResponseHandler {
public:
  ResponseHandler() {};

  void send_username_res(UserSocket &user_sock, tcp_status status,
                         std::string data = "");

  void send_where_res(UserSocket &user_sock, tcp_status status,
                      std::string session);

  void send_join_res(UserSocket &user_sock, tcp_status status);

  void send_create_res(UserSocket &user_sock, tcp_status status);

  void send_leave_res(UserSocket &user_sock, tcp_status status);

private:
};

#endif // !RESPONSE_HANDLER
