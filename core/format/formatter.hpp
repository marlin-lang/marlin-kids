#ifndef marlin_format_formatter_hpp
#define marlin_format_formatter_hpp

#include <string>
#include <vector>

#include "ast.hpp"
#include "node.hpp"
#include "specs.hpp"

namespace marlin::format {

enum struct highlight_token_type {
  keyword,
  op,
  boolean,
  number,
  string,
  placeholder
};

struct highlight_token {
  highlight_token_type type;
  size_t offset;
  size_t length;

  highlight_token(highlight_token_type _type, size_t _offset, size_t _length)
      : type(_type), offset(_offset), length(_length) {}
};

struct display {
  std::string source;
  std::vector<highlight_token> highlights;

  display(std::string _source, std::vector<highlight_token> _highlights)
      : source{std::move(_source)}, highlights{std::move(_highlights)} {}
};

struct formatter {
  template <typename input_type>
  display format(input_type&& nodes, const ast::base& target) {
    size_t paren_precedence{0};
    if (target.has_parent()) {
      auto& parent{target.parent()};
      if (parent.is<ast::unary_expression>()) {
        paren_precedence = ast::unary_op_precedence;
      } else if (parent.is<ast::binary_expression>()) {
        auto& binary{parent.as<ast::binary_expression>()};
        if (&target == binary.left().get()) {
          paren_precedence = precedence_for(binary.op) - 1;
        } else {
          paren_precedence = precedence_for(binary.op);
        }
      }
    }

    return format(std::forward<input_type>(nodes),
                  target.source_code_range.begin, 0, paren_precedence);
  }

  template <typename input_type>
  display format(input_type&& nodes, size_t start_line = 1,
                 const ast::base* parent = nullptr) {
    size_t indent{0};
    if (parent != nullptr) {
      const ast::base* curr_parent{parent};
      while (curr_parent->has_parent()) {
        indent++;
        curr_parent = &curr_parent->parent();
      }
    }
    return format(std::forward<input_type>(nodes), {start_line, 1}, indent, 0);
  }

 private:
  std::string _source_buffer;
  std::vector<highlight_token> _highlights;
  source_loc _current_loc;
  size_t _indent;

  template <typename input_type>
  display format(input_type&& nodes, source_loc start, size_t indent,
                 size_t paren_precedence) {
    _source_buffer.clear();
    _highlights.clear();
    _current_loc = start;
    _indent = indent;
    if constexpr (std::is_same_v<std::decay_t<input_type>, ast::base>) {
      emit_node(nodes);
    } else if constexpr (std::is_same_v<std::decay_t<input_type>, ast::node>) {
      emit_node(*nodes);
    } else {
      // nodes is vector
      assert(nodes.size() != 0);
      emit_vector(nodes, paren_precedence);
    }

    return {std::exchange(_source_buffer, {}), std::exchange(_highlights, {})};
  }

  void emit_string(std::string_view string) {
    _source_buffer.append(string);

    // For now, assume that "\n" will be handled only in emit_new_line
    _current_loc.column += string.size();
  }

  void emit_indent() {
    for (size_t i{0}; i < _indent; i++) {
      emit_string("  ");
    }
  }

  void emit_new_line() {
    emit_string("\n");
    _current_loc = {_current_loc.line + 1, 1};
  }

  void emit_placeholder(std::string_view name) {
    _highlights.emplace_back(highlight_token_type::placeholder,
                             _source_buffer.size(), name.size() + 1);
    emit_string("@");
    emit_string(std::move(name));
  }

  void emit_highlight(std::string_view string, highlight_token_type type) {
    _highlights.emplace_back(type, _source_buffer.size(), string.size());
    emit_string(std::move(string));
  }

  template <typename vector_type>
  void emit_vector(vector_type&& vector, size_t paren_precedence = 0) {
    for (auto& node : vector) {
      emit_node(*node, paren_precedence);
    }
  }

  template <typename vector_type>
  void emit_arguments(vector_type&& args) {
    emit_string("(");
    bool first{true};
    for (auto& arg : args) {
      if (first) {
        first = false;
      } else {
        emit_string(", ");
      }
      emit_node(*arg);
    }
    emit_string(")");
  }

  void emit_node(ast::base& node, size_t paren_precedence = 0) {
    const bool is_line{node.inherits<ast::block>() ||
                       node.inherits<ast::statement>()};
    if (is_line) {
      emit_indent();
    }
    node.source_code_range.begin = _current_loc;
    node.apply<void>(
        [this, &paren_precedence](auto& n) { emit_node(n, paren_precedence); });
    node.source_code_range.end = _current_loc;
    if (is_line) {
      emit_new_line();
    }
  }

  void emit_node(ast::program& program, size_t) {
    emit_vector(program.blocks());
  }

  void emit_node(ast::on_start& on_start, size_t) {
    emit_highlight("on start", highlight_token_type::keyword);
    emit_string(" {");
    emit_new_line();
    _indent++;
    emit_vector(on_start.statements());
    _indent--;
    emit_indent();
    emit_string("}");
  }

  void emit_node(ast::function_placeholder& placeholder, size_t) {
    emit_placeholder(placeholder.name);
    emit_string("()");
  }

