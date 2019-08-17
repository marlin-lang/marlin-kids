#ifndef marlin_exec_standard_lib_hpp
#define marlin_exec_standard_lib_hpp

#include <string>

namespace marlin::exec {

inline const std::string standard_lib{
    R"std(
  function* range(end) {
    for (var i = 0; i < end; i++) {
      yield i;
    }
  }
)std"};

};  // namespace marlin::exec

#endif  // marlin_exec_standard_lib_hpp