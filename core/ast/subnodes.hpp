#ifndef marlin_ast_subnodes_hpp
#define marlin_ast_subnodes_hpp

namespace marlin::ast::subnode {

struct concrete {
  size_t index;
};

struct optional {
  size_t index;
  bool has_value;
};

struct vector {
  size_t index;
  size_t size;
};

}  // namespace marlin::ast::subnode

#endif  // marlin_ast_subnodes_hpp