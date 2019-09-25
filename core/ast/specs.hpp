#ifndef marlin_ast_specs_hpp
#define marlin_ast_specs_hpp

#include <array>
#include <cstdint>
#include <string_view>
#include <type_traits>

namespace marlin {

template <typename enum_type>
[[nodiscard]] inline constexpr auto raw_value(enum_type value) {
  return static_cast<std::underlying_type_t<enum_type>>(value);
}

namespace ast {

enum struct click_state : uint8_t { begin, move, end };

static constexpr std::array<std::string_view, 3> click_state_name_map{
    "touch_begin" /* begin */, "touch_move" /* move */, "touch_end" /* end */
};
[[nodiscard]] inline constexpr std::string_view name_for(
    click_state proc) noexcept {
  return click_state_name_map[raw_value(proc)];
}

static constexpr std::array<std::string_view, 3> click_state_display_map{
    "begin" /* begin */, "move" /* move */, "end" /* end */};
[[nodiscard]] inline constexpr std::string_view display_for(
    click_state proc) noexcept {
  return click_state_display_map[raw_value(proc)];
}

enum struct unary_op : uint8_t { negative, logical_not };

static constexpr std::array<std::string_view, 2> unary_op_symbol_map{
    "-" /* negative */, "not" /* logical_not */
};
[[nodiscard]] inline constexpr std::string_view symbol_for(
    unary_op op) noexcept {
  return unary_op_symbol_map[raw_value(op)];
}

// Using JavaScript precedence for now
static constexpr size_t unary_op_precedence{14UL};

enum struct binary_op : uint8_t {
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
  return binary_op_symbol_map[raw_value(op)];
}

// Using JavaScript precedence for now
static constexpr std::array binary_op_precedence_map{
    11UL /* add */,          11UL /* subtract */,   12UL /* multiply */,
    12UL /* divide */,       8UL /* equal */,       8UL /* not_equal */,
    9UL /* less */,          9UL /* less_equal */,  9UL /* greater */,
    9UL /* greater_equal */, 4UL /* logical_and */, 3UL /* logical_or */
};
[[nodiscard]] inline constexpr size_t precedence_for(binary_op op) noexcept {
  return binary_op_precedence_map[raw_value(op)];
}

enum struct system_procedure : size_t {
  sleep,
  print,
  list_append,
  list_insert,
  list_remove,
  draw_line,
  set_line_width,
  logo_forward,
  logo_backward,
  logo_turn_left,
  logo_turn_right,
  logo_pen_up,
  logo_pen_down
};

static constexpr std::array<std::string_view, 13> system_procedure_name_map{
    "sleep" /* sleep */,
    "print" /* print */,
    "append" /* list_append */,
    "insert" /* list_insert */,
    "remove" /* list_remove */,
    "draw_line" /* draw_line */,
    "set_line_width" /* set_line_width */,
    "logo_forward" /* logo_forward */,
    "logo_backward" /* logo_backward */,
    "logo_turn_left" /* logo_turn_left */,
    "logo_turn_right" /* logo_turn_right */,
    "logo_pen_up" /* logo_pen_up */,
    "logo_pen_down" /* logo_pen_down */};
[[nodiscard]] inline constexpr std::string_view name_for(
    system_procedure proc) noexcept {
  return system_procedure_name_map[raw_value(proc)];
}

static constexpr std::array<std::string_view, 13> system_procedure_display_map{
    "sleep" /* sleep */,
    "print" /* print */,
    "append" /* list_append */,
    "insert" /* list_insert */,
    "remove" /* list_remove */,
    "draw_line" /* draw_line */,
    "set_line_width" /* set_line_width */,
    "logo.forward" /* logo_forward */,
    "logo.backward" /* logo_backward */,
    "logo.turn_left" /* logo_turn_left */,
    "logo.turn_right" /* logo_turn_right */,
    "logo.pen_up" /* logo_pen_up */,
    "logo.pen_down" /* logo_pen_down */};
[[nodiscard]] inline constexpr std::string_view display_for(
    system_procedure proc) noexcept {
  return system_procedure_display_map[raw_value(proc)];
}

enum struct system_function : size_t {
  range1,
  range2,
  range3,
  list_length,
  time,
  abs,
  sqrt,
  sin,
  cos,
  tan,
  asin,
  acos,
  atan,
  ln,
  log,
  round,
  floor,
  ceil
};

static constexpr std::array<std::string_view, 18> system_function_name_map{
    "range1" /* range1 */, "range2" /* range2 */,
    "range3" /* range3 */, "length" /* list_length */,
    "time" /* time */,     "abs" /* abs */,
    "sqrt" /* sqrt */,     "sin" /* sin */,
    "cos" /* cos */,       "tan" /* tan */,
    "asin" /* asin */,     "acos" /* acos */,
    "atan" /* atan */,     "ln" /* ln */,
    "log" /* log */,       "round" /* round */,
    "floor" /* floor */,   "ceil" /* ceil */
};
[[nodiscard]] inline constexpr std::string_view name_for(
    system_function func) noexcept {
  return system_function_name_map[raw_value(func)];
}

static constexpr std::array<std::string_view, 18> system_function_display_map{
    "range" /* range1 */, "range" /* range2 */,
    "range" /* range3 */, "length" /* list_length */,
    "time" /* time */,    "abs" /* abs */,
    "sqrt" /* sqrt */,    "sin" /* sin */,
    "cos" /* cos */,      "tan" /* tan */,
    "asin" /* asin */,    "acos" /* acos */,
    "atan" /* atan */,    "ln" /* ln */,
    "log" /* log */,      "round" /* round */,
    "floor" /* floor */,  "ceil" /* ceil */
};
[[nodiscard]] inline constexpr std::string_view display_for(
    system_function func) noexcept {
  return system_function_display_map[raw_value(func)];
}

enum struct color_mode : uint8_t { rgb, rgba };

static constexpr std::array<std::string_view, 2> color_mode_name_map{
    "rgb" /* rgb */, "rgba" /* rgba */
};
[[nodiscard]] inline constexpr std::string_view name_for(
    color_mode mode) noexcept {
  return color_mode_name_map[raw_value(mode)];
}

static constexpr std::array<std::string_view, 2> color_mode_display_map{
    "rgb" /* rgb */, "rgba" /* rgba */
};
[[nodiscard]] inline constexpr std::string_view display_for(
    color_mode mode) noexcept {
  return color_mode_display_map[raw_value(mode)];
}

}  // namespace ast

}  // namespace marlin

#endif  // marlin_ast_specs_hpp
