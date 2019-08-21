#ifndef marlin_control_expression_inserter_hpp
#define marlin_control_expression_inserter_hpp

#include <optional>

#include "ast.hpp"
#include "byte_span.hpp"
#include "document.hpp"
#include "prototypes.hpp"

namespace marlin::control {

enum class literal_data_type { variable_name, identifier, number, string };

struct expression_inserter {
  friend struct source_selection;

  expression_inserter(document& doc) : _doc{&doc} {}

  bool can_insert() const noexcept { return _selection != nullptr; }
  source_range get_range() const noexcept {
    assert(_selection != nullptr);
    return _selection->source_code_range;
  }

  void move_to_loc(source_loc loc) {
    if (_loc == source_loc{} || _loc != loc) {
      auto& node{_doc->locate(loc)};
      if (node.is<ast::expression_placeholder>()) {
        _selection = &node;
      } else {
        _selection = nullptr;
      }
      _loc = loc;
    }
  }

  template <typename... arg_type>
  std::optional<source_update> insert_literal(literal_data_type type,
                                              arg_type&&... args) {
    switch (type) {
      case literal_data_type::variable_name:
        [[fallthrough]];
      case literal_data_type::identifier:
        return insert(
            identifier_prototype::data(std::forward<arg_type>(args)...));
      case literal_data_type::number:
        return insert(number_prototype::data(std::forward<arg_type>(args)...));
      case literal_data_type::string:
        return insert(string_prototype::data(std::forward<arg_type>(args)...));
    }
  }

  std::optional<source_update> insert(store::data_view data);

 private:
  document* _doc;
  source_loc _loc;
  ast::base* _selection{nullptr};

  // Special constructor for constructing from source_selection
  expression_inserter(document& doc, source_loc loc, ast::base& selection)
      : _doc{&doc}, _loc{loc}, _selection{&selection} {}
};  // namespace marlin::control

}  // namespace marlin::control

#endif  // marlin_control_statement_inserter_hpp
