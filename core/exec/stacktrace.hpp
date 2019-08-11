#ifndef marlin_exec_stacktrace_hpp
#define marlin_exec_stacktrace_hpp

#include <type_traits>
#include <vector>

#include "base.hpp"

namespace marlin::exec {

template <typename code_type, typename = std::enable_if_t<
                                  std::is_same_v<code_type, ast::base> ||
                                  std::is_same_v<code_type, const ast::base>>>
std::vector<code_type*> parse_stacktrace(const std::string& stacktrace,
                                         const std::string& source_url,
                                         code_type& code);
}

#endif  // marlin_exec_stacktrace_hpp