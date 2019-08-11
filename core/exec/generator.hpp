#ifndef marlin_exec_generator_hpp
#define marlin_exec_generator_hpp

#include <type_traits>

#include <jsast/jsast.hpp>

#include "ast.hpp"
#include "errors.hpp"

namespace marlin::exec {

struct generator {
  static inline std::string generate(ast::base& c) {
    jsast::generator gen;
    gen.write(get_node(c));
    return std::move(gen).str();
  }

 private:
  static constexpr const char* main_name{"__main__"};

  static inline jsast::ast::node scoped_callee_with_name(std::string name) {
    return jsast::ast::member_expression{
        jsast::ast::object_expression{
            {jsast::ast::property{jsast::ast::member_identifier{"__thefunc__"},
                                  jsast::ast::identifier{std::move(name)}}}},
        jsast::ast::member_identifier{"__thefunc__"}};
  }

  template <typename vector_type>
  static inline auto get_block(vector_type vector) {
    jsast::utils::move_vector<jsast::ast::node> statements;
    for (auto& statement : vector) {
      statements.emplace_back(get_node(*statement));
    }
    return jsast::ast::block_statement{std::move(statements)};
  }

  static inline jsast::ast::node get_node(ast::base& c) {
    return c.apply<jsast::ast::node>([](auto& node) {
      return jsast::ast::node{get_jsast(node), [&node](source_range range) {
                                node._js_range = range;
                              }};
    });
  }

  // Unused return value type, specified so that the code compiles
  [[noreturn]] static inline jsast::ast::empty_statement get_jsast(
      ast::variable_placeholder& node) {
    throw generation_error{"Unparsed chunk encountered!", node};
  }

  // Unused return value type, specified so that the code compiles
  [[noreturn]] static inline jsast::ast::empty_statement get_jsast(
      ast::expression_placeholder& node) {
    throw generation_error{"Unexpected placeholder!", node};
  }

  static inline auto get_jsast(ast::program& program) {
    jsast::utils::move_vector<jsast::ast::node> blocks;
    for (auto& block : program.blocks()) {
      blocks.emplace_back(get_node(*block));
    }
    blocks.emplace_back(
        jsast::ast::call_expression{scoped_callee_with_name(main_name), {}});
    return jsast::ast::program{std::move(blocks)};
  }

  static inline auto get_jsast(ast::on_start& start) {
    return jsast::ast::function_declaration{
        main_name,
        {},
        jsast::ast::block_statement{get_block(start.statements())}};
  }

  static inline auto get_jsast(ast::assignment& assignment) {
    return jsast::ast::expression_statement{jsast::ast::assignment_expression{
        get_node(*assignment.variable()), jsast::assignment_op::standard,
        get_node(*assignment.value())}};
  }

  static inline auto get_jsast(ast::print_statement& statement) {
    return jsast::ast::expression_statement{jsast::ast::call_expression{
        jsast::ast::identifier{"print"}, {get_node(*statement.value())}}};
  }

  static inline auto get_jsast(ast::if_statement& statement) {
    return jsast::ast::if_statement{get_node(*statement.condition()),
                                    get_block(statement.statements())};
  }

  static inline auto get_jsast(ast::if_else_statement& statement) {
    return jsast::ast::if_statement{get_node(*statement.condition()),
                                    get_block(statement.consequence()),
                                    get_block(statement.alternate())};
  }

  static inline auto get_jsast(ast::unary_expression& unary) {
    static constexpr jsast::unary_op symbol_map[]{
        jsast::unary_op::negative /* negative */
    };
    return jsast::ast::unary_expression{
        symbol_map[static_cast<uint8_t>(unary.op)],
        get_node(*unary.argument())};
  }

  static inline auto get_jsast(ast::binary_expression& binary) {
    static constexpr jsast::binary_op symbol_map[]{
        jsast::binary_op::add /* add */,
        jsast::binary_op::subtract /* subtract */,
        jsast::binary_op::multiply /* multiply */,
        jsast::binary_op::divide /* divide */
    };
    return jsast::ast::binary_expression{
        get_node(*binary.left()), symbol_map[static_cast<uint8_t>(binary.op)],
        get_node(*binary.right())};
  }

  static inline auto get_jsast(ast::identifier& identifier) {
    return jsast::ast::identifier{identifier.name};
  }

  static inline auto get_jsast(ast::variable_name& variable) {
    return jsast::ast::identifier{variable.name};
  }

  static inline auto get_jsast(ast::number_literal& literal) {
    return jsast::ast::raw_literal{literal.value};
  }

  static inline auto get_jsast(ast::string_literal& literal) {
    return jsast::ast::string_literal{literal.value};
  }

};  // namespace marlin::exec

}  // namespace marlin::exec

#endif  // marlin_exec_generator_hpp