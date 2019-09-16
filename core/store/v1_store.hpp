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

inline const std::string_view eval_statement{"eval"};
inline const std::string_view assignment{"assign"};
inline const std::string_view use_global{"global"};

inline const std::string_view system_procedure{"sys_proc"};

inline const std::string_view if_else{"if"};
inline const std::string_view while_loop{"while"};
inline const std::string_view for_loop{"for"};

inline const std::string_view break_statement{"break"};
inline const std::string_view continue_statement{"continue"};
inline const std::string_view return_statement{"return"};

inline const std::string_view placeholder{"placeholder"};
inline const std::string_view identifier{"id"};

inline const std::string_view unary{"unary"};
inline const std::string_view binary{"binary"};

inline const std::string_view system_function{"sys_func"};
inline const std::string_view user_function{"user_func"};

inline const std::string_view number{"number"};
inline const std::string_view string{"string"};

}  // namespace key

struct store : base_store::impl<store> {
  bool recognize(data_view data) override {
    return data.size() >= data_prefix().size() &&
           std::equal(data.begin(), data.begin() + data_prefix().size(),
                      data_prefix().begin(), data_prefix().end());
  }

  std::vector<ast::node> read(data_view data, type_expectation type,
                              user_function_table_interface& table) override {
    assert(recognize(data));

    _iter = data.begin() + data_prefix().size();
    _end = data.end();
    _functions = &table;
    _unknown_calls.clear();

    auto nodes{read_vector(type)};
    if (nodes.size() == 0) {
      throw read_error{"No data is read!"};
    }

    for (auto* call : _unknown_calls) {
      if (_functions->has_function(call->name)) {
        call->func = &_functions->get_function(call->name);
      }
    }
    _unknown_calls.clear();

    return nodes;
  }

  data_vector write(std::vector<const ast::base*> nodes) {
    _data_buffer.clear();
    write_bytes(data_prefix());
    write_vector(nodes);
    return std::exchange(_data_buffer, {});
  }

 private:
  using read_node_entry = ast::node (store::*)(type_expectation);

  static data_view data_prefix() {
    static const data_vector _data{std::byte{'M'}, std::byte{'K'},
                                   std::byte{'B'}, std::byte{1}};
    return _data;
  }

  data_vector _data_buffer;

  data_view::pointer _iter;
  data_view::pointer _end;
  user_function_table_interface* _functions;
  std::vector<ast::user_function_call*> _unknown_calls;

  template <type_expectation... expect_types>
  void assert_type(type_expectation type, std::string message) {
    if (type != type_expectation::any && ((type != expect_types) && ...)) {
      throw read_error{std::move(message)};
    }
  }

  std::string_view read_zero_terminated() {
    auto begin{_iter};
    while (_iter < _end) {
      if (*_iter == std::byte{0}) {
        data_view result{begin, _iter};
        _iter++;
        return result;
      } else {
        _iter++;
      }
    }
    throw read_error{"Unterminating character sequence!"};
  }

  bool read_bool() {
    if (_iter < _end) {
      return static_cast<uint8_t>(*_iter++);
    } else {
      throw read_error{"End of file when expecting boolean!"};
    }
  }

  uint32_t read_int() {
    if (_iter + 4 <= _end) {
      uint32_t result{static_cast<uint8_t>(*_iter++)};
      for (size_t i{0}; i < 3; i++) {
        result <<= 8;
        result |= static_cast<uint8_t>(*_iter++);
      }
      return result;
    } else {
      throw read_error{"End of file when expecting integer!"};
    }
  }

  std::string_view read_string() {
    auto length{read_int()};
    if (_iter + length <= _end) {
      data_view result{_iter, length};
      _iter += length;
      return result;
    } else {
      throw read_error{"End of file when expecting string!"};
    }
  }

  std::vector<ast::node> read_vector(type_expectation type) {
    auto length{read_int()};
    std::vector<ast::node> result;
    result.reserve(length);
    for (size_t i{0}; i < length; i++) {
      result.emplace_back(read_node(type));
    }
    return result;
  }

