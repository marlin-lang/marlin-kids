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

  [[nodiscard]] inline node &argument() { return get_subnode<0>(); }
  [[nodiscard]] inline const node &argument() const { return get_subnode<0>(); }

  [[nodiscard]] inline source_range op_range() const noexcept {
    const auto start{source_code_range.begin};
    const auto length{length_of(op)};
    return {start, {start.line, start.column + length}};
  }

  explicit inline unary_expression(unary_op _op, node _arg)
      : base_type{std::move(_arg)}, op{_op} {}
};

struct binary_expression
    : base::impl<binary_expression, subnode::concrete, subnode::concrete>,
      expression {
  binary_op op;

  source_loc op_loc;

  [[nodiscard]] inline node &left() { return get_subnode<0>(); }
  [[nodiscard]] inline const node &left() const { return get_subnode<0>(); }
  [[nodiscard]] inline node &right() { return get_subnode<1>(); }
  [[nodiscard]] inline const node &right() const { return get_subnode<1>(); }

  [[nodiscard]] inline source_range op_range() const noexcept {
    const auto length{length_of(op)};
    return {op_loc, {op_loc.line, op_loc.column + length}};
  }

  explicit inline binary_expression(node _l, binary_op _op, node _r)
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