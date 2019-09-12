#ifndef marlin_exec_errors_hpp
#define marlin_exec_errors_hpp

#include <stdexcept>

#include "base.hpp"
#include "utils.hpp"

namespace marlin::exec {

struct execution_clash_error : std::exception {};

// Generation errors will only be reported using
// collected_generation_error

struct generation_error : std::exception {
  inline generation_error(std::string message, ast::base& node)
      : _message{std::move(message)}, _node{&node} {}

  [[nodiscard]] const char* what() const noexcept override {
    return _message.data();
  }

  [[nodiscard]] inline ast::base& node() const noexcept { return *_node; }

 private:
  std::string _message;
  ast::base* _node;
};

struct collected_generation_error : std::exception {
  inline collected_generation_error(std::vector<generation_error> errors)
      : _errors{std::move(errors)} {}

  [[nodiscard]] const char* what() const noexcept override {
    return "Error occurred during generation of executable!";
  }

  [[nodiscard]] inline utils::vector_view<std::vector<generation_error>>
  errors() noexcept {
    return _errors;
  }

 private:
  std::vector<generation_error> _errors;
};

};  // namespace marlin::exec

#endif  // marlin_parse_errors_hpp
