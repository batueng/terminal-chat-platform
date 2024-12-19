#include "Session.h"
#include "UserSocket.h"

void Session::handle_session() {
  while (true) {
    {
      boost::unique_lock<boost::mutex> message_lock(message_mtx);
      while (messages.empty()) {
        message_cv.wait(message_lock);
      }

      broadcast_message();
    }
  }
}

void Session::broadcast_message() {
  // ALWAYS LOCK USERS THEN MESSAGE FOR DEADLOCK ORDERING
  boost::shared_lock<boost::shared_mutex> users_lock(users_mtx);
  {
    boost::unique_lock<boost::mutex> message_lock(message_mtx);
    Message message = messages.front();
    messages.pop();
  }

  for (auto &user : users) {
  }
}
