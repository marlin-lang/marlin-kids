#ifndef marlin_store_v1_store_hpp
#define marlin_store_v1_store_hpp

#include <string_view>
#include <unordered_map>

#include "base.hpp"
#include "specs.hpp"
#include "store_definition.hpp"
#include "store_errors.hpp"

namespace marlin::store {

namespace v1 {

namespace key {

inline const std::string_view program{"program"};
inline const std::string_view on_start{"on_start"};

inline const std::string_view assignment{"assign"};
inline const std::string_view print{"print"};

inline const std::string_view if_else{"if"};

inline const std::string_view placeholder{"placeholder"};
inline const std::string_view identifier{"id"};

inline const std::string_view unary{"unary"};
inline const std::string_view binary{"binary"};

inline const std::string_view number{"number"};
inline const std::string_view string{"string"};

}  // namespace key

struct store : base_store::impl<store> {
  bool recognize(std::string_view data) override {
    return data.compare(0, _data_prefix.size(), _data_prefix) == 0;
  }

  reconstruction_result read(std::string_view data, source_loc start,
                             size_t indent, type_expectation type,
                             size_t paren_precedence) override {
    assert(recognize(data));

    _buffer.clear();
    _highlights.clear();
    _current_loc = start;
    _indent = indent;
    auto current{data.begin() + _data_prefix.size()};
    auto nodes{read_vector(current, data.end(), type, paren_precedence)};

    reconstruction_result result{std::move(nodes), std::move(_buffer),
                                 std::move(_highlights)};
    _buffer = {};
    _highlights = {};
    return result;
  }

  std::string write(std::vector<const ast::base*> nodes) {
    _buffer = _data_prefix;
    write_vector(nodes);
    auto result{std::move(_buffer)};
    _buffer = {};
    return result;
  }

 private:
  struct read_node_entry {
    using callable_type = ast::node (store::*)(const char*&, const char*,
                                               type_expectation, size_t);

    callable_type callable;
    bool is_statement;

    read_node_entry(callable_type _callable, bool _is_statement)
        : callable{_callable}, is_statement{_is_statement} {}
  };

  static constexpr std::string_view _data_prefix{"MKB\1"};

  std::string _buffer;
  std::vector<highlight_token> _highlights;
  source_loc _current_loc;
  size_t _indent;

  template <type_expectation... expect_types>
  void assert_type(type_expectation type, std::string message) {
    if (type != type_expectation::any && ((type != expect_types) && ...)) {
      throw read_error{std::move(message)};
    }
  }

  void emit_to_buffer(std::string_view string) {
    _buffer.append(string);

    // For now, assume that "\n" will be handled only in emit_new_line
    _current_loc.column += string.size();

    /* This line counting algorithm may be used later
      size_t column_count{0};
      bool meet_line_end{false};
      for (auto iter{string.rbegin()}; iter < string.rend(); iter++) {
        if (*iter == '\n') {
          if (!meet_line_end) {
            meet_line_end = true;
          }
          _current_loc.line++;
        } else if (!meet_line_end) {
          column_count++;
        }
      }
      if (meet_line_end) {
        _current_loc.column = column_count + 1;
      } else {
        _current_loc.column += column_count;
      }
    */
  }

  void emit_indent() {
    for (size_t i{0}; i < _indent; i++) {
      emit_to_buffer("  ");
    }
  }

  void emit_new_line() {
    emit_to_buffer("\n");
    _current_loc = {_current_loc.line + 1, 1};
  }

  void emit_placeholder(std::string_view name) {
    _highlights.emplace_back(highlight_token_type::placeholder, _buffer.size(),
                             name.size() + 1);
    emit_to_buffer("@");
    emit_to_buffer(std::move(name));
  }

  void emit_highlight(std::string_view string, highlight_token_type type) {
    _highlights.emplace_back(type, _buffer.size(), string.size());
    emit_to_buffer(std::move(string));
  }

  std::string_view read_zero_terminated(const char*& iter, const char* end) {
    auto begin{iter};
    while (iter < end) {
      if (*iter == '\0') {
        std::string_view result{begin, static_cast<size_t>(iter - begin)};
        iter++;
        return result;
      } else {
        iter++;
      }
    }
    throw read_error{"Unterminating character sequence!"};
  }

  bool read_bool(const char*& iter, const char* end) {
    if (iter < end) {
      return static_cast<uint8_t>(*iter++);
    } else {
      throw read_error{"End of file when expecting boolean!"};
    }
  }