  ast::node read_node(type_expectation type) {
    static const std::unordered_map<std::string_view, read_node_entry>
        read_node_map{
            {key::program, &store::read_program},
            {key::on_start, &store::read_on_start},
            {key::function, &store::read_function},
            {key::function_signature, &store::read_function_signature},
            {key::eval_statement, &store::read_eval},
            {key::assignment, &store::read_assignment},
            {key::use_global, &store::read_use_global},
            {key::system_procedure, &store::read_system_procedure},
            {key::if_else, &store::read_if_else},
            {key::while_loop, &store::read_while},
            {key::for_loop, &store::read_for},
            {key::break_statement, &store::read_break},
            {key::continue_statement, &store::read_continue},
            {key::return_statement, &store::read_return},
            {key::placeholder, &store::read_placeholder},
            {key::identifier, &store::read_identifier},
            {key::unary, &store::read_unary_expression},
            {key::binary, &store::read_binary_expression},
            {key::system_function, &store::read_system_function},
            {key::user_function, &store::read_user_function},
            {key::number, &store::read_number_literal},
            {key::string, &store::read_string_literal}};

    const auto it{read_node_map.find(read_zero_terminated())};
    if (it == read_node_map.end()) {
      throw read_error{"Unknown node key encountered!"};
    } else {
      auto node{(this->*(it->second))(type)};
      return node;
    }
  }

  ast::node read_program(type_expectation type) {
    assert_type<type_expectation::program>(type, "Unexpected program!");

    auto blocks{read_vector(type_expectation::block)};
    return ast::make<ast::program>(std::move(blocks));
  }

  ast::node read_on_start(type_expectation type) {
    assert_type<type_expectation::block>(type, "Unexpected block!");
    auto statements{read_vector(type_expectation::statement)};
    return ast::make<ast::on_start>(std::move(statements));
  }

  ast::node read_function_signature(type_expectation type) {
    assert_type<type_expectation::function_signature>(type,
                                                      "Unexpected function!");

    std::string name{read_string()};
    auto params{read_vector(type_expectation::lvalue)};

    if (_functions->has_function(name)) {
      throw read_error{"Repeated function name encountered!"};
    } else {
      std::vector<std::string> param_names;
      for (auto& param : params) {
        if (param->is<ast::variable_name>()) {
          param_names.emplace_back(param->as<ast::variable_name>().name);
        } else {
          throw read_error{"Unexpected node, expecting function parameter!"};
        }
      }
      _functions->add_function({name, std::move(param_names)});
      return ast::make<ast::function_signature>(std::string{std::move(name)},
                                                std::move(params));
    }
  }

  ast::node read_function(type_expectation type) {
    assert_type<type_expectation::block>(type, "Unexpected function!");

    auto signature{read_node(type_expectation::function_signature)};
    auto statements{read_vector(type_expectation::statement)};
    return ast::make<ast::function>(std::move(signature),
                                    std::move(statements));
  }

  ast::node read_eval(type_expectation type) {
    assert_type<type_expectation::statement>(type, "Unexpected statement!");

    auto expression{read_node(type_expectation::rvalue)};
    return ast::make<ast::eval_statement>(std::move(expression));
  }

  ast::node read_assignment(type_expectation type) {
    assert_type<type_expectation::statement>(type, "Unexpected statement!");

    auto variable{read_node(type_expectation::lvalue)};
    auto value{read_node(type_expectation::rvalue)};
    return ast::make<ast::assignment>(std::move(variable), std::move(value));
  }

  ast::node read_use_global(type_expectation type) {
    assert_type<type_expectation::statement>(type, "Unexpected statement!");

    auto variable{read_node(type_expectation::lvalue)};
    return ast::make<ast::use_global>(std::move(variable));
  }

