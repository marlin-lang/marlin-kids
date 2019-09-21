#ifndef marlin_ast_ast_impl
#define marlin_ast_ast_impl

#include <string>

#include "base.impl.hpp"
#include "function_definition.hpp"
#include "specs.hpp"

namespace marlin::ast {

struct block {};
struct statement {};
struct expression {};
struct lvalue {};

struct program : base::impl<program, subnode::vector> {
  [[nodiscard]] decltype(auto) blocks() { return get_subnode<0>(); }
  [[nodiscard]] decltype(auto) blocks() const { return get_subnode<0>(); }

  using base_type::impl;
};

struct on_start : base::impl<on_start, subnode::vector>, block {
  [[nodiscard]] decltype(auto) statements() { return get_subnode<0>(); }
  [[nodiscard]] decltype(auto) statements() const { return get_subnode<0>(); }

  using base_type::impl;
};

struct function_placeholder
    : base::impl<function_placeholder, subnode::vector> {
  std::string name;

  [[nodiscard]] decltype(auto) parameters() { return get_subnode<0>(); }
  [[nodiscard]] decltype(auto) parameters() const { return get_subnode<0>(); }

  explicit function_placeholder(std::string _name, std::vector<node> _args)
      : base_type{std::move(_args)}, name{std::move(_name)} {}
};

struct function_signature : base::impl<function_signature, subnode::vector> {
  std::string name;

  [[nodiscard]] decltype(auto) parameters() { return get_subnode<0>(); }
  [[nodiscard]] decltype(auto) parameters() const { return get_subnode<0>(); }

  explicit function_signature(std::string _name, std::vector<node> _args)
      : base_type{std::move(_args)}, name{std::move(_name)} {}
};

struct function : base::impl<function, subnode::concrete, subnode::vector>,
                  block {
  [[nodiscard]] decltype(auto) signature() { return get_subnode<0>(); }
  [[nodiscard]] decltype(auto) signature() const { return get_subnode<0>(); }

  [[nodiscard]] decltype(auto) statements() { return get_subnode<1>(); }
  [[nodiscard]] decltype(auto) statements() const { return get_subnode<1>(); }

  using base_type::impl;
};

struct eval_statement : base::impl<eval_statement, subnode::concrete>,
                        statement {
  [[nodiscard]] decltype(auto) expression() { return get_subnode<0>(); }
  [[nodiscard]] decltype(auto) expression() const { return get_subnode<0>(); }

  using base_type::impl;
};

struct assignment
    : base::impl<assignment, subnode::concrete, subnode::concrete>,
      statement {
  [[nodiscard]] decltype(auto) variable() { return get_subnode<0>(); }
  [[nodiscard]] decltype(auto) variable() const { return get_subnode<0>(); }

  [[nodiscard]] decltype(auto) value() { return get_subnode<1>(); }
  [[nodiscard]] decltype(auto) value() const { return get_subnode<1>(); }

  using base_type::impl;
};

struct use_global : base::impl<use_global, subnode::concrete>, statement {
  [[nodiscard]] decltype(auto) variable() { return get_subnode<0>(); }
  [[nodiscard]] decltype(auto) variable() const { return get_subnode<0>(); }

