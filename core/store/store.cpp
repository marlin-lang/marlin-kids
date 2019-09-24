#include "store.hpp"

#include "store_errors.hpp"

// Stores
#include "v1_store.hpp"

namespace marlin::store {

[[nodiscard]] reconstruction_result read(data_view data, size_t start_line,
                                         const ast::base& parent,
                                         user_function_table_interface& table) {
  auto* s{base_store::corresponding_store(data)};

  type_expectation type;
  if (parent.is<ast::program>()) {
    type = type_expectation::block;
  } else {
    type = type_expectation::statement;
  }

  auto nodes{s->read(std::move(data), type, table)};
  format::in_place_formatter formatter;
  auto display{formatter.format(nodes, start_line, &parent)};
  return {std::move(nodes), std::move(display)};
}

[[nodiscard]] reconstruction_result read(data_view data,
                                         const ast::base& target,
                                         user_function_table_interface& table) {
  auto* s{base_store::corresponding_store(data)};

  type_expectation type;
  if (target.is<ast::function_signature>() ||
      target.is<ast::function_placeholder>()) {
    type = type_expectation::function_signature;
  } else if (target.inherits<ast::lvalue>() ||
             target.is<ast::variable_placeholder>()) {
    type = type_expectation::lvalue;
  } else if (target.is<ast::parameter>()) {
    type = type_expectation::parameter;
  } else {
    type = type_expectation::rvalue;
  }

  auto nodes{s->read(std::move(data), type, table)};
  format::in_place_formatter formatter;
  auto display{formatter.format(nodes, target)};
  return {std::move(nodes), std::move(display)};
}

[[nodiscard]] reconstruction_result read(data_view data,
                                         user_function_table_interface& table,
                                         type_expectation type) {
  auto* s{base_store::corresponding_store(data)};

  auto nodes{s->read(std::move(data), type, table)};
  format::in_place_formatter formatter;
  auto display{formatter.format(nodes)};
  return {std::move(nodes), std::move(display)};
}

[[nodiscard]] data_vector write(
    std::vector<const ast::base*> nodes,
    std::optional<std::string_view> erase_function_names) {
  return latest_store::_singleton.write(nodes, erase_function_names);
}

}  // namespace marlin::store
