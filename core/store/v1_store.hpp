#ifndef marlin_store_v1_store_hpp
#define marlin_store_v1_store_hpp

#include <algorithm>
#include <type_traits>
#include <unordered_map>

#include "base.hpp"
#include "specs.hpp"
#include "store_definition.hpp"
#include "store_errors.hpp"

// For now we are supporting print statement for back capability
// Will remove later

namespace marlin::store {

namespace v1 {

namespace key {

inline const std::string_view program{"program"};
inline const std::string_view on_start{"on_start"};
inline const std::string_view function{"function"};
inline const std::string_view function_signature{"signature"};

inline const std::string_view assignment{"assign"};
inline const std::string_view use_global{"global"};
inline const std::string_view system_procedure{"sys_proc"};

inline const std::string_view if_else{"if"};
inline const std::string_view while_loop{"while"};
inline const std::string_view for_loop{"for"};

inline const std::string_view break_statement{"break"};
inline const std::string_view continue_statement{"continue"};

inline const std::string_view placeholder{"placeholder"};
inline const std::string_view identifier{"id"};

inline const std::string_view unary{"unary"};
inline const std::string_view binary{"binary"};

inline const std::string_view system_function{"sys_func"};

inline const std::string_view number{"number"};
inline const std::string_view string{"string"};

}  // namespace key

struct store : base_store::impl<store> {
  bool recognize(data_view data) override {
    return data.size() >= data_prefix().size() &&
           std::equal(data.begin(), data.begin() + data_prefix().size(),
                      data_prefix().begin(), data_prefix().end());
  }

  reconstruction_result read(data_view data, source_loc start, size_t indent,
                             type_expectation type,
                             size_t paren_precedence) override {
    assert(recognize(data));

    _source_buffer.clear();
    _highlights.clear();
    _current_loc = start;
    _indent = indent;
    auto current{data.begin() + data_prefix().size()};
    auto nodes{read_vector(current, data.end(), type, paren_precedence)};
    if (nodes.size() == 0) {
      throw read_error{"No data is read!"};
    }

    reconstruction_result result{std::move(nodes), std::move(_source_buffer),
                                 std::move(_highlights)};
    _source_buffer = {};
    _highlights = {};
    return result;
  }

  data_vector write(std::vector<const ast::base*> nodes) {
    _data_buffer.clear();
    write_bytes(data_prefix());
    write_vector(nodes);
    auto result{std::move(_data_buffer)};
    _source_buffer = {};
    return result;
  }

 private:
  struct read_node_entry {
    using callable_type = ast::node (store::*)(data_view::pointer&,
                                               data_view::pointer,
                                               type_expectation, size_t);

    callable_type callable;
    bool is_statement;

    read_node_entry(callable_type _callable, bool _is_statement)
        : callable{_callable}, is_statement{_is_statement} {}
  };

  static data_view data_prefix() {
    static const data_vector _data{std::byte{'M'}, std::byte{'K'},
                                   std::byte{'B'}, std::byte{1}};
    return _data;
  }

  data_vector _data_buffer;

  std::string _source_buffer;
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
    _source_buffer.append(string);

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
    _highlights.emplace_back(highlight_token_type::placeholder,
                             _source_buffer.size(), name.size() + 1);
    emit_to_buffer("@");
    emit_to_buffer(std::move(name));
  }

  void emit_highlight(std::string_view string, highlight_token_type type) {
    _highlights.emplace_back(type, _source_buffer.size(), string.size());
    emit_to_buffer(std::move(string));
  }

  std::string_view read_zero_terminated(data_view::pointer& iter,
                                        data_view::pointer end) {
    auto begin{iter};
    while (iter < end) {
      if (*iter == std::byte{0}) {
        data_view result{begin, iter};
        iter++;
        return result;
      } else {
        iter++;
      }
    }
    throw read_error{"Unterminating character sequence!"};
  }

  bool read_bool(data_view::pointer& iter, data_view::pointer end) {
    if (iter < end) {
      return static_cast<uint8_t>(*iter++);
    } else {
      throw read_error{"End of file when expecting boolean!"};
    }
  }

