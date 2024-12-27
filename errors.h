#include <stdexcept>
#include <string>

class DuplicateSessionError : public std::runtime_error {
public:
  explicit DuplicateSessionError(std::string session_name)
      : std::runtime_error(session_name + " already exists.") {}
};

class SessionNotFound : public std::runtime_error {
public:
  explicit SessionNotFound(std::string session_name)
      : std::runtime_error(session_name + " does not exist.") {}
};

class UserNotFound : public std::runtime_error {
public:
  explicit UserNotFound(std::string user_name)
      : std::runtime_error(user_name + " does not exist.") {}
};
