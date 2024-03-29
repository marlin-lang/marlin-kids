#ifndef marlin_control_prototypes_hpp
#define marlin_control_prototypes_hpp

#include "formatter.hpp"
#include "placeholders.hpp"
#include "store.hpp"

namespace marlin::control {

enum struct pasteboard_t : uint8_t { block, statement, expression, reference };

struct prototype {
  std::string_view name;
  pasteboard_t type;
  store::data_vector data;
  format::display display;

  prototype(std::string_view _name, pasteboard_t _type, const ast::node& _node)
      : name{std::move(_name)},
        type{_type},
        data{store::write({_node.get()})},
        display{formatter.format(*_node)} {}

 private:
  inline static format::const_formatter formatter;
};

inline prototype function_prototype() {
  return {"function", pasteboard_t::block,
          ast::make<ast::function>(ast::make<ast::function_placeholder>(
                                       placeholder::get<ast::function>({0}),
                                       std::vector<ast::node>{}),
                                   std::vector<ast::node>{})};
}

inline prototype eval_prototype() {
  return {"eval", pasteboard_t::statement,
          ast::make<ast::eval_statement>(ast::make<ast::expression_placeholder>(
              placeholder::get<ast::eval_statement>({0})))};
}

inline prototype assignment_prototype() {
  return {
      "assign", pasteboard_t::statement,
      ast::make<ast::assignment>(ast::make<ast::variable_placeholder>(
                                     placeholder::get<ast::assignment>({0})),
                                 ast::make<ast::expression_placeholder>(
                                     placeholder::get<ast::assignment>({1})))};
}

inline prototype use_global_prototype() {
  return {"global", pasteboard_t::statement,
          ast::make<ast::use_global>(ast::make<ast::variable_placeholder>(
              placeholder::get<ast::use_global>({0})))};
}

inline prototype array_modification_prototype(ast::array_modification mod) {
  const auto& placeholders{placeholder_array_modification_args::args(mod)};
  std::vector<ast::node> args;
  args.reserve(placeholders.size());
  for (const auto& arg : placeholders) {
    args.emplace_back(ast::make<ast::expression_placeholder>(std::string{arg}));
  }
  return {name_for(mod), pasteboard_t::statement,
          ast::make<ast::modify_array>(
              mod,
              ast::make<ast::variable_placeholder>(
                  placeholder::get<ast::modify_array>({0})),
              std::move(args))};
}

inline prototype system_procedure_prototype(ast::system_procedure proc) {
  const auto& placeholders{placeholder_system_procedure_args::args(proc)};
  std::vector<ast::node> args;
  args.reserve(placeholders.size());
  for (const auto& arg : placeholders) {
    args.emplace_back(ast::make<ast::expression_placeholder>(std::string{arg}));
  }
  return {name_for(proc), pasteboard_t::statement,
          ast::make<ast::system_procedure_call>(proc, std::move(args))};
}

inline prototype if_prototype() {
  return {"if", pasteboard_t::statement,
          ast::make<ast::if_statement>(
              ast::make<ast::expression_placeholder>(
                  placeholder::get<ast::if_statement>({0})),
              std::vector<ast::node>{})};
}

inline prototype if_else_prototype() {
  return {"if-else", pasteboard_t::statement,
          ast::make<ast::if_else_statement>(
              ast::make<ast::expression_placeholder>(
                  placeholder::get<ast::if_else_statement>(0)),
              std::vector<ast::node>{}, std::vector<ast::node>{})};
}

inline prototype while_prototype() {
  return {"while", pasteboard_t::statement,
          ast::make<ast::while_statement>(
              ast::make<ast::expression_placeholder>(
                  placeholder::get<ast::while_statement>({0})),
              std::vector<ast::node>{})};
}

inline prototype for_prototype() {
  return {"for", pasteboard_t::statement,
          ast::make<ast::for_statement>(
              ast::make<ast::variable_placeholder>(
                  placeholder::get<ast::for_statement>({0})),
              ast::make<ast::expression_placeholder>(
                  placeholder::get<ast::for_statement>({1})),
              std::vector<ast::node>{})};
}

inline prototype break_prototype() {
  return {"break", pasteboard_t::statement, ast::make<ast::break_statement>()};
}

inline prototype continue_prototype() {
  return {"continue", pasteboard_t::statement,
          ast::make<ast::continue_statement>()};
}

inline prototype return_prototype() {
  return {"return", pasteboard_t::statement,
          ast::make<ast::return_statement>()};
}

inline prototype return_result_prototype() {
  return {"return1", pasteboard_t::statement,
          ast::make<ast::return_result_statement>(
              ast::make<ast::expression_placeholder>(
                  placeholder::get<ast::return_result_statement>({0})))};
}

inline prototype unary_prototype(ast::unary_op op) {
  return {symbol_for(op), pasteboard_t::expression,
          ast::make<ast::unary_expression>(
              op, ast::make<ast::expression_placeholder>(
                      placeholder::get<ast::unary_expression>({0})))};
}

inline prototype binary_prototype(ast::binary_op op) {
  return {symbol_for(op), pasteboard_t::expression,
          ast::make<ast::binary_expression>(
              ast::make<ast::expression_placeholder>(
                  placeholder::get<ast::binary_expression>({0})),
              op,
              ast::make<ast::expression_placeholder>(
                  placeholder::get<ast::binary_expression>({1})))};
}

inline prototype subscript_prototype() {
  return {"subscript", pasteboard_t::reference,
          ast::make<ast::subscript_get>(
              ast::make<ast::expression_placeholder>(
                  placeholder::get<ast::subscript_get>({0})),
              ast::make<ast::expression_placeholder>(
                  placeholder::get<ast::subscript_get>({1})))};
}

inline prototype new_array_prototype() {
  return {"new_array", pasteboard_t::expression,
          ast::make<ast::new_array>(std::vector<ast::node>{})};
}

inline prototype new_color_prototype(ast::color_mode mode) {
  const auto& placeholders{placeholder_new_color_args::args(mode)};
  std::vector<ast::node> args;
  args.reserve(placeholders.size());
  for (const auto& arg : placeholders) {
    args.emplace_back(ast::make<ast::expression_placeholder>(std::string{arg}));
  }
  return {name_for(mode), pasteboard_t::expression,
          ast::make<ast::new_color>(mode, std::move(args))};
}

inline prototype system_function_prototype(ast::system_function func) {
  const auto& placeholders{placeholder_system_function_args::args(func)};
  std::vector<ast::node> args;
  args.reserve(placeholders.size());
  for (const auto& arg : placeholders) {
    args.emplace_back(ast::make<ast::expression_placeholder>(std::string{arg}));
  }
  return {name_for(func), pasteboard_t::expression,
          ast::make<ast::system_function_call>(func, std::move(args))};
}

inline prototype user_function_prototype(const function_definition& func) {
  std::vector<ast::node> args;
  args.reserve(func.parameters.size());
  for (const auto& param : func.parameters) {
    args.emplace_back(ast::make<ast::expression_placeholder>(param));
  }
  return {func.name, pasteboard_t::expression,
          ast::make<ast::user_function_call>(func.name, std::move(args))};
}

inline prototype bool_literal_prototype(bool value) {
  return {value ? "true" : "false", pasteboard_t::expression,
          ast::make<ast::bool_literal>(value)};
}

}  // namespace marlin::control

#endif  // marlin_control_prototypes_hpp
