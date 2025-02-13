#ifndef session_h
#define session_h

#include "UserSocket.h"
#include "protocol.h"

#include <boost/thread.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <memory>
#include <queue>
#include <random>
#include <unordered_set>

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

  std::unordered_set<color> available_colors;

  std::random_device rd;

  std::mt19937 engine;

  std::uniform_int_distribution<int> dist;

  void broadcast_msg();

  color pick_color();
};

#endif
