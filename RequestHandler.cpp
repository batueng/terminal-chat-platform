#include "RequestHandler.h"

#include <iostream>
#include <utility>

RequestHandler::RequestHandler(std::string &_ip, uint16_t _port)
    : ip(_ip), port(_port), client_sock(_ip, _port) {}

void RequestHandler::send_username(std::string username) {
  // create req hdr
  tcp_hdr_t uname_hdr = {tcp_method::U_NAME, tcp_status::SUCCESS, 0};
  std::memcpy(uname_hdr.username, username.c_str(), username.size());
  uname_hdr.username[username.size()] = '\0';

  client_sock.send_len(&uname_hdr, sizeof(tcp_hdr_t));

  boost::unique_lock<boost::mutex> res_lock(res_mtx);
  while (res_q.empty()) {
    res_cv.wait(res_lock);
  }

  auto [res_hdr, err_msg] = res_q.front();
  res_q.pop();

  if (res_hdr.status != tcp_status::SUCCESS) {
    std::cout << err_msg << std::endl;
  };
}

void RequestHandler::queue_res(tcp_hdr_t res_hdr, std::string msg) {
  boost::unique_lock<boost::mutex> res_lock(res_mtx);
  res_q.push({res_hdr, msg});
  res_cv.notify_one();
}

void RequestHandler::send_create(std::string &username,
                                 std::string &session_name) {
  tcp_hdr_t create_hdr = {tcp_method::CREATE};
  std::memcpy(create_hdr.username, username.c_str(), username.size());
  create_hdr.username[username.size()] = '\0';
  std::memcpy(create_hdr.session_name, session_name.c_str(),
              session_name.size());
  create_hdr.session_name[session_name.size()] = '\0';

  // send header
  client_sock.send_len(&create_hdr, sizeof(tcp_hdr_t));

  boost::unique_lock<boost::mutex> res_lock(res_mtx);
  while (res_q.empty()) {
    res_cv.wait(res_lock);
  }

  std::cout << "got to create" << std::endl;

  auto [res_hdr, err_msg] = res_q.front();
  res_q.pop();

  // print error if failed
  if (res_hdr.status != tcp_status::SUCCESS) {
    std::cout << err_msg << std::endl;
  };
}

void RequestHandler::send_join(std::string &username,
                               std::string &session_name) {
  tcp_hdr_t join_hdr = {tcp_method::JOIN};
  std::memcpy(join_hdr.username, username.c_str(), username.size());
  join_hdr.username[username.size()] = '\0';
  std::memcpy(join_hdr.session_name, session_name.c_str(), session_name.size());
  join_hdr.session_name[session_name.size()] = '\0';

  // send header
  client_sock.send_len(&join_hdr, sizeof(tcp_hdr_t));

  boost::unique_lock<boost::mutex> res_lock(res_mtx);
  while (res_q.empty()) {
    res_cv.wait(res_lock);
  }

  auto [res_hdr, err_msg] = res_q.front();
  res_q.pop();

  // print error if failed
  if (res_hdr.status != tcp_status::SUCCESS) {
    std::cout << err_msg << std::endl;
  };
}

void RequestHandler::send_where(std::string &username,
                                std::string &target_username) {
  tcp_hdr_t where_hdr = {tcp_method::WHERE};
  where_hdr.data_len = target_username.size();
  std::memcpy(where_hdr.username, username.c_str(), username.size());
  where_hdr.username[username.size()] = '\0';

  // send header
  client_sock.send_len(&where_hdr, sizeof(tcp_hdr_t));
  // send username as data
  client_sock.send_len(target_username.c_str(), target_username.size());

  boost::unique_lock<boost::mutex> res_lock(res_mtx);
  while (res_q.empty()) {
    res_cv.wait(res_lock);
  }

  auto [res_hdr, err_msg] = res_q.front();
  res_q.pop();

  // print error if failed
  if (res_hdr.status != tcp_status::SUCCESS) {
    std::cout << err_msg << std::endl;
  };
}

void RequestHandler::send_message(std::string &username,
                                  std::string &session_name, Message &msg) {
  tcp_hdr_t message_hdr = {tcp_method::MESSAGE};

  std::vector<char> serialized_msg = msg.serialize_message();
  message_hdr.data_len = serialized_msg.size();

  std::memcpy(&message_hdr.username, username.c_str(), username.size());
  message_hdr.username[username.size()] = '\0';

  std::memcpy(&message_hdr.session_name, session_name.c_str(),
              session_name.size());
  message_hdr.session_name[session_name.size()] = '\0';

  client_sock.send_len(&message_hdr, sizeof(tcp_hdr_t));

  client_sock.send_len(serialized_msg.data(), serialized_msg.size());
}
