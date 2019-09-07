#ifndef marlin_native_func_utils_hpp
#define marlin_native_func_utils_hpp

#include <jsc/jsc.hpp>
#include <stdexcept>

#include "exec_env.hpp"

namespace marlin::control {

namespace detail {

template <typename expect_type>
std::optional<expect_type> parse_js_value(jsc::value&, jsc::context& ctx,
                                          jsc::value* exception) {
  // Not supporting interpreting javascript value as
  // arbitrary c++ object for now!
  assert(false);
  *exception = ctx.error("Internal error: unsupported native type!");
  return std::nullopt;
}

template <>
inline std::optional<double> parse_js_value<double>(jsc::value& value,
                                                    jsc::context& ctx,
                                                    jsc::value* exception) {
  auto result = value.to_number();
  if (ctx.ok()) {
    return result;
  } else {
    *exception = ctx.get_exception();
    return std::nullopt;
  }
}

template <>
inline std::optional<std::string> parse_js_value<std::string>(
    jsc::value& value, jsc::context& ctx, jsc::value* exception) {
  auto result = value.to_string();
  if (ctx.ok()) {
    return {std::move(result)};
  } else {
    *exception = ctx.get_exception();
    return std::nullopt;
  }
}

}  // namespace detail

struct native_exec_error : std::exception {
  inline native_exec_error(std::string message)
      : _message{std::move(message)} {}

  [[nodiscard]] const char* what() const noexcept override {
    return _message.data();
  }

 private:
  std::string _message;
};

template <typename object_type>
struct native_container {
  template <typename>
  struct callback {};

  template <typename... arg_type>
  native_container(exec_environment& env, const char* name, arg_type&&... args)
      : _ctx{&env.global_context()},
        _obj{_ctx->container<object_type>(std::forward<arg_type>(args)...)} {
    _ctx->root()[name] = _obj;
  }

  template <typename callable_type>
  void set_native_function(const char* name, callable_type&& func) {
    _obj[name] = _ctx->callable(std::forward<callable_type>(func));
  }

 private:
  jsc::context* _ctx;
  jsc::object _obj;
};

template <typename object_type>
template <typename return_type, typename... arg_type>
struct native_container<object_type>::callback<return_type(arg_type...)> {
  template <typename callable_type>
  struct impl {
    template <typename universal_type>
    impl(universal_type&& callable)
        : _callable{std::forward<universal_type>(callable)} {}

    jsc::value operator()(jsc::context& ctx, jsc::object this_obj,
                          std::vector<jsc::value> args,
                          jsc::value* exception) const {
      if (args.size() != sizeof...(arg_type)) {
        *exception = ctx.error("Incorrect number of arguments!");
        return ctx.undefined();
      } else {
        return parse_args<process_tag<0, arg_type...>>(args, ctx, this_obj,
                                                       exception);
      }
    }

   private:
    template <size_t index, typename... remain_type>
    struct process_tag {
      static constexpr bool finished{true};
    };

    template <typename tag, typename... curr_arg_type>
    jsc::value parse_args(std::vector<jsc::value>& args, jsc::context& ctx,
                          jsc::object& this_obj, jsc::value* exception,
                          curr_arg_type&... current_args) const {
      if constexpr (tag::finished) {
        return call(ctx, this_obj, exception, current_args...);
      } else if (auto value{detail::parse_js_value<typename tag::current>(
                     args[tag::index], ctx, exception)}) {
        return parse_args<typename tag::next>(args, ctx, this_obj, exception,
                                              current_args..., *value);
      } else {
        return ctx.undefined();
      }
    }

    jsc::value call(jsc::context& ctx, jsc::object& this_obj,
                    jsc::value* exception, arg_type&... args) const {
      assert(this_obj.is_container<object_type>());

      try {
        if constexpr (std::is_same_v<decltype(_callable(
                                         this_obj.get_contained<object_type>(),
                                         args...)),
                                     void>) {
          _callable(this_obj.get_contained<object_type>(), args...);
          return ctx.undefined();
        } else if constexpr (std::is_same_v<
                                 decltype(_callable(
                                     this_obj.get_contained<object_type>(),
                                     args...)),
                                 jsc::value>) {
          return _callable(this_obj.get_contained<object_type>(), args...);
        } else {
          return ctx.val(
              _callable(this_obj.get_contained<object_type>(), args...));
        }
      } catch (const native_exec_error& err) {
        *exception = ctx.error(err.what());
        return ctx.undefined();
      }
    }

    callable_type _callable;
  };

  template <typename callable_type>
  static auto wrapped(callable_type&& callable) {
    return impl<std::decay_t<callable_type>>(
        std::forward<callable_type>(callable));
  }
};

template <typename object_type>
template <typename return_type, typename... arg_type>
template <typename callable_type>
template <size_t _index, typename first_type, typename... remain_type>
struct native_container<object_type>::callback<return_type(arg_type...)>::impl<
    callable_type>::process_tag<_index, first_type, remain_type...> {
  static constexpr bool finished{false};

  static constexpr size_t index = _index;
  using current = first_type;
  using next = process_tag<_index + 1, remain_type...>;
};

}  // namespace marlin::control

#endif  // marlin_native_func_utils_hpp