  ast::node read_system_procedure(type_expectation type) {
    assert_type<type_expectation::statement>(type, "Unexpected statement!");

    static const auto system_procedure_inverse_name_map{[]() {
      std::unordered_map<std::string_view, ast::system_procedure> map;
      for (size_t i{0}; i < ast::system_procedure_name_map.size(); i++) {
        map[ast::system_procedure_name_map[i]] = {i};
      }
      return map;
    }()};

    auto name{read_zero_terminated()};
    auto it{system_procedure_inverse_name_map.find(name)};
    if (it == system_procedure_inverse_name_map.end()) {
      throw read_error{"Unknown system function encountered!"};
    } else {
      const auto proc{it->second};
      auto args{read_vector(type_expectation::rvalue)};
      return ast::make<ast::system_procedure_call>(proc, std::move(args));
    }
  }

  ast::node read_if_else(type_expectation type) {
    assert_type<type_expectation::statement>(type, "Unexpected statement!");

    const bool has_else{read_bool()};
    auto condition{read_node(type_expectation::rvalue)};
    auto consequence{read_vector(type_expectation::statement)};
    if (has_else) {
      auto alternate{read_vector(type_expectation::statement)};
      return ast::make<ast::if_else_statement>(
          std::move(condition), std::move(consequence), std::move(alternate));
    } else {
      return ast::make<ast::if_statement>(std::move(condition),
                                          std::move(consequence));
    }
  }

  ast::node read_while(type_expectation type) {
    assert_type<type_expectation::statement>(type, "Unexpected statement!");

    auto condition{read_node(type_expectation::rvalue)};
    auto statements{read_vector(type_expectation::statement)};
    return ast::make<ast::while_statement>(std::move(condition),
                                           std::move(statements));
  }

  ast::node read_for(type_expectation type) {
    assert_type<type_expectation::statement>(type, "Unexpected statement!");

    auto variable{read_node(type_expectation::lvalue)};
    auto list{read_node(type_expectation::rvalue)};
    auto statements{read_vector(type_expectation::statement)};
    return ast::make<ast::for_statement>(std::move(variable), std::move(list),
                                         std::move(statements));
  }

  ast::node read_break(type_expectation type) {
    assert_type<type_expectation::statement>(type, "Unexpected statement!");
    return ast::make<ast::break_statement>();
  }

  ast::node read_continue(type_expectation type) {
    assert_type<type_expectation::statement>(type, "Unexpected statement!");
    return ast::make<ast::continue_statement>();
  }

  ast::node read_return(type_expectation type) {
    assert_type<type_expectation::statement>(type, "Unexpected statement!");

    const bool has_result{read_bool()};
    if (has_result) {
      auto result{read_node(type_expectation::rvalue)};
      return ast::make<ast::return_result_statement>(std::move(result));
    } else {
      return ast::make<ast::return_statement>();
    }
  }

  ast::node read_placeholder(type_expectation type) {
    assert_type<type_expectation::lvalue, type_expectation::function_signature,
                type_expectation::rvalue>(type, "Unexpected placeholder!");

    auto string{read_string()};
    if (type == type_expectation::lvalue) {
      return ast::make<ast::variable_placeholder>(
          std::string{std::move(string)});
    } else if (type == type_expectation::function_signature) {
      return ast::make<ast::function_placeholder>(
          std::string{std::move(string)});
    } else {
      return ast::make<ast::expression_placeholder>(
          std::string{std::move(string)});
    }
  }

  ast::node read_identifier(type_expectation type) {
    assert_type<type_expectation::lvalue, type_expectation::rvalue>(
        type, "Unexpected identifier!");

    auto string{read_string()};
    if (type == type_expectation::lvalue) {
      return ast::make<ast::variable_name>(std::string{std::move(string)});
    } else {
      return ast::make<ast::identifier>(std::string{std::move(string)});
    }
  }

