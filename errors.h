#ifndef ERROR_H
#define ERROR_H

#include "protocol.h"
#include <stdexcept>
#include <string>

// ================================= BASE CLASS ================================

class TCPError : public std::runtime_error {
public:
  TCPError(std::string msg, tcp_status _status) : std::runtime_error(msg) {
    status = _status;
  }

  tcp_status get_status() { return status; }

protected:
  tcp_status status;
};

// ============================= Duplicate Classes =============================

class DuplicateObjectError : public TCPError {
public:
  DuplicateObjectError(std::string key, tcp_status _status)
      : TCPError(key + " already exists.", _status) {}
};

class DuplicateSession : public DuplicateObjectError {
public:
  explicit DuplicateSession(std::string session_name)
      : DuplicateObjectError(session_name, tcp_status::DUP_SESS) {}
};

class DuplicateUser : public DuplicateObjectError {
public:
  explicit DuplicateUser(std::string user_name)
      : DuplicateObjectError(user_name, tcp_status::DUP_USER) {}
};

// ============================= Not Found Classes =============================

class NotFoundError : public TCPError {
public:
  NotFoundError(std::string key, tcp_status _status)
      : TCPError(key + " not found.", _status) {}
};

class SessionNotFound : public NotFoundError {
public:
  explicit SessionNotFound(std::string session_name)
      : NotFoundError(session_name, tcp_status::SESSION_NOT_FOUND) {}
};

class UserNotFound : public NotFoundError {
public:
  explicit UserNotFound(std::string user_name)
      : NotFoundError(user_name, tcp_status::USER_NOT_FOUND) {}
};

// ============================ Invalid Name Classes ===========================

class InvalidUsername : public TCPError {
public:
  explicit InvalidUsername(std::string username)
      : TCPError(
            "Username: " + username +
                " is invalid. Must have length > 0 and contain no whitespace.",
            tcp_status::INVALID_UNAME) {}
};

class InvalidSessionName : public TCPError {
public:
  explicit InvalidSessionName(std::string session_name)
      : TCPError("Session name: " + session_name +
                     " is invalid. Must have length > 0 and contain no "
                     "whitespace.",
                 tcp_status::INVALID_SESS_NAME) {}
};

#endif