  uint32_t read_int(const char*& iter, const char* end) {
    if (iter + 4 <= end) {
      uint32_t result{static_cast<uint8_t>(*iter++)};
      for (size_t i{0}; i < 3; i++) {
        result <<= 8;
        result |= static_cast<uint8_t>(*iter++);
      }
      return result;
    } else {
      throw read_error{"End of file when expecting integer!"};
    }
  }

  std::string_view read_string(const char*& iter, const char* end) {
    auto length{read_int(iter, end)};
    if (iter + length <= end) {
      std::string_view result{iter, length};
      iter += length;
      return result;
    } else {
      throw read_error{"End of file when expecting string!"};
    }
  }

  std::vector<ast::node> read_vector(const char*& iter, const char* end,
                                     type_expectation type,
                                     size_t paren_precedence = 0) {
    auto length{read_int(iter, end)};
    std::vector<ast::node> result;
    result.reserve(length);
    for (size_t i{0}; i < length; i++) {
      result.emplace_back(read_node(iter, end, type, paren_precedence));
    }
    return result;
  }

  ast::node read_node(const char*& iter, const char* end, type_expectation type,
                      size_t paren_precedence = 0) {
    static const std::unordered_map<std::string_view, read_node_entry>
        read_node_map{{key::program, {&store::read_program, true}},
                      {key::on_start, {&store::read_on_start, true}},
                      {key::assignment, {&store::read_assignment, true}},
                      {key::print, {&store::read_print_statement, true}},
                      {key::if_else, {&store::read_if_else_statement, true}},
                      {key::placeholder, {&store::read_placeholder, false}},
                      {key::identifier, {&store::read_identifier, false}},
                      {key::unary, {&store::read_unary_expression, false}},
                      {key::binary, {&store::read_binary_expression, false}},
                      {key::number, {&store::read_number_literal, false}},
                      {key::string, {&store::read_string_literal, false}}};

    auto key{read_zero_terminated(iter, end)};
    const auto it{read_node_map.find(std::move(key))};
    if (it == read_node_map.end()) {
      throw read_error{"Unknown node key encountered!"};
    } else {
      const auto& entry{it->second};
      if (entry.is_statement) {
        emit_indent();
      }
      auto start{_current_loc};
      auto node{(this->*entry.callable)(iter, end, type, paren_precedence)};
      node->source_code_range = {start, _current_loc};
      if (entry.is_statement) {
        emit_new_line();
      }
      return node;
    }
  }

  ast::node read_program(const char*& iter, const char* end,
                         type_expectation type, size_t paren_precedence) {
    assert_type<type_expectation::program>(type, "Unexpected program!");

    auto blocks{read_vector(iter, end, type_expectation::block)};
    return ast::make<ast::program>(std::move(blocks));
  }

  ast::node read_on_start(const char*& iter, const char* end,
                          type_expectation type, size_t paren_precedence) {
    assert_type<type_expectation::block>(type, "Unexpected block!");

    emit_to_buffer("on_start {");
    emit_new_line();
    _indent++;
    auto statements{read_vector(iter, end, type_expectation::statement)};
    _indent--;
    emit_indent();
    emit_to_buffer("}");
    return ast::make<ast::on_start>(std::move(statements));
  }

  ast::node read_assignment(const char*& iter, const char* end,
                            type_expectation type, size_t paren_precedence) {
    assert_type<type_expectation::statement>(type, "Unexpected statement!");

    auto variable{read_node(iter, end, type_expectation::lvalue)};
    emit_to_buffer(" = ");
    auto value{read_node(iter, end, type_expectation::rvalue)};
    emit_to_buffer(";");
    return ast::make<ast::assignment>(std::move(variable), std::move(value));
  }

  ast::node read_print_statement(const char*& iter, const char* end,
                                 type_expectation type,
                                 size_t paren_precedence) {
    assert_type<type_expectation::statement>(type, "Unexpected statement!");

    emit_to_buffer("print(");
    auto value{read_node(iter, end, type_expectation::rvalue)};
    emit_to_buffer(");");
    return ast::make<ast::print_statement>(std::move(value));
  }

