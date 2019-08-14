#ifndef marlin_store_v1_store_hpp
#define marlin_store_v1_store_hpp

#include "base.hpp"
#include "specs.hpp"
#include "store_definition.hpp"
#include "store_errors.hpp"

namespace marlin::store {

namespace v1 {

namespace key {

inline const std::string program{"program"};
inline const std::string on_start{"on_start"};

inline const std::string assignment{"assign"};
inline const std::string print{"print"};

inline const std::string if_else{"if"};

inline const std::string variable_placeholder{"var_ph"};
inline const std::string variable_name{"var"};

inline const std::string expression_placeholder{"expr_ph"};

inline const std::string unary{"unary"};
inline const std::string binary{"binary"};

inline const std::string identifier{"id"};

inline const std::string number{"number"};
inline const std::string string{"string"};

}  // namespace key

struct store : base_store::impl<store> {
  bool recognize(const std::string& data) override {
    return data.compare(0, _data_prefix.size(), data) == 0;
  }

  reconstruction_result read(const std::string& data,
                             const ast::base* parent) override {
    assert(recognize(data));

    throw read_error{"Version 1 scripts are not yet supported!"};
  }

  std::string write(std::vector<const ast::base*> nodes) {
    _buffer = _data_prefix;
    write_vector(nodes);
    return _buffer;
  }

 private:
  inline static const std::string _data_prefix{"MKB\1"};

  std::string _buffer;

  void write_key(const std::string& key) {
    _buffer.append(key);
    _buffer.append("\0");
  }

  void write_symbol(const std::string& symbol) {
    _buffer.append(symbol);
    _buffer.append("\0");
  }

  void write_bool(bool value) {
    uint8_t data{value};
    _buffer.append(reinterpret_cast<char*>(&data), 1);
  }

  void write_int(uint32_t value) {
    uint8_t data[4];
    data[0] = static_cast<uint8_t>(value);
    data[1] = static_cast<uint8_t>(value >> 8);
    data[2] = static_cast<uint8_t>(value >> 16);
    data[3] = static_cast<uint8_t>(value >> 24);
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
    write_vector(statement.statements());
  };

  void write_node(const ast::if_else_statement& statement) {
    write_key(key::if_else);
    write_bool(true);
    write_vector(statement.consequence());
    write_vector(statement.alternate());
  }

  void write_node(const ast::variable_placeholder& placeholder) {
    write_key(key::variable_placeholder);
    write_string(placeholder.name);
  };

  void write_node(const ast::variable_name& variable) {
    write_key(key::variable_name);
    write_string(variable.name);
  };

  void write_node(const ast::expression_placeholder& placeholder) {
    write_key(key::expression_placeholder);
    write_string(placeholder.name);
  };

  void write_node(const ast::unary_expression& unary) {
    write_key(key::unary);
    write_symbol(symbol_for(unary.op));
    write_base(*unary.argument());
  };

  void write_node(const ast::binary_expression& binary) {
    write_key(key::binary);
    write_base(*binary.left());
    write_symbol(symbol_for(binary.op));
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
};

}  // namespace v1

using latest_store = v1::store;

}  // namespace marlin::store

#endif  // marlin_store_v1_store_hpp