#ifndef marlin_control_expression_inserter_hpp
#define marlin_control_expression_inserter_hpp

#include <vector>

#include "ast.hpp"
#include "byte_span.hpp"
#include "document.hpp"
#include "formatter.hpp"
#include "prototypes.hpp"

namespace marlin::control {

enum struct literal_data_type { variable_name, identifier, number, string };

struct source_selection;

struct expression_inserter {
  friend source_selection;

  expression_inserter(document& doc) : _doc{&doc} {}

  bool can_insert() const noexcept { return _selection != nullptr; }
  source_loc get_selection_loc() const noexcept { return _loc; }
  source_range get_range() const noexcept {
    assert(_selection != nullptr);
    return _selection->source_code_range;
  }

  void move_to_loc(source_loc loc, const source_selection* exclusion = nullptr);

  source_update insert_literal(literal_data_type type,
                               std::string_view literal) const&& {
    switch (type) {
      case literal_data_type::variable_name:
        [[fallthrough]];
      case literal_data_type::identifier:
        return std::move(*this).insert_identifier(std::move(literal));
      case literal_data_type::number:
        return std::move(*this).insert_number(std::move(literal));
      case literal_data_type::string:
        return std::move(*this).insert_string(std::move(literal));
    }
  }

  std::vector<source_update> insert(store::data_view data) const&&;

 private:
  document* _doc;
  source_loc _loc;
  ast::base* _selection{nullptr};

  // Special constructor for constructing from source_selection
  expression_inserter(document& doc, source_loc loc, ast::base& selection)
      : _doc{&doc}, _loc{loc}, _selection{&selection} {}

  source_update insert_number(std::string_view value) const&& {
    auto node = ast::make<ast::number_literal>(std::string{std::move(value)});
    return std::move(*this).insert_literal(std::move(node));
  }

  source_update insert_string(std::string_view value) const&& {
    auto node = ast::make<ast::string_literal>(std::string{std::move(value)});
    return std::move(*this).insert_literal(std::move(node));
  }

  source_update insert_identifier(std::string_view value) const&& {
    auto node = ast::make<ast::identifier>(std::string{std::move(value)});
    return std::move(*this).insert_literal(std::move(node));
  }

  source_update insert_literal(ast::node node) const&& {
    auto original{_selection->source_code_range};

    format::formatter formatter;
    auto display{formatter.format(node, *_selection)};

    _doc->replace_expression(*_selection, std::move(node));
    return source_update{original, std::move(display)};
  }
};

}  // namespace marlin::control

#endif  // marlin_control_statement_inserter_hpp
