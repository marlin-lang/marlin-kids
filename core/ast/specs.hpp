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

enum class binary_op : uint8_t {
  add,
  subtract,
  multiply,
  divide,
  equal,
  not_equal,
  less,
  less_equal,
  greater,
  greater_equal,
  logical_and,
  logical_or
};

static constexpr std::array<std::string_view, 12> binary_op_symbol_map{
    "+" /* add */,           "-" /* subtract */,
    "*" /* multiply */,      "/" /* divide */,
    "==" /* equal */,        "!=" /* not_equal */,
    "<" /* less */,          "<=" /* less_equal */,
    ">" /* greater */,       ">=" /* greater_equal */,
    "and" /* logical_and */, "or" /* logical_or */
};
[[nodiscard]] inline constexpr std::string_view symbol_for(
    binary_op op) noexcept {
  return binary_op_symbol_map[static_cast<uint8_t>(op)];
}

// Using JavaScript precedence for now
static constexpr std::array binary_op_precedence_map{
    11UL /* add */,          11UL /* subtract */,   12UL /* multiply */,
    12UL /* divide */,       8UL /* equal */,       8UL /* not_equal */,
    9UL /* less */,          9UL /* less_equal */,  9UL /* greater */,
    9UL /* greater_equal */, 4UL /* logical_and */, 3UL /* logical_or */
};
[[nodiscard]] inline constexpr size_t precedence_for(binary_op op) noexcept {
  return binary_op_precedence_map[static_cast<uint8_t>(op)];
}

enum class system_procedure : size_t {
  sleep,
  print,
  draw_line,
  logo_forward,
  logo_backward,
  logo_turn_left,
  logo_turn_right,
  logo_pen_up,
  logo_pen_down
};

static constexpr std::array<std::string_view, 9> system_procedure_name_map{
    "sleep" /* sleep */,
    "print" /* print */,
    "draw_line" /* draw_line */,
    "logo_forward" /* logo_forward */,
    "logo_backward" /* logo_backward */,
    "logo_turn_left" /* logo_turn_left */,
    "logo_turn_right" /* logo_turn_right */,
    "logo_pen_up" /* logo_pen_up */,
    "logo_pen_down" /* logo_pen_down */};
[[nodiscard]] inline constexpr std::string_view name_for(
    system_procedure proc) noexcept {
  return system_procedure_name_map[static_cast<size_t>(proc)];
}

static constexpr std::array<std::string_view, 9> system_procedure_display_map{
    "sleep" /* sleep */,
    "print" /* print */,
    "draw_line" /* draw_line */,
    "logo.forward" /* logo_forward */,
    "logo.backward" /* logo_backward */,
    "logo.turn_left" /* logo_turn_left */,
    "logo.turn_right" /* logo_turn_right */,
    "logo.pen_up" /* logo_pen_up */,
    "logo.pen_down" /* logo_pen_down */};
[[nodiscard]] inline constexpr std::string_view display_for(
    system_procedure proc) noexcept {
  return system_procedure_display_map[static_cast<size_t>(proc)];
}

enum class system_function : size_t { range1, range2, range3, time, sin, cos };

static constexpr std::array<std::string_view, 6> system_function_name_map{
    "range1" /* range1 */, "range2" /* range2 */, "range3" /* range3 */,
    "time" /* time */,     "sin" /* sin */,       "cos" /* cos */
};
[[nodiscard]] inline constexpr std::string_view name_for(
    system_function func) noexcept {
  return system_function_name_map[static_cast<size_t>(func)];
}

static constexpr std::array<std::string_view, 6> system_function_display_map{
    "range" /* range1 */, "range" /* range2 */, "range" /* range3 */,
    "time" /* time */,    "sin" /* sin */,      "cos" /* cos */
};
[[nodiscard]] inline constexpr std::string_view display_for(
    system_function func) noexcept {
  return system_function_display_map[static_cast<size_t>(func)];
}

}  // namespace marlin::ast

#endif  // marlin_ast_specs_hpp