  ast::node read_if_else_statement(const char*& iter, const char* end,
                                   type_expectation type,
                                   size_t paren_precedence) {
    assert_type<type_expectation::statement>(type, "Unexpected statement!");

    const bool has_else{read_bool(iter, end)};
    emit_highlight("if", highlight_token_type::keyword);
    emit_to_buffer(" (");
    auto condition{read_node(iter, end, type_expectation::rvalue)};
    emit_to_buffer(") {");
    emit_new_line();
    _indent++;
    auto consequence{read_vector(iter, end, type_expectation::statement)};
    _indent--;
    emit_indent();
    emit_to_buffer("}");
    if (has_else) {
      emit_to_buffer(" ");
      const auto else_loc{_current_loc};
      emit_highlight("else", highlight_token_type::keyword);
      emit_to_buffer(" {");
      emit_new_line();
      _indent++;
      auto alternate{read_vector(iter, end, type_expectation::statement)};
      _indent--;
      emit_indent();
      emit_to_buffer("}");
      auto node{ast::make<ast::if_else_statement>(
          std::move(condition), std::move(consequence), std::move(alternate))};
      node->as<ast::if_else_statement>().else_loc = else_loc;
      return node;
    } else {
      return ast::make<ast::if_statement>(std::move(condition),
                                          std::move(consequence));
    }
  }

  ast::node read_placeholder(const char*& iter, const char* end,
                             type_expectation type, size_t paren_precedence) {
    assert_type<type_expectation::lvalue, type_expectation::rvalue>(
        type, "Unexpected placeholder!");

    auto string{read_string(iter, end)};
    emit_placeholder(string);
    if (type == type_expectation::lvalue) {
      return ast::make<ast::variable_placeholder>(
          std::string{std::move(string)});
    } else {
      return ast::make<ast::expression_placeholder>(
          std::string{std::move(string)});
    }
  }

  ast::node read_identifier(const char*& iter, const char* end,
                            type_expectation type, size_t paren_precedence) {
    assert_type<type_expectation::lvalue, type_expectation::rvalue>(
        type, "Unexpected identifier!");

    auto string{read_string(iter, end)};
    emit_to_buffer(string);
    if (type == type_expectation::lvalue) {
      return ast::make<ast::variable_name>(std::string{std::move(string)});
    } else {
      return ast::make<ast::identifier>(std::string{std::move(string)});
    }
  }

  ast::node read_unary_expression(const char*& iter, const char* end,
                                  type_expectation type,
                                  size_t paren_precedence) {
    assert_type<type_expectation::rvalue>(type, "Unexpected expression!");

    static const auto unary_inverse_op_symbol_map{[]() {
      std::unordered_map<std::string_view, ast::unary_op> map;
      for (uint8_t i{0}; i < ast::unary_op_symbol_map.size(); i++) {
        map[ast::unary_op_symbol_map[i]] = static_cast<ast::unary_op>(i);
      }
      return map;
    }()};

    auto op_symbol{read_zero_terminated(iter, end)};
    auto it{unary_inverse_op_symbol_map.find(op_symbol)};
    if (it == unary_inverse_op_symbol_map.end()) {
      throw read_error{"Unknown unary operator encountered!"};
    } else {
      const auto op{it->second};
      const size_t op_precedence{ast::unary_op_precedence};

      if (op_precedence <= paren_precedence) {
        emit_to_buffer("(");
      }
      emit_highlight(op_symbol, highlight_token_type::op);
      auto argument{read_node(iter, end, type_expectation::rvalue,
                              ast::unary_op_precedence)};
      if (op_precedence <= paren_precedence) {
        emit_to_buffer(")");
      }
      return ast::make<ast::unary_expression>(op, std::move(argument));
    }
  }

  ast::node read_binary_expression(const char*& iter, const char* end,
                                   type_expectation type,
                                   size_t paren_precedence) {
    assert_type<type_expectation::rvalue>(type, "Unexpected expression!");

    static const auto binary_inverse_op_symbol_map{[]() {
      std::unordered_map<std::string_view, ast::binary_op> map;
      for (uint8_t i{0}; i < ast::binary_op_symbol_map.size(); i++) {
        map[ast::binary_op_symbol_map[i]] = static_cast<ast::binary_op>(i);
      }
      return map;
    }()};

    auto op_symbol{read_zero_terminated(iter, end)};
    auto it{binary_inverse_op_symbol_map.find(op_symbol)};
    if (it == binary_inverse_op_symbol_map.end()) {
      throw read_error{"Unknown binary operator encountered!"};
    } else {
      const auto op{it->second};
      const size_t op_precedence{ast::precedence_for(op)};

      if (op_precedence <= paren_precedence) {
        emit_to_buffer("(");
      }
      auto left{
          read_node(iter, end, type_expectation::rvalue, op_precedence - 1)};
      emit_to_buffer(" ");
      emit_highlight(op_symbol, highlight_token_type::op);
      emit_to_buffer(" ");
      auto right{read_node(iter, end, type_expectation::rvalue, op_precedence)};
      if (op_precedence <= paren_precedence) {
        emit_to_buffer(")");
      }
      return ast::make<ast::binary_expression>(std::move(left), op,
                                               std::move(right));
    }
  }

