#ifndef marlin_exec_generator_hpp
#define marlin_exec_generator_hpp

#include <algorithm>
#include <array>
#include <deque>
#include <type_traits>
#include <unordered_set>
#include <utility>
#include <vector>

#include <jsast/jsast.hpp>

#include "ast.hpp"
#include "exec_errors.hpp"

namespace marlin::exec {

struct generator {
  std::string generate(ast::base& c) {
    assert(c.is<ast::program>());
    jsast::generator gen;

    // Mark async blocks
    _async_blocks.clear();
    _user_functions.clear();
    _user_function_callees.clear();
    for (auto& block : c.children()) {
      record_calls(*block, *block);

      if (block->is<ast::function>()) {
        auto signature{block->as<ast::function>().signature()};
        if (signature->is<ast::function_signature>()) {
          _user_functions[signature->as<ast::function_signature>().name] =
              &*block;
        }
      }
    }
    {
      std::deque<ast::base*> queue;
      std::copy(_async_blocks.begin(), _async_blocks.end(),
                std::back_inserter(queue));
      while (!queue.empty()) {
        auto block{queue.front()};
        queue.pop_front();

        if (block->is<ast::function>()) {
          auto signature{block->as<ast::function>().signature()};
          if (signature->is<ast::function_signature>()) {
            auto it{_user_function_callees.find(
                signature->as<ast::function_signature>().name)};
            if (it != _user_function_callees.end()) {
              for (auto block : it->second) {
                if (_async_blocks.find(block) == _async_blocks.end()) {
                  _async_blocks.emplace(block);
                  queue.emplace_back(block);
                }
              }
            }
          }
        }
      }
    }

    _errors.clear();
    gen.write(get_node(c));

    _async_blocks.clear();
    _user_functions.clear();
    _user_function_callees.clear();

    if (_errors.size()) {
      throw collected_generation_error{std::exchange(_errors, {})};
    } else {
      return std::move(gen).str();
    }
  }

 private:
  static constexpr const char* main_name{"__main__"};

  static std::string user_function_name(std::string name) {
    return "__func_" + std::move(name);
  }

  static jsast::ast::node env_name(std::string name) {
    return jsast::ast::member_expression{
        jsast::ast::member_expression{jsast::ast::identifier{"window"},
                                      jsast::ast::member_identifier{"env"}},
        jsast::ast::member_identifier{std::move(name)}};
  }

  static jsast::ast::node system_callee(std::string module, std::string name) {
    return jsast::ast::member_expression{
        env_name(std::move(module)),
        jsast::ast::member_identifier{std::move(name)}};
  }

