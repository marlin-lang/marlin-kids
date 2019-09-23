#ifndef marlin_control_expr_inserter_hpp
#define marlin_control_expr_inserter_hpp

#include <type_traits>
#include <utility>
#include <vector>

#include "ast.hpp"
#include "byte_span.hpp"
#include "document.hpp"
#include "formatter.hpp"
#include "prototypes.hpp"

namespace marlin::control {

namespace details {

template <pasteboard_t node_type>
struct dependent_false : std::false_type {};

}  // namespace details

enum struct literal_data_type {
  parameter,
  variable_name,
  identifier,
  number,
  string
};

struct document_update;
struct source_selection;

template <pasteboard_t node_type>
using expr_enable_t = std::enable_if_t<node_type == pasteboard_t::expression ||
                                       node_type == pasteboard_t::reference>;

template <pasteboard_t node_type, typename = expr_enable_t<node_type>>
struct expr_inserter {
  friend source_selection;
  template <typename document_getter>
  friend struct source_inserters;

  static bool placeholder_test(const ast::base& node) {
    if constexpr (node_type == pasteboard_t::expression) {
      return node.is<ast::expression_placeholder>();
    } else if constexpr (node_type == pasteboard_t::reference) {
      return node.is<ast::expression_placeholder>() ||
             node.is<ast::variable_placeholder>();
    } else {
      // Should not happen
      static_assert(details::dependent_false<node_type>::value);
    }
  }

  expr_inserter(document& doc) : _doc{&doc} {}

  bool can_insert() const noexcept { return _selection != nullptr; }
  source_range get_range() const noexcept {
    assert(_selection != nullptr);
    return _selection->source_code_range;
  }

  void move_to_loc(source_loc loc, const source_selection* exclusion = nullptr);

  document_update insert(store::data_view data) &&;
  document_update insert_literal(literal_data_type type,
                                 std::string_view literal) &&;

 private:
  struct placeholder;

  document* _doc;
  source_loc _loc;
  ast::base* _selection{nullptr};

  // Special constructor for constructing from source_selection
  expr_inserter(document& doc, source_loc loc, ast::base& selection)
      : _doc{&doc}, _loc{loc}, _selection{&selection} {}

  std::pair<source_update, ast::base*> insert_number(
      std::string_view value) && {
    auto node = ast::make<ast::number_literal>(std::string{std::move(value)});
    return std::move(*this).insert_literal(std::move(node));
  }

  std::pair<source_update, ast::base*> insert_string(
      std::string_view value) && {
    auto node = ast::make<ast::string_literal>(std::string{std::move(value)});
    return std::move(*this).insert_literal(std::move(node));
  }

  // This function is only defined for reference_inserter (see cpp file)
  std::pair<source_update, ast::base*> insert_identifier(
      literal_data_type type, std::string_view value) &&;

  std::pair<source_update, ast::base*> insert_literal(ast::node node) && {
    auto original{_selection->source_code_range};

    format::in_place_formatter formatter;
    auto display{formatter.format(node, *_selection)};

    auto& base{*node};
    _doc->replace_expression(*_selection, std::move(node));
    return std::make_pair(source_update{original, std::move(display)}, &base);
  }
};

using expression_inserter = expr_inserter<pasteboard_t::expression>;
using reference_inserter = expr_inserter<pasteboard_t::reference>;

}  // namespace marlin::control

#endif  // marlin_control_expr_inserter_hpp
