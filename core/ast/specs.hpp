#ifndef marlin_ast_specs_hpp
#define marlin_ast_specs_hpp

#include <array>
#include <cstdint>

namespace marlin::ast {

enum class unary_op : uint8_t { positive, negative };

static constexpr std::array _unary_op_length_map{
    1UL /* positive */, 1UL /* negative */
};
[[nodiscard]] inline constexpr size_t length_of(unary_op op) noexcept {
  return _unary_op_length_map[static_cast<uint8_t>(op)];
}

enum class binary_op : uint8_t { add, subtract, multiply, divide };

static constexpr std::array _binary_op_length_map{
    1UL /* add */, 1UL /* subtract */, 1UL /* multiply */, 1UL /* divide */
};
[[nodiscard]] inline constexpr size_t length_of(binary_op op) noexcept {
  return _binary_op_length_map[static_cast<uint8_t>(op)];
}

}  // namespace marlin::ast

#endif  // marlin_ast_specs_hpp