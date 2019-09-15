#ifndef marlin_exec_generator_hpp
#define marlin_exec_generator_hpp

#include <array>
#include <type_traits>
#include <unordered_set>
#include <utility>

#include <jsast/jsast.hpp>

#include "ast.hpp"
#include "exec_errors.hpp"

namespace marlin::exec {

struct generator {
  generator(bool is_async = false) : _is_async{is_async} {}

  std::string generate(ast::base& c) {
    jsast::generator gen;

    _errors.clear();
    gen.write(get_node(c));
    if (_errors.size()) {
      throw collected_generation_error{std::exchange(_errors, {})};
    } else {
      return std::move(gen).str();
    }
  }

 private:
  static constexpr const char* main_name{"__main__"};

  bool _is_async;
  std::unordered_set<std::string> _global_identifiers;
  std::vector<generation_error> _errors;

  static jsast::ast::node scoped_callee_with_name(std::string name) {
    return jsast::ast::member_expression{
        jsast::ast::object_expression{
            {jsast::ast::property{jsast::ast::member_identifier{"__thefunc__"},
                                  jsast::ast::identifier{std::move(name)}}}},
        jsast::ast::member_identifier{"__thefunc__"}};
  }

  static jsast::ast::node system_callee(std::string module, std::string name) {
    return jsast::ast::member_expression{
        jsast::ast::identifier{std::move(module)},
        jsast::ast::member_identifier{std::move(name)}};
  }

  static jsast::ast::node check_termination() {
    return jsast::ast::expression_statement{jsast::ast::call_expression{
        jsast::ast::member_expression{
            jsast::ast::identifier{"global_clock"},
            jsast::ast::member_identifier{"check_termination"}},
        {}}};
  }

  bool is_local_identifier(ast::base& node) {
    if (node.is<ast::identifier>() &&
        _global_identifiers.find(node.as<ast::identifier>().name) ==
            _global_identifiers.end()) {
      return true;
    } else if (node.is<ast::variable_name>() &&
               _global_identifiers.find(node.as<ast::variable_name>().name) ==
                   _global_identifiers.end()) {
      return true;
    } else {
      return false;
    }
  }

  template <typename vector_type>
  auto get_block(vector_type vector) {
    jsast::utils::move_vector<jsast::ast::node> statements;
    for (auto& statement : vector) {
      statements.emplace_back(get_node(*statement));
    }
    if (_is_async) {
      statements.emplace_back(check_termination());
    }
    return jsast::ast::block_statement{std::move(statements)};
  }

  jsast::ast::node get_node(ast::base& c) {
    return c.apply<jsast::ast::node>([this](auto& node) {
      return get_jsast(node, [&node](auto js_node) {
        return jsast::ast::node{
            std::move(js_node),
            [&node](source_range range) { node._js_range = range; }};
      });
    });
  }

  template <typename wrapper_type>
  auto get_jsast(ast::variable_placeholder& node, wrapper_type&& wrapper) {
    _errors.emplace_back("Unexpected placeholder!", node);
    return wrapper(jsast::ast::identifier{"__error__"});
  }

  template <typename wrapper_type>
  auto get_jsast(ast::expression_placeholder& node, wrapper_type&& wrapper) {
    _errors.emplace_back("Unexpected placeholder!", node);
    return wrapper(jsast::ast::identifier{"__error__"});
  }

  template <typename wrapper_type>
  auto get_jsast(ast::function_placeholder& node, wrapper_type&& wrapper) {
    // This should never be called
    _errors.emplace_back("Unexpected function signature!", node);
    return wrapper(jsast::ast::identifier{"__error__"});
  }

  template <typename wrapper_type>
  auto get_jsast(ast::function_signature& node, wrapper_type&& wrapper) {
    // This should never be called
    _errors.emplace_back("Unexpected function signature!", node);
    return wrapper(jsast::ast::identifier{"__error__"});
  }

  template <typename wrapper_type>
  auto get_jsast(ast::program& program, wrapper_type&& wrapper) {
    jsast::utils::move_vector<jsast::ast::node> blocks;
    for (auto& block : program.blocks()) {
      blocks.emplace_back(get_node(*block));
    }
    blocks.emplace_back(jsast::ast::expression_statement{
        jsast::ast::call_expression{jsast::ast::identifier{"execute"},
                                    {jsast::ast::identifier{main_name}}}});
    return wrapper(jsast::ast::program{std::move(blocks)});
  }

  template <typename wrapper_type>
  auto get_jsast(ast::on_start& start, wrapper_type&& wrapper) {
    _global_identifiers.clear();
    return wrapper(jsast::ast::function_declaration{
        main_name,
        {},
        jsast::ast::block_statement{get_block(start.statements())},
        true});
  }

