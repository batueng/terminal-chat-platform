#ifndef RequestHandler_h
#define RequestHandler_h

#include "ClientSocket.h"
#include "protocol.h"

#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include <fstream>
#include <queue>
#include <string>

class Client;

class RequestHandler {
public:
  RequestHandler(std::string &_ip, uint16_t _port);

  void queue_res(tcp_hdr_t res_hdr, std::string msg);

  tcp_status send_username(std::string username, std::string &err_msg);

  std::pair<color, tcp_status> send_create(std::string &username,
                                           std::string &session_name,
                                           std::string &err_msg);

  std::pair<color, tcp_status> send_join(std::string &username,
                                         std::string &session_name,
                                         std::string &err_msg);

  std::pair<std::string, tcp_status> send_where(std::string &username,
                                                std::string &target_username,
                                                std::string &err_msg);

  void send_message(std::string &username, std::string &session_name,
                    Message &msg);

  void send_leave(std::string &username, std::string &session_name);

  void send_shutdown(std::string &username, std::string &session_name);

  friend class Client;

private:
  std::string ip;

  uint16_t port;

  ClientSocket client_sock;

  boost::mutex res_mtx;

  boost::condition_variable res_cv;

  std::queue<std::pair<tcp_hdr_t, std::string>> res_q;

  std::ofstream of;
};

#endif
