#ifndef marlin_control_prototypes_hpp
#define marlin_control_prototypes_hpp

#include "placeholders.hpp"
#include "prototype_definition.hpp"
#include "store.hpp"

namespace marlin::control {

struct assignment_prototype : statement_prototype::impl<assignment_prototype> {
  [[nodiscard]] std::string_view name() const override { return "assign"; }

  inline static const store::data_vector data{[]() {
    const auto node{ast::make<ast::assignment>(
        ast::make<ast::variable_placeholder>(
            std::string{placeholder::get<ast::assignment>(0)}),
        ast::make<ast::expression_placeholder>(
            std::string{placeholder::get<ast::assignment>(1)}))};
    return store::write({node.get()});
  }()};
};

struct print_prototype : statement_prototype::impl<print_prototype> {
  [[nodiscard]] std::string_view name() const override { return "print"; }

  inline static const store::data_vector data{[]() {
    const auto node{
        ast::make<ast::print_statement>(ast::make<ast::expression_placeholder>(
            std::string{placeholder::get<ast::print_statement>(0)}))};
    return store::write({node.get()});
  }()};
};

struct if_prototype : statement_prototype::impl<if_prototype> {
  [[nodiscard]] std::string_view name() const override { return "if"; }

  inline static const store::data_vector data{[]() {
    const auto node{ast::make<ast::if_statement>(
        ast::make<ast::expression_placeholder>(
            std::string{placeholder::get<ast::if_statement>(0)}),
        std::vector<ast::node>{})};
    return store::write({node.get()});
  }()};
};

struct if_else_prototype : statement_prototype::impl<if_else_prototype> {
  [[nodiscard]] std::string_view name() const override { return "if-else"; }

  inline static const store::data_vector data{[]() {
    const auto node{ast::make<ast::if_else_statement>(
        ast::make<ast::expression_placeholder>(
            std::string{placeholder::get<ast::if_else_statement>(0)}),
        std::vector<ast::node>{}, std::vector<ast::node>{})};
    return store::write({node.get()});
  }()};
};

template <ast::unary_op _op>
struct unary_prototype : expression_prototype::impl<unary_prototype<_op>> {
  [[nodiscard]] std::string_view name() const override {
    return symbol_for(_op);
  }

  inline static const store::data_vector data{[]() {
    const auto node{ast::make<ast::unary_expression>(
        _op, ast::make<ast::expression_placeholder>(
                 std::string{placeholder::get<ast::unary_expression>(0)}))};
    return store::write({node.get()});
  }()};
};

template struct unary_prototype<ast::unary_op::negative>;

template <ast::binary_op _op>
struct binary_prototype : expression_prototype::impl<binary_prototype<_op>> {
  [[nodiscard]] std::string_view name() const override {
    return symbol_for(_op);
  }

  inline static const store::data_vector data{[]() {
    const auto node{ast::make<ast::binary_expression>(
        ast::make<ast::expression_placeholder>(
            std::string{placeholder::get<ast::binary_expression>(0)}),
        _op,
        ast::make<ast::expression_placeholder>(
            std::string{placeholder::get<ast::binary_expression>(1)}))};
    return store::write({node.get()});
  }()};
};

template struct binary_prototype<ast::binary_op::add>;
template struct binary_prototype<ast::binary_op::subtract>;
template struct binary_prototype<ast::binary_op::multiply>;
template struct binary_prototype<ast::binary_op::divide>;

// Construct literal prototypes manually for now

struct number_prototype {
  static store::data_vector data(std::string_view value) {
    auto node = ast::make<ast::number_literal>(std::string{std::move(value)});
    return store::write({node.get()});
  }

  static auto construct(source_range original, std::string value) {
    auto node = ast::make<ast::number_literal>(value);
    node->source_code_range = {
        original.begin,
        {original.begin.line, original.begin.column + value.size()}};
    std::vector<highlight_token> highlights{
        highlight_token{highlight_token_type::number, 0, value.size()}};
    return std::make_pair(
        std::move(node),
        source_replacement{original, std::move(value), std::move(highlights)});
  }
};

struct string_prototype {
  static store::data_vector data(std::string_view value) {
    auto node = ast::make<ast::string_literal>(std::string{std::move(value)});
    return store::write({node.get()});
  }

  static auto construct(source_range original, std::string str) {
    auto quoted_string{quoted(str)};
    auto node = ast::make<ast::string_literal>(std::move(str));
    node->source_code_range = {
        original.begin,
        {original.begin.line, original.begin.column + quoted_string.size()}};
    std::vector<highlight_token> highlights{
        highlight_token{highlight_token_type::string, 0, quoted_string.size()}};
    return std::make_pair(std::move(node),
                          source_replacement{original, std::move(quoted_string),
                                             std::move(highlights)});
  }
};

struct variable_name_prototype {
  static store::data_vector data(std::string_view value) {
    auto node = ast::make<ast::variable_name>(std::string{std::move(value)});
    return store::write({node.get()});
  }

  static auto construct(source_range original, std::string name) {
    auto node = ast::make<ast::variable_name>(name);
    node->source_code_range = {
        original.begin,
        {original.begin.line, original.begin.column + name.size()}};
    return std::make_pair(std::move(node),
                          source_replacement{original, std::move(name), {}});
  }
};

struct identifier_prototype {
  static store::data_vector data(std::string_view value) {
    auto node = ast::make<ast::identifier>(std::string{std::move(value)});
    return store::write({node.get()});
  }

  static auto construct(source_range original, std::string name) {
    auto node = ast::make<ast::identifier>(name);
    node->source_code_range = {
        original.begin,
        {original.begin.line, original.begin.column + name.size()}};
    return std::make_pair(std::move(node),
                          source_replacement{original, std::move(name), {}});
  }
};

}  // namespace marlin::control

#endif  // marlin_control_prototypes_hpp
