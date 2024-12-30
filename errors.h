#include <stdexcept>
#include <string>

// ============================= Duplicate Classes =============================
class DuplicateObjectError : public std::runtime_error {
public:
  explicit DuplicateObjectError(std::string key)
      : std::runtime_error(key + " already exists.") {}
};

class DuplicateSession : public DuplicateObjectError {
public:
  explicit DuplicateSession(std::string session_name)
      : DuplicateObjectError(session_name) {}
};

class DuplicateUser : public DuplicateObjectError {
public:
  explicit DuplicateUser(std::string user_name)
      : DuplicateObjectError(user_name) {}
};

// ============================= Not Found Classes =============================
class NotFoundError : public std::runtime_error {
public:
  explicit NotFoundError(std::string key)
      : std::runtime_error(key + " already exists.") {}
};

class SessionNotFound : public NotFoundError {
public:
  explicit SessionNotFound(std::string session_name)
      : NotFoundError(session_name) {}
};

class UserNotFound : NotFoundError {
public:
  explicit UserNotFound(std::string user_name) : NotFoundError(user_name) {}
};
