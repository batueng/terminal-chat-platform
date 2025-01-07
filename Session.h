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
#include <vector>

enum class message_type {
  CHAT = 0x00,
  USER_JOIN = 0x01,
  USER_LEFT = 0x02,
};

struct Message {
  message_type msg_t;
  std::string username;
  std::string text;

  Message deserialize_message(const std::vector<char>& data) {
        Message msg;

        size_t offset = 0;

        memcpy(&msg.msg_t, data.data() + offset, sizeof(msg.msg_t));
        offset += sizeof(msg.msg_t);

        uint32_t username_length;
        memcpy(&username_length, data.data() + offset, sizeof(username_length));
        offset += sizeof(username_length);

        msg.username = std::string(data.data() + offset, username_length);
        offset += username_length;

        uint32_t text_length;
        memcpy(&text_length, data.data() + offset, sizeof(text_length));
        offset += sizeof(text_length);

        msg.text = std::string(data.data() + offset, text_length);

        return msg;
    }

    std::vector<char> serialize_message(const Message& msg) {
        std::vector<char> data;

        data.insert(data.end(),
                    reinterpret_cast<const char*>(&msg.msg_t),
                    reinterpret_cast<const char*>(&msg.msg_t) + sizeof(msg.msg_t));
        
        uint32_t username_length = msg.username.size();
        data.insert(data.end(),
                    reinterpret_cast<const char*>(&username_length),
                    reinterpret_cast<const char*>(&username_length) + sizeof(username_length));

        data.insert(data.end(), msg.username.begin(), msg.username.end());

        uint32_t text_length = msg.text.size();
        data.insert(data.end(),
                    reinterpret_cast<const char*>(&text_length),
                    reinterpret_cast<const char*>(&text_length) + sizeof(text_length));

        data.insert(data.end(), msg.text.begin(), msg.text.end());

        return data;
    } 
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
