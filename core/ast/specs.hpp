#ifndef marlin_ast_specs_hpp
#define marlin_ast_specs_hpp

#include <array>
#include <cstdint>

namespace marlin::ast {

enum class unary_op : uint8_t { positive, negative };

enum class binary_op : uint8_t { add, subtract, multiply, divide };

static constexpr std::array _binary_op_symbol_map{
    "+" /* add */, "-" /* subtract */, "*" /* multiply */, "/" /* divide */
};
[[nodiscard]] inline constexpr const char* symbol_for(binary_op op) noexcept {
  return _binary_op_symbol_map[static_cast<uint8_t>(op)];
}

// Using JavaScript precedence for now
static constexpr std::array _binary_op_precedence_map{
    11UL /* add */, 11UL /* subtract */, 12UL /* multiply */, 12UL /* divide */
};
[[nodiscard]] inline constexpr size_t precedence_for(binary_op op) noexcept {
  return _binary_op_precedence_map[static_cast<uint8_t>(op)];
}

}  // namespace marlin::ast

#endif  // marlin_ast_specs_hpp