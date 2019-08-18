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

struct runtime_error : std::exception {
  inline runtime_error(std::string message, std::vector<ast::base*> stack)
      : _message{std::move(message)}, _stack{std::move(stack)} {}

  [[nodiscard]] const char* what() const noexcept override {
    return _message.data();
  }

  [[nodiscard]] inline size_t stack_depth() const { return _stack.size(); }
  [[nodiscard]] inline ast::base& stack(size_t index) const noexcept {
    return *_stack[index];
  }

 private:
  std::string _message;
  std::vector<ast::base*> _stack;
};

struct external_interrupt : std::exception {
  inline external_interrupt(std::vector<ast::base*> stack)
      : _stack{std::move(stack)} {}

  [[nodiscard]] const char* what() const noexcept override {
    return "External interrupt received!";
  }

  [[nodiscard]] inline size_t stack_depth() const { return _stack.size(); }
  [[nodiscard]] inline ast::base& stack(size_t index) const noexcept {
    return *_stack[index];
  }

 private:
  std::vector<ast::base*> _stack;
};

};  // namespace marlin::exec

#endif  // marlin_parse_errors_hpp
