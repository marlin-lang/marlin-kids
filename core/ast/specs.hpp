#ifndef marlin_ast_specs_hpp
#define marlin_ast_specs_hpp

#include <array>
#include <cstdint>

namespace marlin::ast {

enum class unary_op : uint8_t { positive, negative };

enum class binary_op : uint8_t { add, subtract, multiply, divide };

}  // namespace marlin::ast

#endif  // marlin_ast_specs_hpp