#include <cstddef>
#include <string>

// =============================== TCP PROTOCOLS ===============================

enum class tcp_method {
  WHERE = 0x0001,
  JOIN = 0x0002,
  CREATE = 0x0003,
  CHAT = 0x0004
};

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
struct tcp_hdr_t {
  tcp_method method;
  std::string user_name;
  size_t data_len;
  uint64_t session_id;
};
