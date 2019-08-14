#ifndef marlin_exec_environment_hpp
#define marlin_exec_environment_hpp

#include <optional>

#include <jsc/jsc.hpp>

#include "exec_errors.hpp"
#include "generator.hpp"
#include "stacktrace.hpp"

namespace marlin::exec {

struct environment {
  inline environment() {
    register_print_callback([](auto) {});
  }

  template <typename callback_type>
  inline void register_print_callback(callback_type callback) {
    _ctx.root()["print"] =
        _ctx.callable([callback{std::move(callback)}](auto ctx, auto, auto args,
                                                      auto exception) {
          if (args.size() == 0) {
            *exception = ctx.error("To few arguments!");
          } else {
            for (const auto& arg : args) {
              std::string str = arg.to_string();
              if (ctx.ok()) {
                callback(std::move(str));
              } else {
                *exception = ctx.get_exception();
                break;
              }
            }
          }
        });
  }

  inline void execute(ast::base& c,
                      const std::string& source_url = "<anonymous>") {
    generator gen;
    const auto javascript = gen.generate(c);
    _ctx.clear_exception();
    _ctx.eval_script(javascript, source_url);
    if (!_ctx.ok()) {
      std::string stack{
          _ctx.get_exception().to_object()["stack"].get().to_string()};
      throw runtime_error{_ctx.get_exception().to_string(),
                          parse_stacktrace(stack, source_url, c)};
    }
  }

 private:
  jsc::context _ctx;
};

}  // namespace marlin::exec

#endif  // marlin_exec_environment_hpp