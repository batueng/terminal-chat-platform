#ifndef session_h
#define session_h

#include "UserSocket.h"
#include <boost/thread.hpp>
#include <boost/thread/condition_variable.hpp>
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
  explicit Session(std::string &_name);

  void handle_session();

  void queue_msg(Message &msg);

  void add_user(std::shared_ptr<UserSocket> user);

private:
  uint64_t id;
  std::string name;

  // user data/sync
  boost::shared_mutex usr_mtx;

  // messages data/sync
  boost::mutex msg_mtx;
  boost::condition_variable msg_cv;
  std::queue<Message> messages;

  std::unordered_set<std::shared_ptr<UserSocket>> users;

  void broadcast_msg();
};

#endif
