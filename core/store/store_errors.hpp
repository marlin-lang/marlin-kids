#ifndef marlin_store_errors_hpp
#define marlin_store_errors_hpp

#include <stdexcept>
#include <string>

namespace marlin::store {

struct read_error : std::exception {
  inline read_error(std::string message) : _message{std::move(message)} {}

  [[nodiscard]] const char* what() const noexcept override {
    return _message.data();
  }

 private:
  std::string _message;
};

}  // namespace marlin::store

#endif  // marlin_store_store_hpp