  ast::node read_number_literal(const char*& iter, const char* end,
                                type_expectation type,
                                size_t paren_precedence) {
    assert_type<type_expectation::rvalue>(type, "Unexpected literal!");

    auto string{read_string(iter, end)};
    emit_highlight(string, highlight_token_type::number);
    return ast::make<ast::number_literal>(std::string{std::move(string)});
  }

  ast::node read_string_literal(const char*& iter, const char* end,
                                type_expectation type,
                                size_t paren_precedence) {
    assert_type<type_expectation::rvalue>(type, "Unexpected literal!");

    auto string{read_string(iter, end)};
    emit_highlight(quoted(std::string{string}), highlight_token_type::string);
    return ast::make<ast::string_literal>(std::string{std::move(string)});
  }

  void write_key(std::string_view key) {
    _buffer.append(key);
    _buffer.append("\0", 1);
  }

  void write_symbol(std::string_view symbol) {
    _buffer.append(symbol);
    _buffer.append("\0", 1);
  }

  void write_bool(bool value) {
    uint8_t data{value};
    _buffer.append(reinterpret_cast<char*>(&data), 1);
  }

  void write_int(uint32_t value) {
    uint8_t data[4];
    data[3] = static_cast<uint8_t>(value);
    data[2] = static_cast<uint8_t>(value >> 8);
    data[1] = static_cast<uint8_t>(value >> 16);
    data[0] = static_cast<uint8_t>(value >> 24);
    _buffer.append(reinterpret_cast<char*>(data), 4);
  }

  void write_string(const std::string& value) {
    write_int(static_cast<uint32_t>(value.size()));
    _buffer.append(value);
  }

  template <typename vector_type>
  void write_vector(const vector_type& vector) {
    write_int(static_cast<uint32_t>(vector.size()));
    for (const auto& child : vector) {
      write_base(*child);
    }
  }

  void write_base(const ast::base& node) {
    node.apply<void>([this](const auto& n) { write_node(n); });
  }

  void write_node(const ast::program& program) {
    write_key(key::program);
    write_vector(program.blocks());
  };

  void write_node(const ast::on_start& block) {
    write_key(key::on_start);
    write_vector(block.statements());
  };

  void write_node(const ast::assignment& assignment) {
    write_key(key::assignment);
    write_base(*assignment.variable());
    write_base(*assignment.value());
  };

  void write_node(const ast::print_statement& print) {
    write_key(key::print);
    write_base(*print.value());
  };

  void write_node(const ast::if_statement& statement) {
    write_key(key::if_else);
    write_bool(false);
    write_base(*statement.condition());
    write_vector(statement.statements());
  };

  void write_node(const ast::if_else_statement& statement) {
    write_key(key::if_else);
    write_bool(true);
    write_base(*statement.condition());
    write_vector(statement.consequence());
    write_vector(statement.alternate());
  }

  void write_node(const ast::variable_placeholder& placeholder) {
    write_key(key::placeholder);
    write_string(placeholder.name);
  };

  void write_node(const ast::variable_name& variable) {
    write_key(key::identifier);
    write_string(variable.name);
  };

  void write_node(const ast::expression_placeholder& placeholder) {
    write_key(key::placeholder);
    write_string(placeholder.name);
  };

  void write_node(const ast::unary_expression& unary) {
    write_key(key::unary);
    write_symbol(ast::symbol_for(unary.op));
    write_base(*unary.argument());
  };

  void write_node(const ast::binary_expression& binary) {
    write_key(key::binary);
    write_symbol(ast::symbol_for(binary.op));
    write_base(*binary.left());
    write_base(*binary.right());
  };

  void write_node(const ast::identifier& identifier) {
    write_key(key::identifier);
    write_string(identifier.name);
  };

  void write_node(const ast::number_literal& literal) {
    write_key(key::number);
    write_string(literal.value);
  };

  void write_node(const ast::string_literal& literal) {
    write_key(key::string);
    write_string(literal.value);
  };
};  // namespace v1

}  // namespace v1

using latest_store = v1::store;

}  // namespace marlin::store

#endif  // marlin_store_v1_store_hpp