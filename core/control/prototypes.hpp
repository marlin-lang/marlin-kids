#ifndef marlin_control_prototypes_hpp
#define marlin_control_prototypes_hpp

#include "placeholders.hpp"
#include "store.hpp"

namespace marlin::control {

enum struct pasteboard_t : uint8_t { block, statement, expression, reference };

struct prototype {
  std::string_view name;
  pasteboard_t type;
  store::data_vector data;

  prototype(std::string_view _name, pasteboard_t _type,
            store::data_vector _data)
      : name{std::move(_name)}, type{_type}, data{std::move(_data)} {}
};

inline prototype function_prototype() {
  return {"function", pasteboard_t::block, []() {
            const auto node{ast::make<ast::function>(
                ast::make<ast::function_placeholder>(
                    std::string{placeholder::get<ast::function>(0)}),
                std::vector<ast::node>{})};
            return store::write({node.get()});
          }()};
}

inline prototype eval_prototype() {
  return {"eval", pasteboard_t::statement, []() {
            const auto node{ast::make<ast::eval_statement>(
                ast::make<ast::expression_placeholder>(
                    std::string{placeholder::get<ast::eval_statement>(0)}))};
            return store::write({node.get()});
          }()};
}

inline prototype assignment_prototype() {
  return {"assign", pasteboard_t::statement, []() {
            const auto node{ast::make<ast::assignment>(
                ast::make<ast::variable_placeholder>(
                    std::string{placeholder::get<ast::assignment>(0)}),
                ast::make<ast::expression_placeholder>(
                    std::string{placeholder::get<ast::assignment>(1)}))};
            return store::write({node.get()});
          }()};
}

inline prototype use_global_prototype() {
  return {"global", pasteboard_t::statement, []() {
            const auto node{
                ast::make<ast::use_global>(ast::make<ast::variable_placeholder>(
                    std::string{placeholder::get<ast::use_global>(0)}))};
            return store::write({node.get()});
          }()};
}

inline prototype system_procedure_prototype(ast::system_procedure proc) {
  return {
      name_for(proc), pasteboard_t::statement, [&proc]() {
        const auto& placeholders{placeholder_system_procedure_args::args(proc)};
        std::vector<ast::node> args;
        args.reserve(placeholders.size());
        for (const auto& arg : placeholders) {
          args.emplace_back(
              ast::make<ast::expression_placeholder>(std::string{arg}));
        }
        const auto node{
            ast::make<ast::system_procedure_call>(proc, std::move(args))};
        return store::write({node.get()});
      }()};
}

inline prototype if_prototype() {
  return {"if", pasteboard_t::statement, []() {
            const auto node{ast::make<ast::if_statement>(
                ast::make<ast::expression_placeholder>(
                    std::string{placeholder::get<ast::if_statement>(0)}),
                std::vector<ast::node>{})};
            return store::write({node.get()});
          }()};
}

inline prototype if_else_prototype() {
  return {"if-else", pasteboard_t::statement, []() {
            const auto node{ast::make<ast::if_else_statement>(
                ast::make<ast::expression_placeholder>(
                    std::string{placeholder::get<ast::if_else_statement>(0)}),
                std::vector<ast::node>{}, std::vector<ast::node>{})};
            return store::write({node.get()});
          }()};
}

inline prototype while_prototype() {
  return {"while", pasteboard_t::statement, []() {
            const auto node{ast::make<ast::while_statement>(
                ast::make<ast::expression_placeholder>(
                    std::string{placeholder::get<ast::while_statement>(0)}),
                std::vector<ast::node>{})};
            return store::write({node.get()});
          }()};
}

inline prototype for_prototype() {
  return {"for", pasteboard_t::statement, []() {
            const auto node{ast::make<ast::for_statement>(
                ast::make<ast::variable_placeholder>(
                    std::string{placeholder::get<ast::for_statement>(0)}),
                ast::make<ast::expression_placeholder>(
                    std::string{placeholder::get<ast::for_statement>(1)}),
                std::vector<ast::node>{})};
            return store::write({node.get()});
          }()};
}

inline prototype break_prototype() {
  return {"break", pasteboard_t::statement, []() {
            const auto node{ast::make<ast::break_statement>()};
            return store::write({node.get()});
          }()};
}

inline prototype continue_prototype() {
  return {"continue", pasteboard_t::statement, []() {
            const auto node{ast::make<ast::continue_statement>()};
            return store::write({node.get()});
          }()};
}

inline prototype return_prototype() {
  return {"return", pasteboard_t::statement, []() {
            const auto node{ast::make<ast::return_statement>()};
            return store::write({node.get()});
          }()};
}

inline prototype return_result_prototype() {
  return {"return1", pasteboard_t::statement, []() {
            const auto node{ast::make<ast::return_result_statement>(
                ast::make<ast::expression_placeholder>(std::string{
                    placeholder::get<ast::return_result_statement>(0)}))};
            return store::write({node.get()});
          }()};
}

inline prototype unary_prototype(ast::unary_op op) {
  return {symbol_for(op), pasteboard_t::expression, [&op]() {
            const auto node{ast::make<ast::unary_expression>(
                op, ast::make<ast::expression_placeholder>(std::string{
                        placeholder::get<ast::unary_expression>(0)}))};
            return store::write({node.get()});
          }()};
}

inline prototype binary_prototype(ast::binary_op op) {
  return {symbol_for(op), pasteboard_t::expression, [&op]() {
            const auto node{ast::make<ast::binary_expression>(
                ast::make<ast::expression_placeholder>(
                    std::string{placeholder::get<ast::binary_expression>(0)}),
                op,
                ast::make<ast::expression_placeholder>(
                    std::string{placeholder::get<ast::binary_expression>(1)}))};
            return store::write({node.get()});
          }()};
}

inline prototype system_function_prototype(ast::system_function func) {
  return {
      name_for(func), pasteboard_t::expression, [&func]() {
        const auto& placeholders{placeholder_system_function_args::args(func)};
        std::vector<ast::node> args;
        args.reserve(placeholders.size());
        for (const auto& arg : placeholders) {
          args.emplace_back(
              ast::make<ast::expression_placeholder>(std::string{arg}));
        }
        const auto node{
            ast::make<ast::system_function_call>(func, std::move(args))};
        return store::write({node.get()});
      }()};
}

inline prototype user_function_prototype(const function_definition& func) {
  return {func.name, pasteboard_t::expression, [&func]() {
            std::vector<ast::node> args;
            args.reserve(func.parameters.size());
            for (const auto& param : func.parameters) {
              args.emplace_back(ast::make<ast::expression_placeholder>(param));
            }
            const auto node{
                ast::make<ast::user_function_call>(func.name, std::move(args))};
            // No need to assign a function definition for node here
            return store::write({node.get()});
          }()};
}

}  // namespace marlin::control

#endif  // marlin_control_prototypes_hpp
