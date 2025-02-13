#ifndef PROTOCOLS
#define PROTOCOLS

#include <boost/thread/mutex.hpp>
#include <cstddef>
#include <ncurses.h>
#include <string>
#include <vector>

// ================================= CONSTANTS =================================
//
const uint16_t MAX_SESSION_NAME = 20;
const uint16_t MAX_USERNAME = 12;

enum class color : int8_t {
  DEFAULT = -1,
  RED = COLOR_RED,
  GREEN = COLOR_GREEN,
  YELLOW = COLOR_YELLOW,
  BLUE = COLOR_BLUE,
  MAGENTA = COLOR_MAGENTA,
  CYAN = COLOR_CYAN,
  WHITE = COLOR_WHITE,
  END = 8
};

// =============================== TCP PROTOCOLS ===============================

/* Request/Response Format: <tcp_hdr_t><data>
 * Expected <data> by method:
 *  - Request:
 *    - WHERE -> std::string username
 *    - JOIN -> uint64_t session_id
 *    - CREATE -> std::string session_name
 *    - CHAT -> std::string message
 *
 *  - Response:
 *    - WHERE -> uint64_t session_id
 *    - JOIN -> sockaddr to connect to session
 *    - CREATE -> sockaddr to connect to newly created session
 *    - CHAT -> std::string message
 */
enum class tcp_method : uint16_t {
  U_NAME = 0x00,
  WHERE = 0x01,
  JOIN = 0x02,
  CREATE = 0x03,
  MESSAGE = 0x04,
  LEAVE = 0x05,
  ERROR = 0x06
};

enum class tcp_status {
  SUCCESS = 200,
  USER_NOT_FOUND = 301,
  SESSION_NOT_FOUND = 302,
  DUP_USER = 401,
  DUP_SESS = 402,
};

/* tcp_hdr_t: The header of a tcp request or response
 *  - method (short): a tcp_method determining the type of request/response
 *  - data_len (size_t): the len of the payload to follow, payload depends on
 *                       methods above
 *  - username (string): the user who sent this message
 *  - session_name (string): the session_name that this request is going to.
 *                           Only used for chat, join, leave
 */
struct tcp_hdr_t {
  tcp_method method;
  tcp_status status;
  color c = color::DEFAULT;
  size_t data_len;
  char username[MAX_USERNAME];
  char session_name[MAX_SESSION_NAME];
};

// ============================= MESSAGE PROTOCOLS =============================

enum class message_type : uint8_t {
  CHAT = 0,
  USER_JOIN = 1,
  USER_LEFT = 2,
};

struct Message {
  message_type msg_t;

  std::string username;

  color color;

  std::string text;

  std::vector<char> serialize_message();

  static Message deserialize_message(const std::vector<char> &data);
};

extern boost::mutex cout_mtx;

#endif