  uint32_t read_int(data_view::pointer& iter, data_view::pointer end) {
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

  std::string_view read_string(data_view::pointer& iter,
                               data_view::pointer end) {
    auto length{read_int(iter, end)};
    if (iter + length <= end) {
      data_view result{iter, length};
      iter += length;
      return result;
    } else {
      throw read_error{"End of file when expecting string!"};
    }
  }

  std::vector<ast::node> read_vector(data_view::pointer& iter,
                                     data_view::pointer end,
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

  std::vector<ast::node> read_arguments(data_view::pointer& iter,
                                        data_view::pointer end,
                                        type_expectation type) {
    emit_to_buffer("(");
    auto length{read_int(iter, end)};
    std::vector<ast::node> args;
    args.reserve(length);
    for (size_t i{0}; i < length; i++) {
      if (i > 0) {
        emit_to_buffer(", ");
      }
      args.emplace_back(read_node(iter, end, type));
    }
    emit_to_buffer(")");
    return args;
  }

  ast::node read_node(data_view::pointer& iter, data_view::pointer end,
                      type_expectation type, size_t paren_precedence = 0) {
    static const std::unordered_map<std::string_view, read_node_entry>
        read_node_map{
            {key::program, {&store::read_program, true}},
            {key::on_start, {&store::read_on_start, true}},
            {key::function, {&store::read_function, true}},
            {key::function_signature, {&store::read_function_signature, true}},
            {key::assignment, {&store::read_assignment, true}},
            {key::use_global, {&store::read_use_global, true}},
            {key::system_procedure, {&store::read_system_procedure, true}},
            {key::if_else, {&store::read_if_else_statement, true}},
            {key::while_loop, {&store::read_while_statement, true}},
            {key::for_loop, {&store::read_for_statement, true}},
            {key::break_statement, {&store::read_break, true}},
            {key::continue_statement, {&store::read_continue, true}},
            {key::placeholder, {&store::read_placeholder, false}},
            {key::identifier, {&store::read_identifier, false}},
            {key::unary, {&store::read_unary_expression, false}},
            {key::binary, {&store::read_binary_expression, false}},
            {key::system_function, {&store::read_system_function, false}},
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

  ast::node read_program(data_view::pointer& iter, data_view::pointer end,
                         type_expectation type, size_t paren_precedence) {
    assert_type<type_expectation::program>(type, "Unexpected program!");

    auto blocks{read_vector(iter, end, type_expectation::block)};
    return ast::make<ast::program>(std::move(blocks));
  }

  ast::node read_on_start(data_view::pointer& iter, data_view::pointer end,
                          type_expectation type, size_t paren_precedence) {
    assert_type<type_expectation::block>(type, "Unexpected block!");

    emit_highlight("on start", highlight_token_type::keyword);
    emit_to_buffer(" {");
    emit_new_line();
    _indent++;
    auto statements{read_vector(iter, end, type_expectation::statement)};
    _indent--;
    emit_indent();
    emit_to_buffer("}");
    return ast::make<ast::on_start>(std::move(statements));
  }

  ast::node read_function_signature(data_view::pointer& iter,
                                    data_view::pointer end,
                                    type_expectation type,
                                    size_t paren_precedence) {
    assert_type<type_expectation::function_signature>(type,
                                                      "Unexpected function!");

    auto name{read_string(iter, end)};
    emit_to_buffer(name);

    auto args{read_arguments(iter, end, type_expectation::lvalue)};
    return ast::make<ast::function_signature>(std::string{std::move(name)},
                                              std::move(args));
  }

  ast::node read_function(data_view::pointer& iter, data_view::pointer end,
                          type_expectation type, size_t paren_precedence) {
    assert_type<type_expectation::block>(type, "Unexpected function!");

    emit_highlight("function", highlight_token_type::keyword);
    emit_to_buffer(" ");
    auto signature{read_node(iter, end, type_expectation::function_signature)};
    emit_to_buffer(" {");
    emit_new_line();
    _indent++;
    auto statements{read_vector(iter, end, type_expectation::statement)};
    _indent--;
    emit_indent();
    emit_to_buffer("}");
    return ast::make<ast::function>(std::move(signature),
                                    std::move(statements));
  }

  ast::node read_assignment(data_view::pointer& iter, data_view::pointer end,
                            type_expectation type, size_t paren_precedence) {
    assert_type<type_expectation::statement>(type, "Unexpected statement!");

    auto variable{read_node(iter, end, type_expectation::lvalue)};
    emit_to_buffer(" = ");
    auto value{read_node(iter, end, type_expectation::rvalue)};
    emit_to_buffer(";");
    return ast::make<ast::assignment>(std::move(variable), std::move(value));
  }

  ast::node read_use_global(data_view::pointer& iter, data_view::pointer end,
                            type_expectation type, size_t paren_precedence) {
    assert_type<type_expectation::statement>(type, "Unexpected statement!");

    emit_highlight("use global", highlight_token_type::keyword);
    emit_to_buffer(" ");
    auto variable{read_node(iter, end, type_expectation::lvalue)};
    emit_to_buffer(";");
    return ast::make<ast::use_global>(std::move(variable));
  }

  ast::node read_system_procedure(data_view::pointer& iter,
                                  data_view::pointer end, type_expectation type,
                                  size_t paren_precedence) {
    assert_type<type_expectation::statement>(type, "Unexpected statement!");

    static const auto system_procedure_inverse_name_map{[]() {
      std::unordered_map<std::string_view, ast::system_procedure> map;
      for (size_t i{0}; i < ast::system_procedure_name_map.size(); i++) {
        map[ast::system_procedure_name_map[i]] = {i};
      }
      return map;
    }()};
    auto name{read_zero_terminated(iter, end)};
    auto it{system_procedure_inverse_name_map.find(name)};
    if (it == system_procedure_inverse_name_map.end()) {
      throw read_error{"Unknown system function encountered!"};
    } else {
      const auto proc{it->second};
      emit_to_buffer(display_for(proc));

      auto args{read_arguments(iter, end, type_expectation::rvalue)};
      emit_to_buffer(";");

      return ast::make<ast::system_procedure_call>(proc, std::move(args));
    }
  }

  ast::node read_if_else_statement(data_view::pointer& iter,
                                   data_view::pointer end,
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

  ast::node read_while_statement(data_view::pointer& iter,
                                 data_view::pointer end, type_expectation type,
                                 size_t paren_precedence) {
    assert_type<type_expectation::statement>(type, "Unexpected statement!");

    emit_highlight("while", highlight_token_type::keyword);
    emit_to_buffer(" (");
    auto condition{read_node(iter, end, type_expectation::rvalue)};
    emit_to_buffer(") {");
    emit_new_line();
    _indent++;
    auto statements{read_vector(iter, end, type_expectation::statement)};
    _indent--;
    emit_indent();
    emit_to_buffer("}");
    return ast::make<ast::while_statement>(std::move(condition),
                                           std::move(statements));
  }

  ast::node read_for_statement(data_view::pointer& iter, data_view::pointer end,
                               type_expectation type, size_t paren_precedence) {
    assert_type<type_expectation::statement>(type, "Unexpected statement!");

    emit_highlight("for", highlight_token_type::keyword);
    emit_to_buffer(" (");
    auto variable{read_node(iter, end, type_expectation::lvalue)};
    emit_to_buffer(" in ");
    auto list{read_node(iter, end, type_expectation::rvalue)};
    emit_to_buffer(") {");
    emit_new_line();
    _indent++;
    auto statements{read_vector(iter, end, type_expectation::statement)};
    _indent--;
    emit_indent();
    emit_to_buffer("}");
    return ast::make<ast::for_statement>(std::move(variable), std::move(list),
                                         std::move(statements));
  }

  ast::node read_break(data_view::pointer& iter, data_view::pointer end,
                       type_expectation type, size_t paren_precedence) {
    assert_type<type_expectation::statement>(type, "Unexpected statement!");

    emit_highlight("break", highlight_token_type::keyword);
    emit_to_buffer(";");
    return ast::make<ast::break_statement>();
  }

  ast::node read_continue(data_view::pointer& iter, data_view::pointer end,
                          type_expectation type, size_t paren_precedence) {
    assert_type<type_expectation::statement>(type, "Unexpected statement!");

    emit_highlight("continue", highlight_token_type::keyword);
    emit_to_buffer(";");
    return ast::make<ast::continue_statement>();
  }

  ast::node read_placeholder(data_view::pointer& iter, data_view::pointer end,
                             type_expectation type, size_t paren_precedence) {
    assert_type<type_expectation::lvalue, type_expectation::function_signature,
                type_expectation::rvalue>(type, "Unexpected placeholder!");

    auto string{read_string(iter, end)};
    emit_placeholder(string);
    if (type == type_expectation::lvalue) {
      return ast::make<ast::variable_placeholder>(
          std::string{std::move(string)});
    } else if (type == type_expectation::function_signature) {
      emit_to_buffer("()");
      return ast::make<ast::function_placeholder>(
          std::string{std::move(string)});
    } else {
      return ast::make<ast::expression_placeholder>(
          std::string{std::move(string)});
    }
  }

  ast::node read_identifier(data_view::pointer& iter, data_view::pointer end,
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

  ast::node read_unary_expression(data_view::pointer& iter,
                                  data_view::pointer end, type_expectation type,
                                  size_t paren_precedence) {
    assert_type<type_expectation::rvalue>(type, "Unexpected expression!");

    static const auto unary_inverse_op_symbol_map{[]() {
      std::unordered_map<std::string_view, ast::unary_op> map;
      for (uint8_t i{0}; i < ast::unary_op_symbol_map.size(); i++) {
        map[ast::unary_op_symbol_map[i]] = {i};
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

  ast::node read_binary_expression(data_view::pointer& iter,
                                   data_view::pointer end,
                                   type_expectation type,
                                   size_t paren_precedence) {
    assert_type<type_expectation::rvalue>(type, "Unexpected expression!");

    static const auto binary_inverse_op_symbol_map{[]() {
      std::unordered_map<std::string_view, ast::binary_op> map;
      for (uint8_t i{0}; i < ast::binary_op_symbol_map.size(); i++) {
        map[ast::binary_op_symbol_map[i]] = {i};
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
      auto highlight = highlight_token_type::op;
      if (op == ast::binary_op::logical_and ||
          op == ast::binary_op::logical_or) {
        highlight = highlight_token_type::keyword;
      }
      emit_highlight(op_symbol, highlight);
      emit_to_buffer(" ");
      auto right{read_node(iter, end, type_expectation::rvalue, op_precedence)};
      if (op_precedence <= paren_precedence) {
        emit_to_buffer(")");
      }
      return ast::make<ast::binary_expression>(std::move(left), op,
                                               std::move(right));
    }
  }

  ast::node read_system_function(data_view::pointer& iter,
                                 data_view::pointer end, type_expectation type,
                                 size_t paren_precedence) {
    assert_type<type_expectation::rvalue>(type, "Unexpected expression!");

    static const auto system_function_inverse_name_map{[]() {
      std::unordered_map<std::string_view, ast::system_function> map;
      for (size_t i{0}; i < ast::system_function_name_map.size(); i++) {
        map[ast::system_function_name_map[i]] = {i};
      }
      return map;
    }()};

    auto name{read_zero_terminated(iter, end)};
    auto it{system_function_inverse_name_map.find(name)};
    if (it == system_function_inverse_name_map.end()) {
      throw read_error{"Unknown system function encountered!"};
    } else {
      const auto func{it->second};
      emit_to_buffer(display_for(func));

      auto args{read_arguments(iter, end, type_expectation::rvalue)};
      return ast::make<ast::system_function_call>(func, std::move(args));
    }
  }

  ast::node read_number_literal(data_view::pointer& iter,
                                data_view::pointer end, type_expectation type,
                                size_t paren_precedence) {
    assert_type<type_expectation::rvalue>(type, "Unexpected literal!");

    auto string{read_string(iter, end)};
    emit_highlight(string, highlight_token_type::number);
    return ast::make<ast::number_literal>(std::string{std::move(string)});
  }

  ast::node read_string_literal(data_view::pointer& iter,
                                data_view::pointer end, type_expectation type,
                                size_t paren_precedence) {
    assert_type<type_expectation::rvalue>(type, "Unexpected literal!");

    auto string{read_string(iter, end)};
    emit_highlight(quoted(std::string{string}), highlight_token_type::string);
    return ast::make<ast::string_literal>(std::string{std::move(string)});
  }

  void write_byte(uint8_t byte) { _data_buffer.emplace_back(std::byte{byte}); }

  template <typename byte_type,
            typename = std::enable_if_t<std::is_same_v<byte_type, char> ||
                                        std::is_same_v<byte_type, uint8_t> ||
                                        std::is_same_v<byte_type, std::byte>>>
  void write_bytes(const byte_type* begin, const byte_type* end) {
    _data_buffer.insert(_data_buffer.end(),
                        reinterpret_cast<const std::byte*>(begin),
                        reinterpret_cast<const std::byte*>(end));
  }

  void write_bytes(data_view data) { write_bytes(data.begin(), data.end()); }

  void write_bytes(std::string_view string) {
    write_bytes(string.begin(), string.end());
  }

  void write_key(std::string_view key) {
    write_bytes(key.begin(), key.end());
    write_byte(0);
  }

  void write_symbol(std::string_view symbol) {
    write_bytes(symbol.begin(), symbol.end());
    write_byte(0);
  }

  void write_bool(bool value) { write_byte(value); }

  void write_int(uint32_t value) {
    uint8_t data[4];
    data[3] = static_cast<uint8_t>(value);
    data[2] = static_cast<uint8_t>(value >> 8);
    data[1] = static_cast<uint8_t>(value >> 16);
    data[0] = static_cast<uint8_t>(value >> 24);
    write_bytes(data, data + 4);
  }

  void write_string(std::string_view value) {
    write_int(static_cast<uint32_t>(value.size()));
    write_bytes(value.begin(), value.end());
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
  }

  void write_node(const ast::on_start& block) {
    write_key(key::on_start);
    write_vector(block.statements());
  }

  void write_node(const ast::function_placeholder& placeholder) {
    write_key(key::placeholder);
    write_string(placeholder.name);
  }

  void write_node(const ast::function_signature& signature) {
    write_key(key::function_signature);
    write_string(signature.name);
    write_vector(signature.parameters());
  }

  void write_node(const ast::function& function) {
    write_key(key::function);
    write_base(*function.signature());
    write_vector(function.statements());
  }

  void write_node(const ast::assignment& assignment) {
    write_key(key::assignment);
    write_base(*assignment.variable());
    write_base(*assignment.value());
  }

  void write_node(const ast::use_global& use_global) {
    write_key(key::use_global);
    write_base(*use_global.variable());
  }

  void write_node(const ast::system_procedure_call& call) {
    write_key(key::system_procedure);
    write_symbol(ast::name_for(call.proc));
    write_vector(call.arguments());
  }

  void write_node(const ast::if_statement& statement) {
    write_key(key::if_else);
    write_bool(false);
    write_base(*statement.condition());
    write_vector(statement.statements());
  }

  void write_node(const ast::if_else_statement& statement) {
    write_key(key::if_else);
    write_bool(true);
    write_base(*statement.condition());
    write_vector(statement.consequence());
    write_vector(statement.alternate());
  }

  void write_node(const ast::while_statement& statement) {
    write_key(key::while_loop);
    write_base(*statement.condition());
    write_vector(statement.statements());
  }

  void write_node(const ast::for_statement& statement) {
    write_key(key::for_loop);
    write_base(*statement.variable());
    write_base(*statement.list());
    write_vector(statement.statements());
  }

  void write_node(const ast::break_statement& statement) {
    write_key(key::break_statement);
  }

  void write_node(const ast::continue_statement& statement) {
    write_key(key::continue_statement);
  }

  void write_node(const ast::variable_placeholder& placeholder) {
    write_key(key::placeholder);
    write_string(placeholder.name);
  }

  void write_node(const ast::variable_name& variable) {
    write_key(key::identifier);
    write_string(variable.name);
  }

  void write_node(const ast::expression_placeholder& placeholder) {
    write_key(key::placeholder);
    write_string(placeholder.name);
  }

  void write_node(const ast::unary_expression& unary) {
    write_key(key::unary);
    write_symbol(ast::symbol_for(unary.op));
    write_base(*unary.argument());
  }

  void write_node(const ast::binary_expression& binary) {
    write_key(key::binary);
    write_symbol(ast::symbol_for(binary.op));
    write_base(*binary.left());
    write_base(*binary.right());
  }

  void write_node(const ast::system_function_call& call) {
    write_key(key::system_function);
    write_symbol(ast::name_for(call.func));
    write_vector(call.arguments());
  }

  void write_node(const ast::identifier& identifier) {
    write_key(key::identifier);
    write_string(identifier.name);
  }

  void write_node(const ast::number_literal& literal) {
    write_key(key::number);
    write_string(literal.value);
  }

  void write_node(const ast::string_literal& literal) {
    write_key(key::string);
    write_string(literal.value);
  }
};  // namespace v1

}  // namespace v1

using latest_store = v1::store;

}  // namespace marlin::store

#endif  // marlin_store_v1_store_hpp
