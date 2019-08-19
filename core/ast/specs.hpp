#ifndef marlin_ast_specs_hpp
#define marlin_ast_specs_hpp

#include <array>
#include <cstdint>
#include <string_view>

namespace marlin::ast {

enum class unary_op : uint8_t { negative };

static constexpr std::array<std::string_view, 1> unary_op_symbol_map{
    "-" /* negative */
};
[[nodiscard]] inline constexpr std::string_view symbol_for(
    unary_op op) noexcept {
  return unary_op_symbol_map[static_cast<uint8_t>(op)];
}

// Using JavaScript precedence for now
static constexpr size_t unary_op_precedence{14UL};

enum class binary_op : uint8_t { add, subtract, multiply, divide };

static constexpr std::array<std::string_view, 4> binary_op_symbol_map{
    "+" /* add */, "-" /* subtract */, "*" /* multiply */, "/" /* divide */
};
[[nodiscard]] inline constexpr std::string_view symbol_for(
    binary_op op) noexcept {
  return binary_op_symbol_map[static_cast<uint8_t>(op)];
}

// Using JavaScript precedence for now
static constexpr std::array binary_op_precedence_map{
    11UL /* add */, 11UL /* subtract */, 12UL /* multiply */, 12UL /* divide */
};
[[nodiscard]] inline constexpr size_t precedence_for(binary_op op) noexcept {
  return binary_op_precedence_map[static_cast<uint8_t>(op)];
}

enum class system_procedure : size_t {
  draw_line,
  logo_forward,
  logo_backward,
  logo_turn_left,
  logo_turn_right
};

static constexpr std::array<std::string_view, 5> system_procedure_name_map{
    "draw_line" /* draw_line */, "logo_forward" /* logo_forward */,
    "logo_backward" /* logo_backward */, "logo_turn_left" /* logo_turn_left */,
    "logo_turn_right" /* logo_turn_right */};
[[nodiscard]] inline constexpr std::string_view name_for(
    system_procedure proc) noexcept {
  return system_procedure_name_map[static_cast<size_t>(proc)];
}

enum class system_function : size_t { range1, range2, range3, time };

static constexpr std::array<std::string_view, 4> system_function_name_map{
    "range1" /* range1 */, "range2" /* range2 */, "range3" /* range3 */,
    "time" /* time */
};
[[nodiscard]] inline constexpr std::string_view name_for(
    system_function func) noexcept {
  return system_function_name_map[static_cast<size_t>(func)];
}

}  // namespace marlin::ast

#endif  // marlin_ast_specs_hpp