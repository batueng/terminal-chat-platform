#ifndef session_h
#define session_h

#include "UserSocket.h"
#include <boost/thread.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <memory>
#include <queue>
#include <unordered_map>
#include <unordered_set>

struct Message {
  std::string user_name;
  std::string text;
};

static uint64_t id_count;

class Session {
public:
  Session();

  void handle_session();

  void queue_msg(Message msg);

private:
  uint64_t id;

  // messages data/sync
  boost::mutex msg_mtx;
  boost::condition_variable msg_cv;
  std::queue<Message> messages;

  // users data/sync
  boost::shared_mutex users_mtx;

  std::unordered_set<std::shared_ptr<UserSocket>> users;

  void broadcast_msg();
};

#endif
