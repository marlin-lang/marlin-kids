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

  exec_environment(const exec_environment& other) = delete;
  exec_environment(exec_environment&& other)
      : _program{other._program}, _script{std::move(other._script)} {}
  exec_environment& operator=(const exec_environment& other) = delete;
  exec_environment& operator=(exec_environment&& other) {
    _program = other._program;
    _script = std::move(other._script);
    return *this;
  }

  template <typename callback_type>
  inline void add_custom_callback(const std::string& name,
                                  callback_type&& callback) {
    assert(!_in_execution);
    _env.add_custom_callback(name, std::forward<callback_type>(callback));
  }

  template <typename callback_type>
  inline void register_print_callback(callback_type&& callback) {
    assert(!_in_execution);
    _env.register_print_callback(std::forward<callback_type>(callback));
  }

  void execute() {
    _should_terminate.store(false);
    _env.register_termination_flag(
        [this]() { return _should_terminate.load(std::memory_order_acquire); });

    // For now, assume that one exec_environment supports only one run
    _in_execution = true;
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
                            std::move(_env), _program, _script};
    exec_thread.detach();
  }

  void terminate() { _should_terminate.store(true, std::memory_order_release); }

 private:
  ast::base* _program;
  std::string _script;

  bool _in_execution{false};
  exec::environment _env;

  std::atomic<bool> _should_terminate;
};

}  // namespace marlin::control

#endif  // marlin_control_exec_env_hpp
