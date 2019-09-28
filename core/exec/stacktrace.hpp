#ifndef marlin_exec_stacktrace_hpp
#define marlin_exec_stacktrace_hpp

#include <string_view>
#include <type_traits>
#include <vector>

#include "base.hpp"

namespace marlin::exec {

template <typename code_type, typename = std::enable_if_t<
                                  std::is_same_v<code_type, ast::base> ||
                                  std::is_same_v<code_type, const ast::base>>>
std::vector<code_type*> parse_stacktrace(std::string_view stacktrace,
                                         std::string_view source_url,
                                         code_type& code);

}  // namespace marlin::exec

#endif  // marlin_exec_stacktrace_hpp