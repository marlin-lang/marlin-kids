#ifndef marlin_ast_specs_hpp
#define marlin_ast_specs_hpp

#include <array>
#include <cstdint>
#include <initializer_list>
#include <string_view>
#include <type_traits>

namespace marlin {

// Some utils

template <typename enum_type>
[[nodiscard]] inline constexpr auto raw_value(enum_type value) {
  return static_cast<std::underlying_type_t<enum_type>>(value);
}

namespace details {

template <typename specific_type, typename...>
struct array_type_helper {
  using type = specific_type;
};
template <typename... arg_types>
struct array_type_helper<void, arg_types...> : std::common_type<arg_types...> {
};

template <class specific_type, class... arg_types>
using array_type =
    std::array<typename array_type_helper<specific_type, arg_types...>::type,
               sizeof...(arg_types)>;

}  // namespace details

template <typename elem_type = void, typename... arg_types>
[[nodiscard]] inline constexpr details::array_type<elem_type, arg_types...>
make_array(arg_types&&... args) {
  return {std::forward<arg_types>(args)...};
}

namespace ast {
enum struct click_state : uint8_t { begin, move, end };

static constexpr auto click_state_name_map{make_array<std::string_view>(
    "touch_begin" /* begin */, "touch_move" /* move */, "touch_end" /* end */)};
[[nodiscard]] inline constexpr std::string_view name_for(
    click_state proc) noexcept {
  return click_state_name_map[raw_value(proc)];
}

static constexpr auto click_state_display_map{make_array<std::string_view>(
    "begin" /* begin */, "move" /* move */, "end" /* end */)};
[[nodiscard]] inline constexpr std::string_view display_for(
    click_state proc) noexcept {
  return click_state_display_map[raw_value(proc)];
}

enum struct unary_op : uint8_t { negative, logical_not };

static constexpr auto unary_op_symbol_map{
    make_array<std::string_view>("-" /* negative */, "not" /* logical_not */)};
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

static constexpr auto binary_op_symbol_map{make_array<std::string_view>(
    "+" /* add */, "-" /* subtract */, "*" /* multiply */, "/" /* divide */,
    "==" /* equal */, "!=" /* not_equal */, "<" /* less */,
    "<=" /* less_equal */, ">" /* greater */, ">=" /* greater_equal */,
    "and" /* logical_and */, "or" /* logical_or */)};
[[nodiscard]] inline constexpr std::string_view symbol_for(
    binary_op op) noexcept {
  return binary_op_symbol_map[raw_value(op)];
}

// Using JavaScript precedence for now
static constexpr auto binary_op_precedence_map{make_array<size_t>(
    11UL /* add */, 11UL /* subtract */, 12UL /* multiply */, 12UL /* divide */,
    8UL /* equal */, 8UL /* not_equal */, 9UL /* less */, 9UL /* less_equal */,
    9UL /* greater */, 9UL /* greater_equal */, 4UL /* logical_and */,
    3UL /* logical_or */)};
[[nodiscard]] inline constexpr size_t precedence_for(binary_op op) noexcept {
  return binary_op_precedence_map[raw_value(op)];
}

enum struct array_modification : size_t { append, insert, remove };

static constexpr auto array_modification_name_map{make_array<std::string_view>(
    "append" /* append */, "insert" /* insert */, "remove" /* remove */)};
[[nodiscard]] inline constexpr std::string_view name_for(
    array_modification mod) noexcept {
  return array_modification_name_map[raw_value(mod)];
}

static constexpr auto array_modification_display_map{
    make_array<std::string_view>("array.append" /* append */,
                                 "array.insert" /* insert */,
                                 "array.remove" /* remove */)};
[[nodiscard]] inline constexpr std::string_view display_for(
    array_modification mod) noexcept {
  return array_modification_display_map[raw_value(mod)];
}

enum struct system_procedure : size_t {
  sleep,
  print,
  draw_line,
  set_line_width,
  logo_forward,
  logo_backward,
  logo_turn_left,
  logo_turn_right,
  logo_pen_up,
  logo_pen_down
};

static constexpr auto system_procedure_name_map{make_array<std::string_view>(
    "sleep" /* sleep */, "print" /* print */, "draw_line" /* draw_line */,
    "set_line_width" /* set_line_width */, "logo_forward" /* logo_forward */,
    "logo_backward" /* logo_backward */, "logo_turn_left" /* logo_turn_left */,
    "logo_turn_right" /* logo_turn_right */, "logo_pen_up" /* logo_pen_up */,
    "logo_pen_down" /* logo_pen_down */)};
[[nodiscard]] inline constexpr std::string_view name_for(
    system_procedure proc) noexcept {
  return system_procedure_name_map[raw_value(proc)];
}

static constexpr auto system_procedure_display_map{make_array<std::string_view>(
    "sleep" /* sleep */, "print" /* print */, "draw_line" /* draw_line */,
    "set_line_width" /* set_line_width */, "logo.forward" /* logo_forward */,
    "logo.backward" /* logo_backward */, "logo.turn_left" /* logo_turn_left */,
    "logo.turn_right" /* logo_turn_right */, "logo.pen_up" /* logo_pen_up */,
    "logo.pen_down" /* logo_pen_down */)};
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

static constexpr auto system_function_name_map{make_array<std::string_view>(
    "range1" /* range1 */, "range2" /* range2 */, "range3" /* range3 */,
    "length" /* list_length */, "time" /* time */, "abs" /* abs */,
    "sqrt" /* sqrt */, "sin" /* sin */, "cos" /* cos */, "tan" /* tan */,
    "asin" /* asin */, "acos" /* acos */, "atan" /* atan */, "ln" /* ln */,
    "log" /* log */, "round" /* round */, "floor" /* floor */,
    "ceil" /* ceil */)};
[[nodiscard]] inline constexpr std::string_view name_for(
    system_function func) noexcept {
  return system_function_name_map[raw_value(func)];
}

static constexpr auto system_function_display_map{make_array<std::string_view>(
    "range" /* range1 */, "range" /* range2 */, "range" /* range3 */,
    "length" /* list_length */, "time" /* time */, "abs" /* abs */,
    "sqrt" /* sqrt */, "sin" /* sin */, "cos" /* cos */, "tan" /* tan */,
    "asin" /* asin */, "acos" /* acos */, "atan" /* atan */, "ln" /* ln */,
    "log" /* log */, "round" /* round */, "floor" /* floor */,
    "ceil" /* ceil */)};
[[nodiscard]] inline constexpr std::string_view display_for(
    system_function func) noexcept {
  return system_function_display_map[raw_value(func)];
}

enum struct color_mode : uint8_t { rgb, rgba };

static constexpr auto color_mode_name_map{
    make_array<std::string_view>("rgb" /* rgb */, "rgba" /* rgba */)};
[[nodiscard]] inline constexpr std::string_view name_for(
    color_mode mode) noexcept {
  return color_mode_name_map[raw_value(mode)];
}

static constexpr auto color_mode_display_map{
    make_array<std::string_view>("rgb" /* rgb */, "rgba" /* rgba */)};
[[nodiscard]] inline constexpr std::string_view display_for(
    color_mode mode) noexcept {
  return color_mode_display_map[raw_value(mode)];
}

}  // namespace ast

}  // namespace marlin

#endif  // marlin_ast_specs_hpp
