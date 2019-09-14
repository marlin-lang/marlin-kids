#include "store.hpp"

#include "store_errors.hpp"

// Stores
#include "v1_store.hpp"

namespace marlin::store {

[[nodiscard]] reconstruction_result read(data_view data,
                                         const ast::base& parent,
                                         size_t start_line) {
  auto* s{base_store::corresponding_store(data)};

  type_expectation type;
  if (parent.is<ast::program>()) {
    type = type_expectation::block;
  } else {
    type = type_expectation::statement;
  }

  size_t indent{0};
  const ast::base* curr_parent{&parent};
  while (curr_parent->has_parent()) {
    indent++;
    curr_parent = &curr_parent->parent();
  }

  return s->read(std::move(data), {start_line, 1}, indent, type);
}

[[nodiscard]] reconstruction_result read(data_view data,
                                         const ast::base& target) {
  auto* s{base_store::corresponding_store(data)};

  type_expectation type;
  if (target.is<ast::function_signature>() ||
      target.is<ast::function_placeholder>()) {
    type = type_expectation::function_signature;
  } else if (target.is<ast::variable_placeholder>() ||
             target.is<ast::variable_name>()) {
    type = type_expectation::lvalue;
  } else {
    type = type_expectation::rvalue;
  }

  size_t paren_precedence{0};
  if (target.has_parent()) {
    if (target.parent().is<ast::unary_expression>()) {
      paren_precedence = ast::unary_op_precedence;
    } else if (target.parent().is<ast::binary_expression>()) {
      auto& binary{target.parent().as<ast::binary_expression>()};
      if (&target == binary.left().get()) {
        paren_precedence = precedence_for(binary.op) - 1;
      } else {
        paren_precedence = precedence_for(binary.op);
      }
    }
  }

  return s->read(std::move(data), target.source_code_range.begin, 0, type,
                 paren_precedence);
}

[[nodiscard]] reconstruction_result read(data_view data,
                                         type_expectation type) {
  auto* s{base_store::corresponding_store(data)};
  return s->read(std::move(data), {1, 1}, 0, type);
}

[[nodiscard]] data_vector write(std::vector<const ast::base*> nodes) {
  return latest_store::_singleton.write(nodes);
}

}  // namespace marlin::store
