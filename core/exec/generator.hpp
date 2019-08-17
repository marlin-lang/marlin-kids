#ifndef marlin_exec_generator_hpp
#define marlin_exec_generator_hpp

#include <array>
#include <type_traits>

#include <jsast/jsast.hpp>

#include "ast.hpp"
#include "exec_errors.hpp"

namespace marlin::exec {

struct generator {
  generator() noexcept = default;

  std::string generate(ast::base& c) {
    jsast::generator gen;

    _errors.clear();
    gen.write(get_node(c));
    if (_errors.size()) {
      auto errors{std::move(_errors)};
      _errors = {};
      throw collected_generation_error{std::move(errors)};
    } else {
      return std::move(gen).str();
    }
  }

 private:
  static constexpr const char* main_name{"__main__"};

  std::vector<generation_error> _errors;

  jsast::ast::node scoped_callee_with_name(std::string name) {
    return jsast::ast::member_expression{
        jsast::ast::object_expression{
            {jsast::ast::property{jsast::ast::member_identifier{"__thefunc__"},
                                  jsast::ast::identifier{std::move(name)}}}},
        jsast::ast::member_identifier{"__thefunc__"}};
  }

  template <typename vector_type>
  auto get_block(vector_type vector) {
    jsast::utils::move_vector<jsast::ast::node> statements;
    for (auto& statement : vector) {
      statements.emplace_back(get_node(*statement));
    }
    return jsast::ast::block_statement{std::move(statements)};
  }

  jsast::ast::node get_node(ast::base& c) {
    return c.apply<jsast::ast::node>([this](auto& node) {
      return jsast::ast::node{get_jsast(node), [&node](source_range range) {
                                node._js_range = range;
                              }};
    });
  }

  auto get_jsast(ast::variable_placeholder& node) {
    _errors.emplace_back("Unexpected placeholder!", node);
    return jsast::ast::identifier{"__error__"};
  }

  auto get_jsast(ast::expression_placeholder& node) {
    _errors.emplace_back("Unexpected placeholder!", node);
    return jsast::ast::identifier{"__error__"};
  }

  auto get_jsast(ast::program& program) {
    jsast::utils::move_vector<jsast::ast::node> blocks;
    for (auto& block : program.blocks()) {
      blocks.emplace_back(get_node(*block));
    }
    blocks.emplace_back(
        jsast::ast::call_expression{scoped_callee_with_name(main_name), {}});
    return jsast::ast::program{std::move(blocks)};
  }

  auto get_jsast(ast::on_start& start) {
    return jsast::ast::function_declaration{
        main_name,
        {},
        jsast::ast::block_statement{get_block(start.statements())}};
  }

  auto get_jsast(ast::assignment& assignment) {
    return jsast::ast::expression_statement{jsast::ast::assignment_expression{
        get_node(*assignment.variable()), jsast::assignment_op::standard,
        get_node(*assignment.value())}};
  }

  auto get_jsast(ast::print_statement& statement) {
    return jsast::ast::expression_statement{jsast::ast::call_expression{
        jsast::ast::identifier{"print"}, {get_node(*statement.value())}}};
  }

  auto get_jsast(ast::if_statement& statement) {
    return jsast::ast::if_statement{get_node(*statement.condition()),
                                    get_block(statement.statements())};
  }

  auto get_jsast(ast::if_else_statement& statement) {
    return jsast::ast::if_statement{get_node(*statement.condition()),
                                    get_block(statement.consequence()),
                                    get_block(statement.alternate())};
  }

  auto get_jsast(ast::while_statement& statement) {
    return jsast::ast::while_statement{get_node(*statement.condition()),
                                       get_block(statement.statements())};
  }

  auto get_jsast(ast::for_statement& statement) {
    return jsast::ast::for_of_statement{get_node(*statement.variable()),
                                        get_node(*statement.list()),
                                        get_block(statement.statements())};
  }

  auto get_jsast(ast::unary_expression& unary) {
    static constexpr std::array symbol_map{
        jsast::unary_op::negative /* negative */
    };
    return jsast::ast::unary_expression{
        symbol_map[static_cast<uint8_t>(unary.op)],
        get_node(*unary.argument())};
  }

  auto get_jsast(ast::binary_expression& binary) {
    static constexpr std::array symbol_map{
        jsast::binary_op::add /* add */,
        jsast::binary_op::subtract /* subtract */,
        jsast::binary_op::multiply /* multiply */,
        jsast::binary_op::divide /* divide */
    };
    return jsast::ast::binary_expression{
        get_node(*binary.left()), symbol_map[static_cast<uint8_t>(binary.op)],
        get_node(*binary.right())};
  }

  auto get_jsast(ast::system_function_call& call) {
    static constexpr std::array<jsast::ast::node (*)(), 3> callee_map{
        []() {
          return jsast::ast::node{jsast::ast::identifier{"range"}};
        } /* range1 */,
        []() {
          return jsast::ast::node{jsast::ast::identifier{"range"}};
        } /* range2 */,
        []() {
          return jsast::ast::node{jsast::ast::identifier{"range"}};
        } /* range3 */
    };
    jsast::utils::move_vector<jsast::ast::node> args;
    for (auto& arg : call.arguments()) {
      args.emplace_back(get_node(*arg));
    }
    return jsast::ast::call_expression{
        callee_map[static_cast<size_t>(call.func)](), std::move(args)};
  }

  auto get_jsast(ast::identifier& identifier) {
    return jsast::ast::identifier{identifier.name};
  }

  auto get_jsast(ast::variable_name& variable) {
    return jsast::ast::identifier{variable.name};
  }

  auto get_jsast(ast::number_literal& literal) {
    return jsast::ast::raw_literal{literal.value};
  }

  auto get_jsast(ast::string_literal& literal) {
    return jsast::ast::string_literal{literal.value};
  }

};  // namespace marlin::exec

}  // namespace marlin::exec

#endif  // marlin_exec_generator_hpp
