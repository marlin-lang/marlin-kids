#include "expr_inserter.hpp"

#include "source_selection.hpp"
#include "store.hpp"

namespace marlin::control {

template <pasteboard_t node_type, typename enable_type>
void expr_inserter<node_type, enable_type>::move_to_loc(
    source_loc loc, const source_selection* exclusion) {
  if (_loc == source_loc{} || _loc != loc) {
    auto& node{_doc->locate(loc)};
    if (placeholder_test(node)) {
      if (exclusion != nullptr) {
        const auto range{exclusion->get_range()};
        if (node.source_code_range.begin >= range.end ||
            node.source_code_range.end <= range.begin) {
          _selection = &node;
        } else {
          _selection = nullptr;
        }
      } else {
        _selection = &node;
      }
    } else {
      _selection = nullptr;
    }
    _loc = loc;
  }
}

template void expression_inserter::move_to_loc(
    source_loc loc, const source_selection* exclusion);
template void reference_inserter::move_to_loc(
    source_loc loc, const source_selection* exclusion);

template <pasteboard_t node_type, typename enable_type>
document_update expr_inserter<node_type, enable_type>::insert(
    store::data_view data) && {
  assert(_selection != nullptr);
  assert(placeholder_test(*_selection));

  document_update updates;
  _doc->start_recording_side_effects();

  std::optional<store::reconstruction_result> try_result;
  try {
    try_result = store::read(data, *_selection, *_doc);
  } catch (const store::read_error&) {
    // Leave try_result as std::nullopt
  }

  if (try_result.has_value()) {
    auto& result{*try_result};
    assert(result.nodes.size() == 1);

    updates.selection_update =
        source_selection{*_doc, *result.nodes[0], dropping_rule};

    auto original{_selection->source_code_range};
    _doc->replace_expression(*_selection, std::move(result.nodes[0]));
    updates.source_updates.emplace_back(original, std::move(result.display));
  }

  _doc->gather_side_effects(updates.source_updates);
  return updates;
}

template document_update expression_inserter::insert(store::data_view data) &&;
template document_update reference_inserter::insert(store::data_view data) &&;

template <pasteboard_t node_type, typename enable_type>
document_update expr_inserter<node_type, enable_type>::insert_literal(
    literal_data_type type, std::string_view literal) && {
  document_update updates;
  _doc->start_recording_side_effects();

  auto& doc{*_doc};

  std::optional<source_update> main_update;
  ast::base* new_node{nullptr};
  if constexpr (node_type == pasteboard_t::expression) {
    switch (type) {
      case literal_data_type::number:
        std::tie(main_update, new_node) =
            std::move(*this).insert_number(std::move(literal));
        break;
      case literal_data_type::string:
        std::tie(main_update, new_node) =
            std::move(*this).insert_string(std::move(literal));
        break;
      default:
        // Should not happen
        assert(false);
    }
  } else if constexpr (node_type == pasteboard_t::reference) {
    switch (type) {
      case literal_data_type::parameter:
        [[fallthrough]];
      case literal_data_type::variable_name:
        [[fallthrough]];
      case literal_data_type::identifier:
        std::tie(main_update, new_node) =
            std::move(*this).insert_identifier(type, std::move(literal));
        break;
      default:
        // Should not happen
        assert(false);
    }
  } else {
    // Should not happen
    static_assert(details::dependent_false<node_type>::value);
  }

  if (main_update.has_value()) {
    updates.source_updates.emplace_back(*std::move(main_update));
  }
  if (new_node != nullptr) {
    updates.selection_update = source_selection{doc, *new_node, dropping_rule};
  }

  doc.gather_side_effects(updates.source_updates);
  return updates;
}

template document_update expression_inserter::insert_literal(
    literal_data_type type, std::string_view literal) &&;
template document_update reference_inserter::insert_literal(
    literal_data_type type, std::string_view literal) &&;

template <pasteboard_t node_type, typename enable_type>
std::pair<source_update, ast::base*>
expr_inserter<node_type, enable_type>::insert_identifier(
    literal_data_type type, std::string_view value) && {
  static_assert(node_type == pasteboard_t::reference);

  if (_selection->is<ast::parameter>()) {
    assert(type == literal_data_type::parameter);

    auto& doc{*_doc};
    const ast::function_signature* parent{nullptr};
    if (_selection->has_parent() &&
        _selection->parent().is<ast::function_signature>()) {
      parent = &_selection->parent().as<ast::function_signature>();
    }

    auto update{std::move(*this).insert_literal(
        ast::make<ast::parameter>(std::string{std::move(value)}))};

    if (parent != nullptr) {
      doc.refresh_function_signature(parent->name, *parent);
    }

    return update;
  } else if (_selection->is<ast::variable_placeholder>() ||
             _selection->inherits<ast::lvalue>()) {
    assert(type == literal_data_type::variable_name);

    return std::move(*this).insert_literal(
        ast::make<ast::variable_name>(std::string{std::move(value)}));
  } else {
    assert(type == literal_data_type::identifier);

    return std::move(*this).insert_literal(
        ast::make<ast::identifier>(std::string{std::move(value)}));
  }
}

template std::pair<source_update, ast::base*>
reference_inserter::insert_identifier(literal_data_type type,
                                      std::string_view value) &&;

}  // namespace marlin::control
