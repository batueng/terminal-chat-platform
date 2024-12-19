#ifndef session_h
#define session_h

#include "UserSocket.h"
#include <boost/thread.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <memory>
#include <queue>
#include <unordered_map>
#include <unordered_set>

class Session {
public:
  Session();

  void handle_session();

  void queue_message();

private:
  static uint64_t id;

  struct Message {
    std::string user_name;
    std::string text;
  };

  // messages data/sync
  boost::mutex message_mtx;
  boost::condition_variable message_cv;
  std::queue<Message> messages;

  // users data/sync
  boost::shared_mutex users_mtx;

  std::unordered_set<std::shared_ptr<UserSocket>> users;

  void broadcast_message();
};

#endif
