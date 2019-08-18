#ifndef marlin_control_exec_env_hpp
#define marlin_control_exec_env_hpp

#include <atomic>
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

  exec_environment(const exec_environment& other)
      : _program{other._program}, _script{other._script} {}
  exec_environment(exec_environment&& other)
      : _program{other._program}, _script{std::move(other._script)} {}
  exec_environment& operator=(const exec_environment& other) {
    _program = other._program;
    _script = other._script;
    return *this;
  }
  exec_environment& operator=(exec_environment&& other) {
    _program = other._program;
    _script = std::move(other._script);
    return *this;
  }

  template <typename print_callback_type>
  void execute(print_callback_type&& print_callback) {
    _should_terminate.store(false);
    marlin::exec::environment env;
    env.register_print_callback(
        std::forward<print_callback_type>(print_callback));
    env.register_termination_flag(
        [this]() { return _should_terminate.load(std::memory_order_acquire); });

    std::thread exec_thread{[](marlin::exec::environment env,
                               ast::base* program, const std::string& script) {
                              try {
                                env.execute(*program, script, true);
                              } catch (const exec::runtime_error&) {
                                // TODO: unimplemented
                                assert(false);
                              } catch (const exec::external_interrupt&) {
                                // Do nothing
                              }
                            },
                            std::move(env), _program, _script};
    exec_thread.detach();
  }

  void terminate() { _should_terminate.store(true, std::memory_order_release); }

 private:
  ast::base* _program;
  std::string _script;

  std::atomic<bool> _should_terminate;
};  // namespace marlin::control

}  // namespace marlin::control

#endif  // marlin_control_exec_env_hpp
