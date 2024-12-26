#include <cstddef>
#include <string>

// =============================== CONSTANTS ===============================
const uint16_t MAX_SESSION_NAME = 20;
const uint16_t MAX_USERNAME = 12;

// =============================== TCP PROTOCOLS ===============================

/* Request/Response Format: <tcp_hdr_t><data>
 * Expected <data> by method:
 *  - Request:
 *    - WHERE -> std::string user_name
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
  WHERE = 0x0001,
  JOIN = 0x0002,
  CREATE = 0x0003,
  CHAT = 0x0004,
  LEAVE = 0x005,
};

/* tcp_hdr_t: The header of a tcp request or response
 *  - method (short): a tcp_method determining the type of request/response
 *  - data_len (size_t): the len of the payload to follow, payload depends on
 *                       methods above
 *  - session_name (string): the session_name that this request is going to.
 *                           Only used for chat, join, leave
 *  - user_name (string): the user who sent this message
 */
struct tcp_hdr_t {
  tcp_method method;
  size_t data_len;
  char user_name[MAX_USERNAME];
  char session_name[MAX_SESSION_NAME];
};
