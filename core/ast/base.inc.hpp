#ifndef marlin_ast_base_inc
#define marlin_ast_base_inc

namespace marlin::ast {

namespace utils {

template <typename target, size_t current_index, typename... types>
struct _type_map {};

template <typename target, size_t current_index, typename t0, typename... types>
struct _type_map<target, current_index, t0, types...> {
  static constexpr size_t index{
      _type_map<target, current_index + 1, types...>::index};
};

template <typename target, size_t current_index, typename... types>
struct _type_map<target, current_index, target, types...> {
  static constexpr size_t index{current_index};
};

template <typename target, typename... types>
struct type_map {
  static constexpr size_t index{_type_map<target, 0, types...>::index};
};

}  // namespace utils

#define ASTS(X)              \
  X(program)                 \
  X(on_start)                \
  X(function_placeholder)    \
  X(function_signature)      \
  X(parameter)               \
  X(function)                \
  X(eval_statement)          \
  X(assignment)              \
  X(use_global)              \
  X(system_procedure_call)   \
  X(if_statement)            \
  X(if_else_statement)       \
  X(while_statement)         \
  X(for_statement)           \
  X(break_statement)         \
  X(continue_statement)      \
  X(return_statement)        \
  X(return_result_statement) \
  X(variable_placeholder)    \
  X(variable_name)           \
  X(expression_placeholder)  \
  X(unary_expression)        \
  X(binary_expression)       \
  X(new_array)               \
  X(system_function_call)    \
  X(user_function_call)      \
  X(identifier)              \
  X(number_literal)          \
  X(string_literal)

#define _LIST_TEMPLATE(NAME) NAME,

template <typename node_type>
inline constexpr size_t base::get_typeid() noexcept {
  return utils::type_map<node_type, ASTS(_LIST_TEMPLATE) void>::index;
}

#define _SWITCH_CASE_TEMPLATE(NAME) \
  case get_typeid<NAME>():          \
    return callable(as<NAME>());

template <typename return_type, typename callable_type>
inline return_type base::apply(callable_type callable) {
  switch (type()) {
    default:
      /* cannot happen! */
      [[fallthrough]];
      ASTS(_SWITCH_CASE_TEMPLATE)
  }
}
template <typename return_type, typename callable_type>
inline return_type base::apply(callable_type callable) const {
  switch (type()) {
    default:
      /* cannot happen! */
      [[fallthrough]];
      ASTS(_SWITCH_CASE_TEMPLATE)
  }
}

}  // namespace marlin::ast

#endif  // marlin_ast_base_inc
