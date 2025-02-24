#include "RequestHandler.h"
#include "protocol.h"

#include <iostream>
#include <utility>

RequestHandler::RequestHandler(std::string &_ip, uint16_t _port)
    : ip(_ip), port(_port), client_sock(_ip, _port) {}

tcp_status RequestHandler::send_username(std::string username,
                                         std::string &err_msg) {

  tcp_hdr_t uname_hdr(tcp_method::U_NAME, tcp_status::SUCCESS, color::DEFAULT,
                      0, username, "");

  client_sock.send_len(&uname_hdr, sizeof(tcp_hdr_t));

  boost::unique_lock<boost::mutex> res_lock(res_mtx);
  res_cv.wait(res_lock, [&] { return !res_q.empty(); });

  auto res_pair = res_q.front();
  tcp_hdr_t res_hdr = res_pair.first;
  err_msg = res_pair.second;

  res_q.pop();

  return res_hdr.status;
}

void RequestHandler::queue_res(tcp_hdr_t res_hdr, std::string msg) {
  boost::unique_lock<boost::mutex> res_lock(res_mtx);

  res_q.push({res_hdr, msg});

  if (!of.is_open()) {
    of.open(std::string(res_hdr.username) + ".txt");
  }
  of << res_hdr.username << " queued response for "
     << static_cast<int>(res_hdr.method) << " with msg " << msg << std::endl;

  res_cv.notify_one();

  of << res_hdr.username << " has notified the cv of response "
     << static_cast<int>(res_hdr.method) << " with msg " << msg << std::endl;
}

std::pair<color, tcp_status>
RequestHandler::send_create(std::string &username, std::string &session_name,
                            std::string &err_msg) {

  tcp_hdr_t create_hdr(tcp_method::CREATE, tcp_status::SUCCESS, color::DEFAULT,
                       0, username, session_name);

  // send header
  client_sock.send_len(&create_hdr, sizeof(tcp_hdr_t));

  boost::unique_lock<boost::mutex> res_lock(res_mtx);
  res_cv.wait(res_lock, [&] { return !res_q.empty(); });

  auto [res_hdr, data] = res_q.front();
  res_q.pop();

  err_msg = data;

  return {res_hdr.c, res_hdr.status};
}

std::pair<color, tcp_status>
RequestHandler::send_join(std::string &username, std::string &session_name,
                          std::string &err_msg) {

  tcp_hdr_t join_hdr(tcp_method::JOIN, tcp_status::SUCCESS, color::DEFAULT, 0,
                     username, session_name);
  // send header
  client_sock.send_len(&join_hdr, sizeof(tcp_hdr_t));

  boost::unique_lock<boost::mutex> res_lock(res_mtx);
  res_cv.wait(res_lock, [&] { return !res_q.empty(); });

  auto [res_hdr, data] = res_q.front();
  res_q.pop();

  return {res_hdr.c, res_hdr.status};
}

std::pair<std::string, tcp_status>
RequestHandler::send_where(std::string &username, std::string &target_username,
                           std::string &err_msg) {

  tcp_hdr_t where_hdr(tcp_method::WHERE, tcp_status::SUCCESS, color::DEFAULT,
                      target_username.size(), username, "");

  if (!of.is_open()) {
    of.open(username + ".txt");
  }
  // send header
  client_sock.send_len(&where_hdr, sizeof(tcp_hdr_t));
  // send username as data
  client_sock.send_len(target_username.c_str(), target_username.size());

  boost::unique_lock<boost::mutex> res_lock(res_mtx);
  of.close();
  res_cv.wait(res_lock, [&] { return !res_q.empty(); });

  auto [res_hdr, data] = res_q.front();
  res_q.pop();

  return {data, res_hdr.status};
}

void RequestHandler::send_message(std::string &username,
                                  std::string &session_name, Message &msg) {

  std::vector<char> serialized_msg = msg.serialize_message();

  tcp_hdr_t msg_hdr(tcp_method::MESSAGE, tcp_status::SUCCESS, msg.color,
                    serialized_msg.size(), username, session_name);

  client_sock.send_len(&msg_hdr, sizeof(tcp_hdr_t));
  client_sock.send_len(serialized_msg.data(), serialized_msg.size());
}

void RequestHandler::send_leave(std::string &username,
                                std::string &session_name) {
  tcp_hdr_t leave_hdr(tcp_method::LEAVE, tcp_status::SUCCESS, color::DEFAULT, 0,
                      username, session_name);
  client_sock.send_len(&leave_hdr, sizeof(tcp_hdr_t));

  boost::unique_lock<boost::mutex> res_lock(res_mtx);
  res_cv.wait(res_lock, [&] { return !res_q.empty(); });

  auto [res_hdr, err_msg] = res_q.front();
  res_q.pop();

  if (res_hdr.status != tcp_status::SUCCESS)
    std::cout << err_msg << std::endl;
}

void RequestHandler::send_shutdown(std::string &username) {

  tcp_hdr_t shutdown_hdr(tcp_method::U_SHUTDOWN, tcp_status::SUCCESS, color::DEFAULT, 0,
                         username, "");

  client_sock.send_len(&shutdown_hdr, sizeof(tcp_hdr_t));
}