  using callee_entry = std::pair<jsast::ast::node (*)(), bool>;
  static constexpr std::array<callee_entry, 13> system_procedure_callee_map{
      std::pair{[]() { return env_name("sleep"); }, true} /* sleep */,
      std::pair{[]() { return env_name("print"); }, false} /* print */,
      std::pair{[]() { return system_callee("array_utils", "append"); },
                false} /* list_append */,
      std::pair{[]() { return system_callee("array_utils", "insert"); },
                false} /* list_insert */,
      std::pair{[]() { return system_callee("array_utils", "remove"); },
                false} /* list_remove */,
      std::pair{[]() { return system_callee("graphics", "drawLine"); },
                true} /* draw_line */,
      std::pair{[]() { return system_callee("graphics", "setLineWidth"); },
                false} /* set_line_width */,
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
  static constexpr std::array<callee_entry, 18> system_function_callee_map{
      std::pair{[]() { return env_name("range"); }, false} /* range1 */,
      std::pair{[]() { return env_name("range"); }, false} /* range2 */,
      std::pair{[]() { return env_name("range"); }, false} /* range3 */,
      std::pair{[]() { return system_callee("array_utils", "length"); },
                false} /* list_length */,
      std::pair{[]() { return env_name("time"); }, false} /* time */,
      std::pair{[]() { return system_callee("math_utils", "abs"); },
                false} /* abs */,
      std::pair{[]() { return system_callee("math_utils", "sqrt"); },
                false} /* sqrt */,
      std::pair{[]() { return system_callee("math_utils", "sin"); },
                false} /* sin */,
      std::pair{[]() { return system_callee("math_utils", "cos"); },
                false} /* cos */,
      std::pair{[]() { return system_callee("math_utils", "tan"); },
                false} /* tan */,
      std::pair{[]() { return system_callee("math_utils", "asin"); },
                false} /* asin */,
      std::pair{[]() { return system_callee("math_utils", "acos"); },
                false} /* acos */,
      std::pair{[]() { return system_callee("math_utils", "atan"); },
                false} /* atan */,
      std::pair{[]() { return system_callee("math_utils", "ln"); },
                false} /* ln */,
      std::pair{[]() { return system_callee("math_utils", "log"); },
                false} /* log */,
      std::pair{[]() { return system_callee("math_utils", "round"); },
                false} /* round */,
      std::pair{[]() { return system_callee("math_utils", "floor"); },
                false} /* floor */,
      std::pair{[]() { return system_callee("math_utils", "ceil"); },
                false} /* ceil */
  };

  std::unordered_set<ast::base*> _async_blocks;
  std::unordered_map<std::string_view, ast::base*> _user_functions;
  std::unordered_map<std::string_view, std::unordered_set<ast::base*>>
      _user_function_callees;

  std::unordered_set<std::string_view> _global_identifiers;
  std::vector<generation_error> _errors;

  void record_calls(ast::base& node, ast::base& block) {
    node.apply<void>([this, &block](auto& n) { record_if_is_call(n, block); });
    for (auto& child : node.children()) {
      record_calls(*child, block);
    }
  }

  template <typename node_type>
  void record_if_is_call(node_type&, ast::base&) {}

  void record_if_is_call(ast::system_procedure_call& call, ast::base& block) {
    auto [node_maker,
          async]{system_procedure_callee_map[static_cast<size_t>(call.proc)]};
    if (async) {
      _async_blocks.emplace(&block);
    }
  }
  void record_if_is_call(ast::system_function_call& call, ast::base& block) {
    auto [node_maker,
          async]{system_function_callee_map[static_cast<size_t>(call.func)]};
    if (async) {
      _async_blocks.emplace(&block);
    }
  }
  void record_if_is_call(ast::user_function_call& call, ast::base& block) {
    _user_function_callees[call.name].emplace(&block);
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
    blocks.emplace_back(
        jsast::ast::expression_statement{jsast::ast::call_expression{
            env_name("execute"), {jsast::ast::identifier{main_name}}}});
    return wrapper(jsast::ast::program{std::move(blocks)});
  }

  template <typename wrapper_type>
  auto get_jsast(ast::on_start& start, wrapper_type&& wrapper) {
    _global_identifiers.clear();
    auto block{get_block(start.statements())};

    auto async{_async_blocks.find(&start) != _async_blocks.end()};
    return wrapper(jsast::ast::function_declaration{
        main_name, {}, jsast::ast::block_statement{std::move(block)}, async});
  }

  template <typename wrapper_type>
  auto get_jsast(ast::parameter& param, wrapper_type&& wrapper) {
    return wrapper(jsast::ast::identifier{"__var_" + param.name});
  }

  template <typename wrapper_type>
  auto get_jsast(ast::function& function, wrapper_type&& wrapper) {
    if (function.signature()->is<ast::function_signature>()) {
      auto& signature{function.signature()->as<ast::function_signature>()};
      jsast::utils::move_vector<jsast::ast::node> params;
      for (auto& param : signature.parameters()) {
        params.emplace_back(get_node(*param));
      }

      _global_identifiers.clear();
      auto block{get_block(function.statements())};

      auto async{_async_blocks.find(&function) != _async_blocks.end()};
      return wrapper(jsast::ast::function_declaration{
          user_function_name(signature.name), std::move(params),
          jsast::ast::block_statement{std::move(block)}, async});
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
  auto get_jsast(ast::eval_statement& eval, wrapper_type&& wrapper) {
    return wrapper(
        jsast::ast::expression_statement{get_node(*eval.expression())});
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
      auto& name = use_global.variable()->as<ast::variable_name>().name;
      _global_identifiers.emplace(name);
    } else {
      _errors.emplace_back("Unexpected node, expecting variable name!",
                           use_global);
    }
    return wrapper(jsast::ast::empty_statement{});
  }

  template <typename wrapper_type>
  auto get_jsast(ast::system_procedure_call& call, wrapper_type&& wrapper) {
    jsast::utils::move_vector<jsast::ast::node> args;
    for (auto& arg : call.arguments()) {
      args.emplace_back(get_node(*arg));
    }
    auto [node_maker,
          async]{system_procedure_callee_map[static_cast<size_t>(call.proc)]};
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
  auto get_jsast(ast::return_statement&, wrapper_type&& wrapper) {
    return wrapper(jsast::ast::return_statement{});
  }

  template <typename wrapper_type>
  auto get_jsast(ast::return_result_statement& statement,
                 wrapper_type&& wrapper) {
    return wrapper(jsast::ast::return_statement{get_node(*statement.result())});
  }

  template <typename wrapper_type>
  auto get_jsast(ast::unary_expression& unary, wrapper_type&& wrapper) {
    static constexpr std::array symbol_map{
        jsast::unary_op::negative /* negative */
        ,
        jsast::unary_op::logical_not /* logical_not */,
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

  template <typename subscript_type, typename wrapper_type>
  auto get_subscript(subscript_type& subscript, wrapper_type&& wrapper) {
    auto list{get_node(*subscript.list())};
    auto index{get_node(*subscript.index())};
    return wrapper(jsast::ast::member_expression{
        jsast::ast::call_expression{env_name("as_array"), {std::move(list)}},
        std::move(index)});
  }

  template <typename wrapper_type>
  auto get_jsast(ast::subscript_set& subscript, wrapper_type&& wrapper) {
    return get_subscript(subscript, wrapper);
  }

  template <typename wrapper_type>
  auto get_jsast(ast::subscript_get& subscript, wrapper_type&& wrapper) {
    return get_subscript(subscript, wrapper);
  }

  template <typename wrapper_type>
  auto get_jsast(ast::new_array& init, wrapper_type&& wrapper) {
    jsast::utils::move_vector<std::optional<jsast::ast::node>> elems;
    for (auto& elem : init.elements()) {
      elems.emplace_back(get_node(*elem));
    }
    return wrapper(jsast::ast::array_expression{std::move(elems)});
  }

  template <typename wrapper_type>
  auto get_jsast(ast::system_function_call& call, wrapper_type&& wrapper) {
    jsast::utils::move_vector<jsast::ast::node> args;
    for (auto& arg : call.arguments()) {
      args.emplace_back(get_node(*arg));
    }
    auto [node_maker,
          async]{system_function_callee_map[static_cast<size_t>(call.func)]};
    if (async) {
      return wrapper(jsast::ast::await_expression{
          jsast::ast::call_expression{node_maker(), std::move(args)}});
    } else {
      return wrapper(
          jsast::ast::call_expression{node_maker(), std::move(args)});
    }
  }

  template <typename wrapper_type>
  auto get_jsast(ast::user_function_call& call, wrapper_type&& wrapper) {
    if (call.func() != nullptr) {
      auto arguments = call.arguments();
      if (arguments.size() == call.func()->parameters.size()) {
        jsast::utils::move_vector<jsast::ast::node> args;
        for (auto& arg : call.arguments()) {
          args.emplace_back(get_node(*arg));
        }

        assert(_user_functions.find(call.name) != _user_functions.end());
        if (_async_blocks.find(_user_functions[call.name]) ==
            _async_blocks.end()) {
          return wrapper(jsast::ast::call_expression{
              jsast::ast::identifier{user_function_name(call.name)},
              std::move(args)});
        } else {
          return wrapper(
              jsast::ast::await_expression{jsast::ast::call_expression{
                  jsast::ast::identifier{user_function_name(call.name)},
                  std::move(args)}});
        }
      } else {
        _errors.emplace_back("Incorrect number of arguments!", call);
        return wrapper(jsast::ast::identifier{"__error__"});
      }
    } else {
      _errors.emplace_back("Call to unknown user function!", call);
      return wrapper(jsast::ast::identifier{"__error__"});
    }
  }

  template <typename wrapper_type>
  auto get_identifier(std::string name, wrapper_type&& wrapper) {
    if (_global_identifiers.find(name) != _global_identifiers.end()) {
      return wrapper(jsast::ast::member_expression{
          env_name("globals"), jsast::ast::member_identifier{std::move(name)}});
    } else {
      return wrapper(jsast::ast::identifier{"__var_" + std::move(name)});
    }
  }

  template <typename wrapper_type>
  auto get_jsast(ast::variable_name& variable, wrapper_type&& wrapper) {
    return get_identifier(variable.name, std::forward<wrapper_type>(wrapper));
  }

  template <typename wrapper_type>
  auto get_jsast(ast::identifier& identifier, wrapper_type&& wrapper) {
    return get_identifier(identifier.name, std::forward<wrapper_type>(wrapper));
  }

  template <typename wrapper_type>
  auto get_jsast(ast::number_literal& literal, wrapper_type&& wrapper) {
    return wrapper(jsast::ast::raw_literal{literal.value});
  }

  template <typename wrapper_type>
  auto get_jsast(ast::string_literal& literal, wrapper_type&& wrapper) {
    return wrapper(jsast::ast::string_literal{literal.value});
  }
};

}  // namespace marlin::exec

#endif  // marlin_exec_generator_hpp