  ast::node read_unary_expression(type_expectation type) {
    assert_type<type_expectation::rvalue>(type, "Unexpected expression!");

    static const auto unary_inverse_op_symbol_map{[]() {
      std::unordered_map<std::string_view, ast::unary_op> map;
      for (uint8_t i{0}; i < ast::unary_op_symbol_map.size(); i++) {
        map[ast::unary_op_symbol_map[i]] = {i};
      }
      return map;
    }()};

    auto op_symbol{read_zero_terminated()};
    auto it{unary_inverse_op_symbol_map.find(op_symbol)};
    if (it == unary_inverse_op_symbol_map.end()) {
      throw read_error{"Unknown unary operator encountered!"};
    } else {
      const auto op{it->second};
      auto argument{read_node(type_expectation::rvalue)};
      return ast::make<ast::unary_expression>(op, std::move(argument));
    }
  }

  ast::node read_binary_expression(type_expectation type) {
    assert_type<type_expectation::rvalue>(type, "Unexpected expression!");

    static const auto binary_inverse_op_symbol_map{[]() {
      std::unordered_map<std::string_view, ast::binary_op> map;
      for (uint8_t i{0}; i < ast::binary_op_symbol_map.size(); i++) {
        map[ast::binary_op_symbol_map[i]] = {i};
      }
      return map;
    }()};

    auto op_symbol{read_zero_terminated()};
    auto it{binary_inverse_op_symbol_map.find(op_symbol)};
    if (it == binary_inverse_op_symbol_map.end()) {
      throw read_error{"Unknown binary operator encountered!"};
    } else {
      const auto op{it->second};
      auto left{read_node(type_expectation::rvalue)};
      auto right{read_node(type_expectation::rvalue)};
      return ast::make<ast::binary_expression>(std::move(left), op,
                                               std::move(right));
    }
  }

  ast::node read_system_function(type_expectation type) {
    assert_type<type_expectation::rvalue>(type, "Unexpected expression!");

    static const auto system_function_inverse_name_map{[]() {
      std::unordered_map<std::string_view, ast::system_function> map;
      for (size_t i{0}; i < ast::system_function_name_map.size(); i++) {
        map[ast::system_function_name_map[i]] = {i};
      }
      return map;
    }()};

    auto name{read_zero_terminated()};
    auto it{system_function_inverse_name_map.find(name)};
    if (it == system_function_inverse_name_map.end()) {
      throw read_error{"Unknown system function encountered!"};
    } else {
      const auto func{it->second};
      auto args{read_vector(type_expectation::rvalue)};
      return ast::make<ast::system_function_call>(func, std::move(args));
    }
  }

  ast::node read_user_function(type_expectation type) {
    assert_type<type_expectation::rvalue>(type, "Unexpected expression!");

    std::string name{read_string()};
    auto args{read_vector(type_expectation::rvalue)};
    if (_functions->has_function(name)) {
      auto* definition{&_functions->get_function(name)};
      return ast::make<ast::user_function_call>(std::move(name), definition,
                                                std::move(args));
    } else {
      auto node{ast::make<ast::user_function_call>(std::move(name), nullptr,
                                                   std::move(args))};
      _unknown_calls.emplace_back(&node->as<ast::user_function_call>());
      return node;
    }
  }

  ast::node read_number_literal(type_expectation type) {
    assert_type<type_expectation::rvalue>(type, "Unexpected literal!");

    return ast::make<ast::number_literal>(std::string{read_string()});
  }

  ast::node read_string_literal(type_expectation type) {
    assert_type<type_expectation::rvalue>(type, "Unexpected literal!");

    return ast::make<ast::string_literal>(std::string{read_string()});
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

  void write_node(const ast::eval_statement& eval) {
    write_key(key::eval_statement);
    write_base(*eval.expression());
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

  void write_node(const ast::return_statement&) {
    write_key(key::return_statement);
    write_bool(false);
  }

  void write_node(const ast::return_result_statement& statement) {
    write_key(key::return_statement);
    write_bool(true);
    write_base(*statement.result());
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

  void write_node(const ast::user_function_call& call) {
    write_key(key::user_function);
    write_string(call.name);
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