  void emit_node(ast::function_signature& signature, size_t) {
    emit_string(signature.name);
    emit_arguments(signature.parameters());
  }

  void emit_node(ast::function& function, size_t) {
    emit_highlight("func", highlight_token_type::keyword);
    emit_string(" ");
    emit_node(*function.signature());
    emit_string(" {");
    emit_new_line();
    _indent++;
    emit_vector(function.statements());
    _indent--;
    emit_indent();
    emit_string("}");
  }

  void emit_node(ast::assignment& assignment, size_t) {
    emit_node(*assignment.variable());
    emit_string(" = ");
    emit_node(*assignment.value());
    emit_string(";");
  }

  void emit_node(ast::use_global& use_global, size_t) {
    emit_highlight("use global", highlight_token_type::keyword);
    emit_string(" ");
    emit_node(*use_global.variable());
    emit_string(";");
  }

  void emit_node(ast::system_procedure_call& call, size_t) {
    emit_string(display_for(call.proc));
    emit_arguments(call.arguments());
    emit_string(";");
  }

  void emit_node(ast::if_statement& statement, size_t) {
    emit_highlight("if", highlight_token_type::keyword);
    emit_string(" (");
    emit_node(*statement.condition());
    emit_string(") {");
    emit_new_line();
    _indent++;
    emit_vector(statement.statements());
    _indent--;
    emit_indent();
    emit_string("}");
  }

  void emit_node(ast::if_else_statement& statement, size_t) {
    emit_highlight("if", highlight_token_type::keyword);
    emit_string(" (");
    emit_node(*statement.condition());
    emit_string(") {");
    emit_new_line();
    _indent++;
    emit_vector(statement.consequence());
    _indent--;
    emit_indent();
    emit_string("} ");
    statement.else_loc = _current_loc;
    emit_highlight("else", highlight_token_type::keyword);
    emit_string(" {");
    emit_new_line();
    _indent++;
    emit_vector(statement.alternate());
    _indent--;
    emit_indent();
    emit_string("}");
  }

  void emit_code(ast::while_statement& statement, size_t) {
    emit_highlight("while", highlight_token_type::keyword);
    emit_string(" (");
    emit_node(*statement.condition());
    emit_string(") {");
    emit_new_line();
    _indent++;
    emit_vector(statement.statements());
    _indent--;
    emit_indent();
    emit_string("}");
  }

  void emit_node(ast::for_statement& statement, size_t) {
    emit_highlight("for", highlight_token_type::keyword);
    emit_string(" (");
    emit_node(*statement.variable());
    emit_string(" in ");
    emit_node(*statement.list());
    emit_string(") {");
    emit_new_line();
    _indent++;
    emit_vector(statement.statements());
    _indent--;
    emit_indent();
    emit_string("}");
  }

  void emit_node(ast::break_statement&, size_t) {
    emit_highlight("break", highlight_token_type::keyword);
    emit_string(";");
  }

  void emit_node(ast::continue_statement&, size_t) {
    emit_highlight("continue", highlight_token_type::keyword);
    emit_string(";");
  }

  void emit_node(ast::variable_placeholder& placeholder, size_t) {
    emit_placeholder(placeholder.name);
  }

  void emit_node(ast::variable_name& variable, size_t) {
    emit_string(variable.name);
  }

  void emit_node(ast::expression_placeholder& placeholder, size_t) {
    emit_placeholder(placeholder.name);
  }

  void emit_node(ast::unary_expression& unary, size_t paren_precedence) {
    if (ast::unary_op_precedence <= paren_precedence) {
      emit_string("(");
    }
    emit_highlight(symbol_for(unary.op), highlight_token_type::op);
    emit_node(*unary.argument(), ast::unary_op_precedence);
    if (ast::unary_op_precedence <= paren_precedence) {
      emit_string(")");
    }
  }

  void emit_node(ast::binary_expression& binary, size_t paren_precedence) {
    const size_t op_precedence{ast::precedence_for(binary.op)};
    if (op_precedence <= paren_precedence) {
      emit_string("(");
    }
    emit_node(*binary.left(), op_precedence - 1);
    emit_string(" ");
    auto highlight = highlight_token_type::op;
    if (binary.op == ast::binary_op::logical_and ||
        binary.op == ast::binary_op::logical_or) {
      highlight = highlight_token_type::keyword;
    }
    emit_highlight(symbol_for(binary.op), highlight);
    emit_string(" ");
    emit_node(*binary.right(), op_precedence);
    if (op_precedence <= paren_precedence) {
      emit_string(")");
    }
  }

  void emit_node(ast::system_function_call& call, size_t) {
    emit_string(display_for(call.func));
    emit_arguments(call.arguments());
  }

  void emit_node(ast::user_function_call& call, size_t) {
    emit_string(call.name);
    emit_arguments(call.arguments());
  }

  void emit_node(ast::identifier& identifier, size_t) {
    emit_string(identifier.name);
  }

  void emit_node(ast::number_literal& literal, size_t) {
    emit_highlight(literal.value, highlight_token_type::number);
  }

  void emit_node(ast::string_literal& literal, size_t) {
    emit_highlight(quoted(literal.value), highlight_token_type::string);
  }
};

}  // namespace marlin::format

#endif  // marlin_format_formatter_hpp