  using base_type::impl;
};

struct system_procedure_call
    : base::impl<system_procedure_call, subnode::vector>,
      statement {
  system_procedure proc;

  [[nodiscard]] decltype(auto) arguments() { return get_subnode<0>(); }
  [[nodiscard]] decltype(auto) arguments() const { return get_subnode<0>(); }

  explicit system_procedure_call(system_procedure _proc,
                                 std::vector<node> _args)
      : base_type{std::move(_args)}, proc{_proc} {}
};

struct if_statement
    : base::impl<if_statement, subnode::concrete, subnode::vector>,
      statement {
  [[nodiscard]] decltype(auto) condition() { return get_subnode<0>(); }
  [[nodiscard]] decltype(auto) condition() const { return get_subnode<0>(); }

  [[nodiscard]] decltype(auto) statements() { return get_subnode<1>(); }
  [[nodiscard]] decltype(auto) statements() const { return get_subnode<1>(); }

  using base_type::impl;
};

struct if_else_statement : base::impl<if_else_statement, subnode::concrete,
                                      subnode::vector, subnode::vector>,
                           statement {
  source_loc else_loc;

  [[nodiscard]] decltype(auto) condition() { return get_subnode<0>(); }
  [[nodiscard]] decltype(auto) condition() const { return get_subnode<0>(); }

  [[nodiscard]] decltype(auto) consequence() { return get_subnode<1>(); }
  [[nodiscard]] decltype(auto) consequence() const { return get_subnode<1>(); }

  [[nodiscard]] decltype(auto) alternate() { return get_subnode<2>(); }
  [[nodiscard]] decltype(auto) alternate() const { return get_subnode<2>(); }

  using base_type::impl;
};

struct while_statement
    : base::impl<while_statement, subnode::concrete, subnode::vector>,
      statement {
  [[nodiscard]] decltype(auto) condition() { return get_subnode<0>(); }
  [[nodiscard]] decltype(auto) condition() const { return get_subnode<0>(); }

  [[nodiscard]] decltype(auto) statements() { return get_subnode<1>(); }
  [[nodiscard]] decltype(auto) statements() const { return get_subnode<1>(); }

  using base_type::impl;
};

struct for_statement : base::impl<for_statement, subnode::concrete,
                                  subnode::concrete, subnode::vector>,
                       statement {
  [[nodiscard]] decltype(auto) variable() { return get_subnode<0>(); }
  [[nodiscard]] decltype(auto) variable() const { return get_subnode<0>(); }

  [[nodiscard]] decltype(auto) list() { return get_subnode<1>(); }
  [[nodiscard]] decltype(auto) list() const { return get_subnode<1>(); }

  [[nodiscard]] decltype(auto) statements() { return get_subnode<2>(); }
  [[nodiscard]] decltype(auto) statements() const { return get_subnode<2>(); }

  using base_type::impl;
};

struct break_statement : base::impl<break_statement>, statement {
  using base_type::impl;
};

struct continue_statement : base::impl<continue_statement>, statement {
  using base_type::impl;
};

struct return_statement : base::impl<return_statement>, statement {
  using base_type::impl;
};

struct return_result_statement
    : base::impl<return_result_statement, subnode::concrete>,
      statement {
  [[nodiscard]] decltype(auto) result() { return get_subnode<0>(); }
  [[nodiscard]] decltype(auto) result() const { return get_subnode<0>(); }

  using base_type::impl;
};

struct variable_placeholder : base::impl<variable_placeholder> {
  std::string name;

  explicit variable_placeholder(std::string _name) : name{std::move(_name)} {}
};

struct variable_name : base::impl<variable_name>, lvalue {
  std::string name;

  explicit variable_name(std::string _name) : name{std::move(_name)} {}
};

struct expression_placeholder : base::impl<expression_placeholder> {
  std::string name;

  template <typename... arg_type>
  explicit expression_placeholder(std::string _name) : name{std::move(_name)} {}
};

struct unary_expression : base::impl<unary_expression, subnode::concrete>,
                          expression {
  unary_op op;

  [[nodiscard]] decltype(auto) argument() { return get_subnode<0>(); }
  [[nodiscard]] decltype(auto) argument() const { return get_subnode<0>(); }

  explicit unary_expression(unary_op _op, node _arg)
      : base_type{std::move(_arg)}, op{_op} {}
};

struct binary_expression
    : base::impl<binary_expression, subnode::concrete, subnode::concrete>,
      expression {
  binary_op op;

  [[nodiscard]] decltype(auto) left() { return get_subnode<0>(); }
  [[nodiscard]] decltype(auto) left() const { return get_subnode<0>(); }
  [[nodiscard]] decltype(auto) right() { return get_subnode<1>(); }
  [[nodiscard]] decltype(auto) right() const { return get_subnode<1>(); }

  explicit binary_expression(node _l, binary_op _op, node _r)
      : base_type{std::move(_l), std::move(_r)}, op{_op} {}
};

struct system_function_call : base::impl<system_function_call, subnode::vector>,
                              expression {
  system_function func;

  [[nodiscard]] decltype(auto) arguments() { return get_subnode<0>(); }
  [[nodiscard]] decltype(auto) arguments() const { return get_subnode<0>(); }

  explicit system_function_call(system_function _func, std::vector<node> _args)
      : base_type{std::move(_args)}, func{_func} {}
};

struct user_function_call : base::impl<user_function_call, subnode::vector>,
                            expression {
  std::string name;

  [[nodiscard]] decltype(auto) arguments() { return get_subnode<0>(); }
  [[nodiscard]] decltype(auto) arguments() const { return get_subnode<0>(); }

  explicit user_function_call(std::string name, std::vector<node> _args)
      : base_type{std::move(_args)}, name{std::move(name)} {}

  const function_definition* func() const { return _func; }

  // returns true if changed
  bool assign_definition(const function_definition* func);

 private:
  const function_definition* _func;
};

struct identifier : base::impl<identifier>, expression {
  std::string name;

  explicit identifier(std::string _name) : name{std::move(_name)} {}
};

struct number_literal : base::impl<number_literal>, expression {
  std::string value;

  explicit number_literal(std::string _value) : value{std::move(_value)} {}
};

struct string_literal : base::impl<string_literal>, expression {
  std::string value;

  explicit string_literal(std::string _value) : value{std::move(_value)} {}
};

}  // namespace marlin::ast

#endif  // marlin_ast_ast_impl
