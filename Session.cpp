#include "Session.h"
#include "protocol.h"

void Session::handle_session() {
  while (true) {
    {
      boost::unique_lock<boost::mutex> msg_lock(message_mtx);
      while (messages.empty()) {
        message_cv.wait(msg_lock);
      }

      broadcast_message();
    }
  }
}

void Session::broadcast_message() {
  boost::shared_lock<boost::shared_mutex> users_lock(users_mtx);
  Message message;
  {
    boost::unique_lock<boost::mutex> msg_lock(message_mtx);
    message = messages.front();
    messages.pop();
  }

  for (auto &user : users) {
    tcp_hdr_t chat_hdr;
    chat_hdr.method = tcp_method::CHAT;
    chat_hdr.data_len = sizeof(message);
    chat_hdr.session_id = id;
    std::memcpy(chat_hdr.user_name, user->name.c_str(), MAX_USERNAME);

    user->send_len(&chat_hdr, sizeof(chat_hdr));
  }
}
