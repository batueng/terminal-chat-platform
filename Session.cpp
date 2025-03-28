#include "Session.h"
#include "protocol.h"

#include <iostream>

Session::Session(std::string &_name) : name(_name), id(id_count++) {
  engine = std::mt19937(rd());
  std::uniform_int_distribution<int> dist(static_cast<int>(color::RED),
                                          static_cast<int>(color::END));

  for (uint8_t i = static_cast<uint8_t>(color::RED);
       i < static_cast<uint8_t>(color::END); ++i) {
    available_colors.insert(static_cast<color>(i));
  }
}

void Session::handle_session() {
  while (true) {
    {
      boost::unique_lock<boost::mutex> msg_lock(msg_mtx);
      while (messages.empty()) {
        msg_cv.wait(msg_lock);
      }
    }

    broadcast_msg();
  }
}

void Session::broadcast_msg() {
  boost::unique_lock<boost::shared_mutex> brdcst_lock(usr_mtx);

  Message msg;
  {
    boost::unique_lock<boost::mutex> msg_lock(msg_mtx);
    msg = messages.front();
    messages.pop();
  }

  const std::vector<char> serialized_msg = msg.serialize_message();

  for (auto &user : users) {
    if (msg.username == user->name)
      continue;

    tcp_hdr_t msg_hdr(tcp_method::MESSAGE, tcp_status::SUCCESS, msg.color,
                      serialized_msg.size(), msg.username, "");

    user->send_len(&msg_hdr, sizeof(msg_hdr));
    user->send_len(serialized_msg.data(), serialized_msg.size());
  }
}

void Session::queue_msg(Message &msg) {
  boost::unique_lock<boost::mutex> msg_lock(msg_mtx);
  std::cout << "Queuing message: " << msg.text << std::endl;
  messages.push(msg);
  msg_cv.notify_one();
}

void Session::add_user(std::shared_ptr<UserSocket> user) {
  boost::unique_lock<boost::shared_mutex> brdcst_lock(usr_mtx);
  user.get()->set_color(pick_color());
  std::cout << "User added to session " << name << ": " << user->get_name()
            << std::endl;
  users.insert(user);
}

void Session::remove_user(std::shared_ptr<UserSocket> user) {
  boost::unique_lock<boost::shared_mutex> usr_lock(usr_mtx);

  std::cout << "User removed from session " << name << ": " << user->get_name()
            << std::endl;

  available_colors.insert(user->get_color());

  users.erase(user);
}

std::string Session::get_name() { return name; }

size_t Session::num_users() {
  boost::shared_lock<boost::shared_mutex> usr_lock(usr_mtx);

  return users.size();
}

color Session::pick_color() {
  if (available_colors.empty())
    return static_cast<color>(dist(engine));

  for (uint8_t i = static_cast<uint8_t>(color::RED);
       i < static_cast<uint8_t>(color::END); ++i) {

    color c = static_cast<color>(i);
    if (available_colors.contains(c)) {
      available_colors.erase(c);
      return c;
    }
  }

  return color::DEFAULT;
}
