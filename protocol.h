#include <cstddef>
#include <string>

// =============================== TCP PROTOCOLS ===============================

enum class tcp_method {
  WHERE = 0x0001,
  JOIN = 0x0002,
  CREATE = 0x0003,
};

/* Request/Response Format: <tcp_hdr_t><data>
 * Expected <data> by method:
 *  - Request:
 *    - WHERE -> std::string user_name
 *    - JOIN -> uint64_t session_id
 *
 *  - Response:
 *    - WHERE -> uint64_t session_id
 *    - JOIN -> sockaddr to connect to session
 *    - CREATE -> sockaddr to connect to newly created session
 */
struct tcp_hdr_t {
  size_t hdr_len;
  tcp_method method;
  std::string user_name;
  size_t data_len;
};

// =============================== TCS PROTOCOLS ===============================

enum class tcs_method {
  CHAT = 0x0001,
  JOIN = 0x0002,
  LEAVE = 0x0003,
};

/* Request/Response Format: <tcs_hdr_t><data>
 * Expected <data> by method:
 *  - CHAT -> std::string user_name
 *
 *  - Request:
 *    - JOIN -> uint64_t session_id
 *
 *  - Response:
 *    - JOIN ->
 *    - LEAVE ->
 */
struct tcs_hdr_t {
  size_t hdr_len;
  tcs_method method;
  std::string user_name;
  size_t data_len;
};
