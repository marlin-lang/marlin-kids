#ifndef marlin_control_expression_inserter_hpp
#define marlin_control_expression_inserter_hpp

#include "ast.hpp"
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
  source_replacement insert_literal(literal_data_type type,
                                    arg_type&&... args) {
    switch (type) {
      case literal_data_type::variable_name:
        return insert_literal<variable_name_prototype>(
            std::forward<arg_type>(args)...);
      case literal_data_type::identifier:
        return insert_literal<identifier_prototype>(
            std::forward<arg_type>(args)...);
      case literal_data_type::number:
        return insert_literal<number_prototype>(
            std::forward<arg_type>(args)...);
      case literal_data_type::string:
        return insert_literal<string_prototype>(
            std::forward<arg_type>(args)...);
    }
  }

  source_replacement insert_prototype(size_t index) {
    assert(_selection != nullptr);

    auto [node, update]{expression_prototypes[index]->construct(*_selection)};
    _doc->replace_expression(*_selection, std::move(node));
    return update;
  }

 private:
  document* _doc;
  source_loc _loc;
  ast::base* _selection{nullptr};

  // Special constructor for constructing from source_selection
  expression_inserter(document& doc, source_loc loc, ast::base& selection)
      : _doc{&doc}, _loc{loc}, _selection{&selection} {}

  template <typename prototype_type, typename... arg_type>
  source_replacement insert_literal(arg_type&&... args) {
    assert(_selection != nullptr);

    auto [node, update]{prototype_type::construct(
        _selection->source_code_range, std::forward<arg_type>(args)...)};
    _doc->replace_expression(*_selection, std::move(node));
    return update;
  }
};

}  // namespace marlin::control

#endif  // marlin_control_statement_inserter_hpp
