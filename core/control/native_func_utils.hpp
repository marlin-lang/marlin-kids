#ifndef marlin_native_func_utils_hpp
#define marlin_native_func_utils_hpp

#include <jsc/jsc.hpp>

#include "exec_env.hpp"

namespace marlin::control {

// for reference
using func_type = void (*)(jsc::context, jsc::object, std::vector<jsc::value>,
                           jsc::value*);

template <typename container_type>
struct native_container {
  template <typename... arg_type>
  native_container(exec_environment& env, const char* name, arg_type&&... args)
      : _ctx{&env.global_context()},
        _obj{_ctx->container<container_type>(std::forward<arg_type>(args)...)} {
    _ctx->root()[name] = _obj;
  }

  template <typename callable_type>
  void set_native_function(const char* name, callable_type&& callable) {
    _obj[name] = _ctx->callable(
        [callable{std::forward<callable_type>(callable)}](
            jsc::context& ctx, jsc::object this_obj,
            std::vector<jsc::value> args, jsc::value* exception) {
          assert(this_obj.is_container<container_type>());
          return callable(ctx, this_obj.get_contained<container_type>(),
                          std::move(args), exception);
        });
  }

 private:
  jsc::context* _ctx;
  jsc::object _obj;
};

}  // namespace marlin::control

#endif  // marlin_native_func_utils_hpp
