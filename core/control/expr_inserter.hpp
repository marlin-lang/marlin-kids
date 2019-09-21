#ifndef marlin_control_expr_inserter_hpp
#define marlin_control_expr_inserter_hpp

#include <type_traits>
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

enum struct literal_data_type { variable_name, identifier, number, string };

struct source_selection;

template <pasteboard_t node_type>
using expr_enable_t = std::enable_if_t<node_type == pasteboard_t::expression ||
                                       node_type == pasteboard_t::reference>;

template <pasteboard_t node_type, typename = expr_enable_t<node_type>>
struct expr_inserter {
  friend source_selection;

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
  source_loc get_selection_loc() const noexcept { return _loc; }
  source_range get_range() const noexcept {
    assert(_selection != nullptr);
    return _selection->source_code_range;
  }

  void move_to_loc(source_loc loc, const source_selection* exclusion = nullptr);

  source_update insert_literal(literal_data_type type,
                               std::string_view literal) const&& {
    if constexpr (node_type == pasteboard_t::expression) {
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
    } else if constexpr (node_type == pasteboard_t::reference) {
      if (type == literal_data_type::variable_name ||
          type == literal_data_type::identifier) {
        return std::move(*this).insert_identifier(std::move(literal));
      } else {
        // Should not happen
        assert(false);
        return {{{0, 0}, {0, 0}}, {"", {}}};
      }
    } else {
      // Should not happen
      static_assert(details::dependent_false<node_type>::value);
    }
  }

  std::vector<source_update> insert(store::data_view data) const&&;

 private:
  struct placeholder;

  document* _doc;
  source_loc _loc;
  ast::base* _selection{nullptr};

  // Special constructor for constructing from source_selection
  expr_inserter(document& doc, source_loc loc, ast::base& selection)
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
    if constexpr (node_type == pasteboard_t::expression) {
      auto node{ast::make<ast::identifier>(std::string{std::move(value)})};
      return std::move(*this).insert_literal(std::move(node));
    } else if constexpr (node_type == pasteboard_t::reference) {
      auto node{
          _selection->is<ast::variable_placeholder>() ||
                  _selection->inherits<ast::lvalue>()
              ? ast::make<ast::variable_name>(std::string{std::move(value)})
              : ast::make<ast::identifier>(std::string{std::move(value)})};
      return std::move(*this).insert_literal(std::move(node));
    } else {
      // Should not happen
      static_assert(details::dependent_false<node_type>::value);
    }
  }

  source_update insert_literal(ast::node node) const&& {
    auto original{_selection->source_code_range};

    format::in_place_formatter formatter;
    auto display{formatter.format(node, *_selection)};

    _doc->replace_expression(*_selection, std::move(node));
    return source_update{original, std::move(display)};
  }
};

using expression_inserter = expr_inserter<pasteboard_t::expression>;
using reference_inserter = expr_inserter<pasteboard_t::reference>;

}  // namespace marlin::control

#endif  // marlin_control_expr_inserter_hpp
