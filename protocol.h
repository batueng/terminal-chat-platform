#ifndef PROTOCOLS
#define PROTOCOLS

#include <cstddef>
#include <string>
#include <vector>

// ================================= CONSTANTS =================================
//
const uint16_t MAX_SESSION_NAME = 20;
const uint16_t MAX_USERNAME = 12;

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
enum class tcp_method {
  U_NAME = 0x000,
  WHERE = 0x0001,
  JOIN = 0x0002,
  CREATE = 0x0003,
  MESSAGE = 0x0004,
  LEAVE = 0x005,
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
  std::string text;

  std::vector<char> serialize_message();

  static Message deserialize_message(const std::vector<char> &data);
};
#endif
