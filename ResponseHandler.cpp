#include "ResponseHandler.h"
#include "UserSocket.h"
#include "protocol.h"

void ResponseHandler::send_username_res(UserSocket &user_sock,
                                        tcp_status status, std::string data) {
  tcp_hdr_t uname_hdr = {tcp_method::U_NAME, status, data.size()};
  std::string username = user_sock.get_name();
  std::memcpy(uname_hdr.username, username.c_str(), username.size());
  uname_hdr.username[username.size()] = '\0';

  user_sock.send_len(&uname_hdr, sizeof(tcp_hdr_t));

  if (data.size()) {
    user_sock.send_len(data.c_str(), data.size());
  }
}
