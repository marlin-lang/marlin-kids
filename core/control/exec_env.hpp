#ifndef marlin_control_exec_env_hpp
#define marlin_control_exec_env_hpp

#include <atomic>
#include <memory>
#include <string_view>
#include <thread>

#include "base.hpp"
#include "environment.hpp"
#include "generator.hpp"

namespace marlin::control {

struct exec_environment {
  exec_environment(ast::base& program)
      : _program{&program}, _script{[&program]() {
          exec::generator gen{true};
          return gen.generate(program);
        }()} {}

  exec_environment(const exec_environment& other) = delete;
  exec_environment(exec_environment&& other)
      : _program{other._program}, _script{std::move(other._script)} {}
  exec_environment& operator=(const exec_environment& other) = delete;
  exec_environment& operator=(exec_environment&& other) {
    _program = other._program;
    _script = std::move(other._script);
    return *this;
  }

  [[nodiscard]] jsc::context& global_context() { return _env.global_context(); }

  void execute() {
    assert(!_in_execution);
    _should_terminate->store(false);
    _env.register_termination_flag([tflag{_should_terminate}]() {
      return tflag->load(std::memory_order_acquire);
    });

    // For now, assume that one exec_environment supports only one run
    _in_execution = true;
    std::thread exec_thread{[](marlin::exec::environment env,
                               ast::base* program, const std::string& script) {
                              try {
                                env.execute(*program, script, true);
                              } catch (const exec::runtime_error& e) {
                                // TODO: unimplemented
                                assert(false);
                              } catch (const exec::external_interrupt&) {
                                // Do nothing
                              }
                            },
                            std::move(_env), _program, _script};
    exec_thread.detach();
  }

  void terminate() {
    _should_terminate->store(true, std::memory_order_release);
  }

 private:
  ast::base* _program;
  std::string _script;

  bool _in_execution{false};
  // _env is valid only before execution starts, i.e. _in_execution == false
  // Upon execution, its value is moved to the execution thread
  exec::environment _env;

  std::shared_ptr<std::atomic<bool>> _should_terminate{
      std::make_shared<std::atomic<bool>>(false)};
};

}  // namespace marlin::control

#endif  // marlin_control_exec_env_hpp
