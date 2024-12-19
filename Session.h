#ifndef session_h
#define session_h

#include <boost/thread.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <memory>
#include <unordered_map>

class UserSocket;

class Session {
public:
  Session(const Session &) = default;
  Session(Session &&) = default;
  Session &operator=(const Session &) = default;
  Session &operator=(Session &&) = default;
  void handle_session();

  void queue_message();
  friend class UserSocket;

private:
  boost::shared_mutex;

  std::unordered_map<std::string, UserSocket> users;

  void welcome_message(int user_fd);

  void broadcast_message();
};

#endif
