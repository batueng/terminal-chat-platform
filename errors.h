#include <stdexcept>
#include <string>

class DuplicateSessionError : public std::runtime_error {
public:
  explicit DuplicateSessionError(std::string session_name)
      : std::runtime_error(session_name + " already exists") {}
};