  template <typename wrapper_type>
  auto get_jsast(ast::function& function, wrapper_type&& wrapper) {
    if (function.signature()->is<ast::function_signature>()) {
      auto& signature{function.signature()->as<ast::function_signature>()};
      auto name{"__func_" + signature.name};
      jsast::utils::move_vector<jsast::ast::node> params;
      for (auto& param : signature.parameters()) {
        params.emplace_back(get_node(*param));
      }
      _global_identifiers.clear();
      return wrapper(jsast::ast::function_declaration{
          std::move(name), std::move(params),
          jsast::ast::block_statement{get_block(function.statements())}, true});
    } else if (function.signature()->is<ast::function_placeholder>()) {
      _errors.emplace_back("Unexpected placeholder!", *function.signature());
      return wrapper(jsast::ast::empty_statement{});
    } else {
      _errors.emplace_back("Unexpected node, expecting function signature!",
                           *function.signature());
      return wrapper(jsast::ast::empty_statement{});
    }
  }

  template <typename wrapper_type>
  auto get_jsast(ast::assignment& assignment, wrapper_type&& wrapper) {
    if (is_local_identifier(*assignment.variable())) {
      return wrapper(jsast::ast::variable_declaration{
          {jsast::ast::variable_declarator{get_node(*assignment.variable()),
                                           get_node(*assignment.value())}},
          jsast::variable_declaration_type::var});
    } else {
      return wrapper(
          jsast::ast::expression_statement{jsast::ast::assignment_expression{
              get_node(*assignment.variable()), jsast::assignment_op::standard,
              get_node(*assignment.value())}});
    }
  }

  template <typename wrapper_type>
  auto get_jsast(ast::use_global& use_global, wrapper_type&& wrapper) {
    if (use_global.variable()->is<ast::variable_name>()) {
      auto name = use_global.variable()->as<ast::variable_name>().name;
      _global_identifiers.emplace(std::move(name));
    } else {
      _errors.emplace_back("Unexpected node, expecting variable name!",
                           use_global);
    }
    return wrapper(jsast::ast::empty_statement{});
  }

  template <typename wrapper_type>
  auto get_jsast(ast::system_procedure_call& call, wrapper_type&& wrapper) {
    static constexpr std::array<std::pair<jsast::ast::node (*)(), bool>, 9>
        callee_map{
            std::pair{[]() {
                        return jsast::ast::node{
                            jsast::ast::identifier{"sleep"}};
                      },
                      true} /* sleep */,
            std::pair{[]() {
                        return jsast::ast::node{
                            jsast::ast::identifier{"print"}};
                      },
                      false} /* print */,
            std::pair{[]() { return system_callee("graphics", "drawLine"); },
                      true} /* draw_line */,
            std::pair{[]() { return system_callee("logo", "forward"); },
                      true} /* logo_forward */,
            std::pair{[]() { return system_callee("logo", "backward"); },
                      true} /* logo_backward */,
            std::pair{[]() { return system_callee("logo", "turnLeft"); },
                      false} /* logo_turn_left */,
            std::pair{[]() { return system_callee("logo", "turnRight"); },
                      false} /* logo_turn_right */,
            std::pair{[]() { return system_callee("logo", "penUp"); },
                      false} /* logo_pen_up */,
            std::pair{[]() { return system_callee("logo", "penDown"); },
                      false} /* logo_pen_down */
        };
    jsast::utils::move_vector<jsast::ast::node> args;
    for (auto& arg : call.arguments()) {
      args.emplace_back(get_node(*arg));
    }
    auto [node_maker, async] = callee_map[static_cast<size_t>(call.proc)];
    if (async) {
      return wrapper(
          jsast::ast::expression_statement{jsast::ast::await_expression{
              jsast::ast::call_expression{node_maker(), std::move(args)}}});
    } else {
      return wrapper(jsast::ast::expression_statement{
          jsast::ast::call_expression{node_maker(), std::move(args)}});
    }
  }

  template <typename wrapper_type>
  auto get_jsast(ast::if_statement& statement, wrapper_type&& wrapper) {
    return wrapper(jsast::ast::if_statement{get_node(*statement.condition()),
                                            get_block(statement.statements())});
  }

  template <typename wrapper_type>
  auto get_jsast(ast::if_else_statement& statement, wrapper_type&& wrapper) {
    return wrapper(jsast::ast::if_statement{get_node(*statement.condition()),
                                            get_block(statement.consequence()),
                                            get_block(statement.alternate())});
  }

  template <typename wrapper_type>
  auto get_jsast(ast::while_statement& statement, wrapper_type&& wrapper) {
    return wrapper(jsast::ast::while_statement{
        get_node(*statement.condition()), get_block(statement.statements())});
  }

  template <typename wrapper_type>
  auto get_jsast(ast::for_statement& statement, wrapper_type&& wrapper) {
    if (is_local_identifier(*statement.variable())) {
      return wrapper(jsast::ast::for_of_statement{
          jsast::ast::variable_declaration{
              {jsast::ast::variable_declarator{
                  get_node(*statement.variable())}},
              jsast::variable_declaration_type::var},
          get_node(*statement.list()), get_block(statement.statements())});
    } else {
      return wrapper(jsast::ast::for_of_statement{
          get_node(*statement.variable()), get_node(*statement.list()),
          get_block(statement.statements())});
    }
  }

