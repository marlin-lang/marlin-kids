#ifndef marlin_ast_ast_impl
#define marlin_ast_ast_impl

#include <string>

#include "base.impl.hpp"
#include "specs.hpp"

namespace marlin::ast {

struct block {};
struct statement {};
struct expression {};

struct program : base::impl<program, subnode::vector> {
  [[nodiscard]] decltype(auto) blocks() { return get_subnode<0>(); }
  [[nodiscard]] decltype(auto) blocks() const { return get_subnode<0>(); }

  using base_type::impl;
};

struct on_start : base::impl<on_start, subnode::vector> {
  [[nodiscard]] decltype(auto) statements() { return get_subnode<0>(); }
  [[nodiscard]] decltype(auto) statements() const { return get_subnode<0>(); }

  using base_type::impl;
};

struct variable_declaration
    : base::impl<variable_declaration, subnode::concrete, subnode::concrete>,
      statement {
  [[nodiscard]] decltype(auto) variable() { return get_subnode<0>(); }
  [[nodiscard]] decltype(auto) variable() const { return get_subnode<0>(); }

  [[nodiscard]] decltype(auto) value() { return get_subnode<1>(); }
  [[nodiscard]] decltype(auto) value() const { return get_subnode<1>(); }

  using base_type::impl;
};

struct print_statement : base::impl<print_statement, subnode::concrete>,
                         statement {
  [[nodiscard]] decltype(auto) value() { return get_subnode<0>(); }
  [[nodiscard]] decltype(auto) value() const { return get_subnode<0>(); }

  using base_type::impl;
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

struct variable_placeholder : base::impl<variable_placeholder> {
  std::string name;

  explicit variable_placeholder(std::string _name) : name{std::move(_name)} {}
};

struct variable_name : base::impl<variable_name> {
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

struct identifier : base::impl<identifier>, expression {
  std::string name;

  explicit identifier(std::string _name) : name{std::move(_name)} {}
};

struct number_literal : base::impl<number_literal>, expression {
  std::string value;

  explicit number_literal(std::string _value) : value{std::move(_value)} {}
};

}  // namespace marlin::ast

#endif  // marlin_ast_ast_impl