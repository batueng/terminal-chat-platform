#include "UserSocket.h"
#include <string>
class ResponseHandler {
public:
  ResponseHandler();

  void send_username_response(UserSocket& user_sock, int status);

  void send_where_response(UserSocket& user_sock, int status, std::string session);

  void send_join_response(UserSocket& user_sock, int status);

  void send_create_response(UserSocket& user_sock, int status);

  void send_leave_response(UserSocket& user_sock, int status);

private:
};
