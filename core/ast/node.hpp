#ifndef marlin_ast_node_hpp
#define marlin_ast_node_hpp

#include <memory>

namespace marlin {

namespace ast {

struct base;

struct base_deleter {
  void operator()(base *b);
};

using node = std::unique_ptr<base, base_deleter>;

}  // namespace ast

using code = ast::node;

}  // namespace marlin

#endif  // marlin_ast_node_hpp