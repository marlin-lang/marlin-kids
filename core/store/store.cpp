#include "store.hpp"

#include "store_errors.hpp"

// Stores
#include "v1_store.hpp"

namespace marlin::store {

[[nodiscard]] reconstruction_result read(std::string_view data,
                                         const ast::base* parent,
                                         size_t start_line) {
  for (auto* s : base_store::get_stores()) {
    if (s->recognize(data)) {
      size_t indent{0};
      if (parent != nullptr) {
        const ast::base* curr_parent{parent};
        while (curr_parent->has_parent()) {
          indent++;
          curr_parent = &curr_parent->parent();
        }
      }

      return s->read(std::move(data), {start_line, 1}, indent);
    }
  }
  throw read_error{"Unrecognized data format!"};
}

[[nodiscard]] reconstruction_result read(std::string_view data,
                                         const ast::base& target) {
  for (auto* s : base_store::get_stores()) {
    if (s->recognize(data)) {
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

      return s->read(std::move(data), target.source_code_range.begin, 0,
                     paren_precedence);
    }
  }
  throw read_error{"Unrecognized data format!"};
}

[[nodiscard]] std::string write(std::vector<const ast::base*> nodes) {
  return latest_store::_singleton.write(nodes);
}

}  // namespace marlin::store