#include "RequestHandler.h"
#include <iostream>

RequestHandler::RequestHandler(std::string &_ip, uint16_t _port)
    : ip(_ip), port(_port), client_sock(_ip, _port) {}

void RequestHandler::send_username(std::string username) {
  // create req hdr
  tcp_hdr_t uname_hdr = {tcp_method::U_NAME, tcp_status::SUCCESS, 0};
  std::memcpy(uname_hdr.username, username.c_str(), username.size());
  uname_hdr.username[username.size()] = '\0';

  client_sock.send_len(&uname_hdr, sizeof(tcp_hdr_t));

  // recv response
  std::string res = client_sock.recv_len(sizeof(tcp_hdr_t));
  tcp_hdr_t *res_hdr = reinterpret_cast<tcp_hdr_t *>(res.data());

  // print error if failed
  if (res_hdr->status != tcp_status::SUCCESS) {
    std::string err_mes = client_sock.recv_len(res_hdr->data_len);
    std::cout << err_mes << std::endl;
  };
}
