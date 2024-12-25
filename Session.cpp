#include "Session.h"
#include "protocol.h"

Session::Session(std::string _name) : name(_name), id(id_count++) {}

void Session::handle_session() {
  boost::unique_lock<boost::mutex> msg_lock(msg_mtx);
  while (true) {
    while (messages.empty()) {
      msg_cv.wait(msg_lock);
    }

    broadcast_msg();
  }
}

void Session::broadcast_msg() {
  boost::unique_lock<boost::shared_mutex> brdcst_lock(usr_mtx);

  Message msg;
  {
    boost::shared_lock<boost::mutex> msg_lock(msg_mtx);
    msg = messages.front();
    messages.pop();
  }

  for (auto &user : users) {
    tcp_hdr_t chat_hdr;
    chat_hdr.method = tcp_method::CHAT;
    chat_hdr.data_len = sizeof(msg);
    chat_hdr.session_id = id;
    std::memcpy(chat_hdr.user_name, user->name.c_str(), MAX_USERNAME);

    user->send_len(&chat_hdr, sizeof(chat_hdr));
  }
}

void Session::queue_msg(Message msg) {
  boost::unique_lock<boost::mutex> msg_lock(msg_mtx);
  messages.push(msg);
  msg_cv.notify_one();
}

void Session::add_user(std::shared_ptr<UserSocket> user) {
  boost::unique_lock<boost::shared_mutex> brdcst_lock(usr_mtx);
  users.insert(user);
}
