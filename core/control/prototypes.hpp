#ifndef marlin_control_prototypes_hpp
#define marlin_control_prototypes_hpp

#include "placeholders.hpp"
#include "proto_gen.hpp"
#include "prototype_definition.hpp"

namespace marlin::control {

struct assignment_prototype : statement_prototype::impl<assignment_prototype> {
  [[nodiscard]] std::string name() const override { return "assign"; }

  inline static const proto_gen::statement_generator generator{proto_gen::node{
      [](auto left, auto right) {
        return ast::make<ast::assignment>(std::move(left), std::move(right));
      },
      variable_placeholder(placeholder::get<ast::assignment>(0)) + " = " +
          expression_placeholder(placeholder::get<ast::assignment>(1)) + ";"}};
};

struct print_prototype : statement_prototype::impl<print_prototype> {
  [[nodiscard]] std::string name() const override { return "print"; }

  inline static const proto_gen::statement_generator generator{proto_gen::node{
      [](auto value) {
        return ast::make<ast::print_statement>(std::move(value));
      },
      "print(" +
          expression_placeholder(placeholder::get<ast::print_statement>(0)) +
          ");"}};
};

struct if_prototype : statement_prototype::impl<if_prototype> {
  [[nodiscard]] std::string name() const override { return "if"; }

  inline static const proto_gen::statement_generator generator{proto_gen::node{
      [](auto condition) {
        return ast::make<ast::if_statement>(std::move(condition),
                                            std::vector<ast::node>{});
      },
      keyword("if") + " (" +
          expression_placeholder(placeholder::get<ast::if_statement>(0)) +
          ") {" + newline() + "}"}};
};

struct if_else_prototype : statement_prototype::impl<if_else_prototype> {
  [[nodiscard]] std::string name() const override { return "if-else"; }

  inline static source_loc else_loc;
  inline static const proto_gen::statement_generator generator{proto_gen::node{
      [](auto condition) {
        ast::node node{ast::make<ast::if_else_statement>(
            std::move(condition), std::vector<ast::node>{},
            std::vector<ast::node>{})};
        node->as<ast::if_else_statement>().else_loc = else_loc;
        return node;
      },
      keyword("if") + " (" +
          expression_placeholder(placeholder::get<ast::if_statement>(0)) +
          ") {" + newline() + "} " + store_loc(else_loc) + keyword("else") +
          " {" + newline() + "}"}};
};

template <ast::unary_op _op>
struct unary_prototype : expression_prototype::impl<unary_prototype<_op>> {
  [[nodiscard]] std::string name() const override { return symbol_for(_op); }

  inline static const auto content{
      op(symbol_for(_op)) +
      expression_placeholder(placeholder::get<ast::unary_expression>(0))};
  inline static const proto_gen::expression_generator generator_no_paren{
      proto_gen::node{[](auto argument) {
                        return ast::make<ast::unary_expression>(
                            _op, std::move(argument));
                      },
                      content}};
  inline static const proto_gen::expression_generator generator_with_paren{
      proto_gen::node{[](auto argument) {
                        return ast::make<ast::unary_expression>(
                            _op, std::move(argument));
                      },
                      "(" + content + ")"}};

  [[nodiscard]] std::pair<ast::node, source_replacement> construct(
      const ast::base& target) const override {
    assert(target.has_parent());

    if (target.parent().is<marlin::ast::unary_expression>()) {
      return generator_with_paren.construct(target.source_code_range);
    } else {
      return generator_no_paren.construct(target.source_code_range);
    }
  }
};

template struct unary_prototype<ast::unary_op::negative>;

template <ast::binary_op _op>
struct binary_prototype : expression_prototype::impl<binary_prototype<_op>> {
  [[nodiscard]] std::string name() const override { return symbol_for(_op); }

  inline static const auto content{
      expression_placeholder(placeholder::get<ast::binary_expression>(0)) +
      (std::string{" "} + op(symbol_for(_op)) + " ") +
      expression_placeholder(placeholder::get<ast::binary_expression>(1))};
  inline static const proto_gen::expression_generator generator_no_paren{
      proto_gen::node{[](auto left, auto right) {
                        return ast::make<ast::binary_expression>(
                            std::move(left), _op, std::move(right));
                      },
                      content}};
  inline static const proto_gen::expression_generator generator_with_paren{
      proto_gen::node{[](auto left, auto right) {
                        return ast::make<ast::binary_expression>(
                            std::move(left), _op, std::move(right));
                      },
                      "(" + content + ")"}};

  [[nodiscard]] std::pair<ast::node, source_replacement> construct(
      const ast::base& target) const override {
    assert(target.has_parent());

    bool needs_paren{false};
    if (target.parent().is<marlin::ast::unary_expression>()) {
      needs_paren = true;
    } else if (target.parent().is<marlin::ast::binary_expression>()) {
      const auto& binary{target.parent().as<marlin::ast::binary_expression>()};
      const auto parent_precedence = precedence_for(binary.op);
      const auto this_precedence = precedence_for(_op);
      if (this_precedence < parent_precedence ||
          (this_precedence == parent_precedence &&
           binary.right().get() == &target)) {
        needs_paren = true;
      }
    }

    if (needs_paren) {
      return generator_with_paren.construct(target.source_code_range);
    } else {
      return generator_no_paren.construct(target.source_code_range);
    }
  }
};

template struct binary_prototype<ast::binary_op::add>;
template struct binary_prototype<ast::binary_op::subtract>;
template struct binary_prototype<ast::binary_op::multiply>;
template struct binary_prototype<ast::binary_op::divide>;

struct number_prototype {
  static auto construct(source_range original, std::string value) {
    auto value_copy{value};
    const proto_gen::expression_generator gen{proto_gen::node{
        [value{std::move(value_copy)}]() {
          return ast::make<ast::number_literal>(std::move(value));
        },
        number(std::move(value))}};
    return gen.construct(original);
  }
};

struct string_prototype {
  static auto construct(source_range original, std::string str) {
    auto quoted_string{quoted(str)};
    const proto_gen::expression_generator gen{proto_gen::node{
        [str{std::move(str)}]() { return ast::make<ast::string_literal>(str); },
        string(std::move(quoted_string))}};
    return gen.construct(original);
  }
};

struct variable_name_prototype {
  static auto construct(source_range original, std::string name) {
    auto name_copy{name};
    const proto_gen::expression_generator gen{
        proto_gen::node{[name{std::move(name_copy)}]() {
                          return ast::make<ast::variable_name>(std::move(name));
                        },
                        std::move(name)}};
    return gen.construct(original);
  }
};

struct identifier_prototype {
  static auto construct(source_range original, std::string name) {
    auto name_copy{name};
    const proto_gen::expression_generator gen{
        proto_gen::node{[name{std::move(name_copy)}]() {
                          return ast::make<ast::identifier>(std::move(name));
                        },
                        std::move(name)}};
    return gen.construct(original);
  }
};

}  // namespace marlin::control

#endif  // marlin_control_prototypes_hpp
