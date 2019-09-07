#ifndef marlin_exec_environment_hpp
#define marlin_exec_environment_hpp

#include <optional>
#include <string_view>

#include <jsc/jsc.hpp>

#include "exec_errors.hpp"
#include "generator.hpp"
#include "stacktrace.hpp"
#include "standard_lib.hpp"

namespace marlin::exec {

struct environment {
  environment() {
    _ctx.eval_script(standard_lib, "std.js");
    assert(_ctx.ok());
    _ctx.clear_exception();
  }

  // [LEGACY] (for reference)
  // Print multiple arguments will be implemented in another way later
  // -----------------------------------------------------------------
  // template <typename callback_type>
  // void register_print_callback(callback_type callback) {
  //   _ctx.root()["print"] =
  //       _ctx.callable([callback{std::forward<callback_type>(callback)}](
  //                         auto ctx, auto, auto args, auto exception) {
  //         if (args.size() == 0) {
  //           *exception = ctx.error("To few arguments!");
  //         } else {
  //           for (const auto& arg : args) {
  //             std::string str = arg.to_string();
  //             if (ctx.ok()) {
  //               callback(std::move(str));
  //             } else {
  //               *exception = ctx.get_exception();
  //               break;
  //             }
  //           }
  //         }
  //       });
  // }

  // terminate_check() shall return true for termination
  template <typename terminate_check_type>
  void register_termination_flag(terminate_check_type&& terminate_check) {
    _ctx.root()["__interrupt__"] = _ctx.callable(
        [terminate_check{std::forward<terminate_check_type>(terminate_check)}](
            auto ctx, auto, auto args, auto) {
          assert(args.size() == 0);
          return ctx.val(terminate_check());
        });
  }

  [[nodiscard]] jsc::context& global_context() { return _ctx; }

  void execute(ast::base& c, const std::string& script, bool is_async = false,
               const std::string& source_url = "<anonymous>") {
    _ctx.clear_exception();
    _ctx.eval_script(script, source_url);
    if (!_ctx.ok()) {
      const auto stack{
          _ctx.get_exception().to_object()["stack"].get().to_string()};
      auto stack_vector{parse_stacktrace(stack, source_url, c)};

      if (is_async && _ctx.root()["is_external_interrupt"]
                          .get()
                          .to_object()
                          .call(_ctx.get_exception())
                          .to_boolean()) {
        throw external_interrupt{std::move(stack_vector)};
      } else {
        throw runtime_error{_ctx.get_exception().to_string(),
                            std::move(stack_vector)};
      }
    }
  }

 private:
  jsc::context _ctx;
};

}  // namespace marlin::exec

#endif  // marlin_exec_environment_hpp
