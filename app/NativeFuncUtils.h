// For now, only allow functions without return value

#include "native_func_utils.hpp"

#import <Foundation/Foundation.h>

void dispatch_on_main(void (^block)(void)) {
  if (NSThread.isMainThread) {
    block();
  } else {
    dispatch_sync(dispatch_get_main_queue(), block);
  }
}

template <typename GlobalEnvironment>
struct NativeEnvironment {
  struct EnvironmentContainer {
    __weak GlobalEnvironment* environment;

    EnvironmentContainer(GlobalEnvironment* _env) : environment{_env} {}
  };

  template <typename Callable>
  struct SubEnvironment {
    template <typename UniversalCallable>
    SubEnvironment(NativeEnvironment<GlobalEnvironment>& parent, UniversalCallable&& conversion)
        : _parent{&parent}, _conversion{std::forward<UniversalCallable>(conversion)} {}

    template <typename... Args, typename SubEnvironmentCallable>
    void register_native_instruction(const char* name, SubEnvironmentCallable&& callable) {
      _parent->register_native_instruction<Args...>(
          name,
          [callable{std::forward<SubEnvironmentCallable>(callable)}, conversion{_conversion}](
              auto container, Args... args) { return callable(conversion(container), args...); });
    }

   private:
    NativeEnvironment<GlobalEnvironment>* _parent;
    Callable _conversion;
  };

  NativeEnvironment(marlin::control::exec_environment& env, const char* name,
                    GlobalEnvironment* global_env)
      : _container{env, name, global_env} {}

  template <typename... Args, typename Callable>
  void register_native_instruction(const char* name, Callable&& callable) {
    _container.set_native_function(
        name, NativeContainer::template callback<void(Args...)>::wrapped(
                  [callable{std::forward<Callable>(callable)}](auto container, Args... args) {
                    dispatch_on_main(^{
                      if (auto self{container->environment}) {
                        callable(self, args...);
                      }
                    });
                  }));
  }

  template <typename Callable>
  auto makeSubEnvironment(Callable&& callable) {
    return SubEnvironment<std::decay_t<Callable>>{*this, std::forward<Callable>(callable)};
  }

 private:
  using NativeContainer = marlin::control::native_container<EnvironmentContainer>;

  NativeContainer _container;
};
