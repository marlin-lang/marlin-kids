#ifndef marlin_ast_node_hpp
#define marlin_ast_node_hpp

#include <memory>

namespace marlin::ast {

struct base;

struct base_deleter {
  void operator()(base *b);
};

using node = std::unique_ptr<base, base_deleter>;

template <typename node_type, typename... arg_type>
node make(arg_type &&... args) {
  return node{new node_type{std::forward<arg_type>(args)...}};
}

}  // namespace marlin::ast

#endif  // marlin_ast_node_hpp