  template <typename wrapper_type>
  auto get_jsast(ast::break_statement&, wrapper_type&& wrapper) {
    return wrapper(jsast::ast::break_statement{});
  }

  template <typename wrapper_type>
  auto get_jsast(ast::continue_statement&, wrapper_type&& wrapper) {
    return wrapper(jsast::ast::continue_statement{});
  }

  template <typename wrapper_type>
  auto get_jsast(ast::unary_expression& unary, wrapper_type&& wrapper) {
    static constexpr std::array symbol_map{
        jsast::unary_op::negative /* negative */
    };
    return wrapper(
        jsast::ast::unary_expression{symbol_map[static_cast<uint8_t>(unary.op)],
                                     get_node(*unary.argument())});
  }

  template <typename wrapper_type>
  auto get_jsast(ast::binary_expression& binary, wrapper_type&& wrapper) {
    if (binary.op == ast::binary_op::logical_and) {
      return wrapper(jsast::ast::logical_expression{
          get_node(*binary.left()), jsast::logical_op::logical_and,
          get_node(*binary.right())});
    } else if (binary.op == ast::binary_op::logical_or) {
      return wrapper(jsast::ast::logical_expression{
          get_node(*binary.left()), jsast::logical_op::logical_or,
          get_node(*binary.right())});
    } else {
      static constexpr std::array symbol_map{
          jsast::binary_op::add /* add */,
          jsast::binary_op::subtract /* subtract */,
          jsast::binary_op::multiply /* multiply */,
          jsast::binary_op::divide /* divide */,
          jsast::binary_op::strict_equal /* equal */,
          jsast::binary_op::strict_not_equal /* not_equal */,
          jsast::binary_op::less /* less */,
          jsast::binary_op::less_equal /* less_equal */,
          jsast::binary_op::greater /* greater */,
          jsast::binary_op::greater_equal /* greater_equal */
      };
      return wrapper(jsast::ast::binary_expression{
          get_node(*binary.left()), symbol_map[static_cast<uint8_t>(binary.op)],
          get_node(*binary.right())});
    }
  }

  template <typename wrapper_type>
  auto get_jsast(ast::system_function_call& call, wrapper_type&& wrapper) {
    static constexpr std::array<jsast::ast::node (*)(), 6> callee_map{
        []() {
          return jsast::ast::node{jsast::ast::identifier{"range"}};
        } /* range1 */
        ,
        []() {
          return jsast::ast::node{jsast::ast::identifier{"range"}};
        } /* range2 */,
        []() {
          return jsast::ast::node{jsast::ast::identifier{"range"}};
        } /* range3 */,
        []() {
          return jsast::ast::node{jsast::ast::member_expression{
              jsast::ast::identifier{"Date"},
              jsast::ast::member_identifier{"now"}}};
        } /* time */,
        []() {
          return jsast::ast::node{jsast::ast::member_expression{
              jsast::ast::identifier{"math_extra"},
              jsast::ast::member_identifier{"sin"}}};
        } /* sin */,
        []() {
          return jsast::ast::node{jsast::ast::member_expression{
              jsast::ast::identifier{"math_extra"},
              jsast::ast::member_identifier{"cos"}}};
        } /* cos */
    };
    jsast::utils::move_vector<jsast::ast::node> args;
    for (auto& arg : call.arguments()) {
      args.emplace_back(get_node(*arg));
    }
    return wrapper(jsast::ast::call_expression{
        callee_map[static_cast<size_t>(call.func)](), std::move(args)});
  }

  template <typename wrapper_type>
  auto get_identifier(std::string name, wrapper_type&& wrapper) {
    if (_global_identifiers.find(name) != _global_identifiers.end()) {
      return wrapper(jsast::ast::member_expression{
          jsast::ast::identifier{"globals"},
          jsast::ast::member_identifier{std::move(name)}});
    } else {
      return wrapper(jsast::ast::identifier{"__var_" + std::move(name)});
    }
  }

  template <typename wrapper_type>
  auto get_jsast(ast::identifier& identifier, wrapper_type&& wrapper) {
    return get_identifier(identifier.name, std::forward<wrapper_type>(wrapper));
  }

  template <typename wrapper_type>
  auto get_jsast(ast::variable_name& variable, wrapper_type&& wrapper) {
    return get_identifier(variable.name, std::forward<wrapper_type>(wrapper));
  }

  template <typename wrapper_type>
  auto get_jsast(ast::number_literal& literal, wrapper_type&& wrapper) {
    return wrapper(jsast::ast::raw_literal{literal.value});
  }

  template <typename wrapper_type>
  auto get_jsast(ast::string_literal& literal, wrapper_type&& wrapper) {
    return wrapper(jsast::ast::string_literal{literal.value});
  }

};  // namespace marlin::exec

}  // namespace marlin::exec

#endif  // marlin_exec_generator_hpp
