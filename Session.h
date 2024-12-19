#ifndef session_h
#define session_h

#include <boost/thread.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <memory>
#include <queue>
#include <unordered_map>

class UserSocket;

class Session {
public:
  Session();

  void handle_session();

  void queue_message();

  friend class UserSocket;

private:
  struct Message {
    std::string user_name;
    std::string text;
  };

  boost::shared_mutex sess_mtx;
  boost::condition_variable message_cv;
  std::queue<Message> messages;

  std::unordered_map<std::string, std::shared_ptr<UserSocket>> users;

  void welcome_message(int user_fd);

  void broadcast_message();
};

#endif
