#ifndef marlin_control_prototypes_hpp
#define marlin_control_prototypes_hpp

#include "placeholders.hpp"
#include "prototype_definition.hpp"
#include "store.hpp"

namespace marlin::control {

struct assignment_prototype : statement_prototype::impl<assignment_prototype> {
  [[nodiscard]] std::string_view name() const override { return "assign"; }

  inline static const store::data_vector _data{[]() {
    const auto node{ast::make<ast::assignment>(
        ast::make<ast::variable_placeholder>(
            std::string{placeholder::get<ast::assignment>(0)}),
        ast::make<ast::expression_placeholder>(
            std::string{placeholder::get<ast::assignment>(1)}))};
    return store::write({node.get()});
  }()};
};

template <ast::system_procedure _proc>
struct system_procedure_prototype
    : statement_prototype::impl<system_procedure_prototype<_proc>> {
  [[nodiscard]] std::string_view name() const override {
    return name_for(_proc);
  }

  inline static const store::data_vector _data{[]() {
    const auto& placeholders{placeholder_system_procedure_args::args(_proc)};
    std::vector<ast::node> args;
    args.reserve(placeholders.size());
    for (const auto& arg : placeholders) {
      args.emplace_back(
          ast::make<ast::expression_placeholder>(std::string{arg}));
    }
    const auto node{
        ast::make<ast::system_procedure_call>(_proc, std::move(args))};
    return store::write({node.get()});
  }()};
};

struct if_prototype : statement_prototype::impl<if_prototype> {
  [[nodiscard]] std::string_view name() const override { return "if"; }

  inline static const store::data_vector _data{[]() {
    const auto node{ast::make<ast::if_statement>(
        ast::make<ast::expression_placeholder>(
            std::string{placeholder::get<ast::if_statement>(0)}),
        std::vector<ast::node>{})};
    return store::write({node.get()});
  }()};
};

struct if_else_prototype : statement_prototype::impl<if_else_prototype> {
  [[nodiscard]] std::string_view name() const override { return "if-else"; }

  inline static const store::data_vector _data{[]() {
    const auto node{ast::make<ast::if_else_statement>(
        ast::make<ast::expression_placeholder>(
            std::string{placeholder::get<ast::if_else_statement>(0)}),
        std::vector<ast::node>{}, std::vector<ast::node>{})};
    return store::write({node.get()});
  }()};
};

struct while_prototype : statement_prototype::impl<while_prototype> {
  [[nodiscard]] std::string_view name() const override { return "while"; }

  inline static const store::data_vector _data{[]() {
    const auto node{ast::make<ast::while_statement>(
        ast::make<ast::expression_placeholder>(
            std::string{placeholder::get<ast::while_statement>(0)}),
        std::vector<ast::node>{})};
    return store::write({node.get()});
  }()};
};

struct for_prototype : statement_prototype::impl<for_prototype> {
  [[nodiscard]] std::string_view name() const override { return "for"; }

  inline static const store::data_vector _data{[]() {
    const auto node{ast::make<ast::for_statement>(
        ast::make<ast::variable_placeholder>(
            std::string{placeholder::get<ast::for_statement>(0)}),
        ast::make<ast::expression_placeholder>(
            std::string{placeholder::get<ast::for_statement>(1)}),
        std::vector<ast::node>{})};
    return store::write({node.get()});
  }()};
};

struct break_prototype : statement_prototype::impl<break_prototype> {
  [[nodiscard]] std::string_view name() const override { return "break"; }

  inline static const store::data_vector _data{[]() {
    const auto node{ast::make<ast::break_statement>()};
    return store::write({node.get()});
  }()};
};

struct continue_prototype : statement_prototype::impl<continue_prototype> {
  [[nodiscard]] std::string_view name() const override { return "continue"; }

  inline static const store::data_vector _data{[]() {
    const auto node{ast::make<ast::continue_statement>()};
    return store::write({node.get()});
  }()};
};

template <ast::unary_op _op>
struct unary_prototype : expression_prototype::impl<unary_prototype<_op>> {
  [[nodiscard]] std::string_view name() const override {
    return symbol_for(_op);
  }

  inline static const store::data_vector _data{[]() {
    const auto node{ast::make<ast::unary_expression>(
        _op, ast::make<ast::expression_placeholder>(
                 std::string{placeholder::get<ast::unary_expression>(0)}))};
    return store::write({node.get()});
  }()};
};

template <ast::binary_op _op>
struct binary_prototype : expression_prototype::impl<binary_prototype<_op>> {
  [[nodiscard]] std::string_view name() const override {
    return symbol_for(_op);
  }

  inline static const store::data_vector _data{[]() {
    const auto node{ast::make<ast::binary_expression>(
        ast::make<ast::expression_placeholder>(
            std::string{placeholder::get<ast::binary_expression>(0)}),
        _op,
        ast::make<ast::expression_placeholder>(
            std::string{placeholder::get<ast::binary_expression>(1)}))};
    return store::write({node.get()});
  }()};
};

template <ast::system_function _func>
struct system_function_prototype
    : expression_prototype::impl<system_function_prototype<_func>> {
  [[nodiscard]] std::string_view name() const override {
    return name_for(_func);
  }

  inline static const store::data_vector _data{[]() {
    const auto& placeholders{placeholder_system_function_args::args(_func)};
    std::vector<ast::node> args;
    args.reserve(placeholders.size());
    for (const auto& arg : placeholders) {
      args.emplace_back(
          ast::make<ast::expression_placeholder>(std::string{arg}));
    }
    const auto node{
        ast::make<ast::system_function_call>(_func, std::move(args))};
    return store::write({node.get()});
  }()};
};

struct number_prototype {
  static store::data_vector data(std::string_view value) {
    auto node = ast::make<ast::number_literal>(std::string{std::move(value)});
    return store::write({node.get()});
  }
};

struct string_prototype {
  static store::data_vector data(std::string_view value) {
    auto node = ast::make<ast::string_literal>(std::string{std::move(value)});
    return store::write({node.get()});
  }
};

struct identifier_prototype {
  static store::data_vector data(std::string_view value) {
    auto node = ast::make<ast::identifier>(std::string{std::move(value)});
    return store::write({node.get()});
  }
};

}  // namespace marlin::control

#endif  // marlin_control_prototypes_hpp
