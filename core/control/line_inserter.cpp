#include "line_inserter.hpp"

#include "store.hpp"

namespace marlin::control {

template <pasteboard_t node_type, typename enable_type>
document_update line_inserter<node_type, enable_type>::insert(
    store::data_view data) {
  assert(_loc.has_value());

  document_update updates;
  _doc->start_recording_side_effects();

  std::optional<store::reconstruction_result> try_result;
  try {
    try_result = store::read(data, _loc->line, *_loc->parent, *_doc);
  } catch (const store::read_error&) {
    // Leave try_result as std::nullopt
  }

  if (try_result.has_value()) {
    auto& result{*try_result};
    assert(result.nodes.size() > 0);
    auto line_offset{static_cast<ptrdiff_t>(
                         result.nodes.back()->source_code_range.end.line) +
                     1 - static_cast<ptrdiff_t>(_loc->line)};

    // For now, we only support selecting one node
    assert(result.nodes.size() == 1);
    updates.selection_update = source_selection{*_doc, *result.nodes[0]};

    for (auto& node : result.nodes) {
      _loc->block.emplace(_loc->index, std::move(node));
    }
    _doc->update_source_line_after_node(*_loc->block[_loc->index], line_offset);

    updates.source_updates.emplace_back(
        source_range{{_loc->line, 1}, {_loc->line, 1}},
        std::move(result.display));
  }

  _doc->gather_side_effects(updates.source_updates);
  return updates;
}

template document_update block_inserter::insert(store::data_view data);
template document_update statement_inserter::insert(store::data_view data);

template <pasteboard_t node_type, typename enable_type>
template <pasteboard_t element_type, typename>
std::optional<typename line_inserter<node_type, enable_type>::location>
line_inserter<node_type, enable_type>::find_insert_location_in_vector(
    size_t line, ast::base& parent, ast::subnode::vector_view<ast::base> vector,
    size_t current_indent) {
  if constexpr (static_cast<uint8_t>(node_type) >=
                static_cast<uint8_t>(element_type)) {
    for (size_t i{0}; i < vector.size(); i++) {
      if (line <= vector[i]->source_code_range.begin.line) {
        if constexpr (node_type == element_type) {
          return location{parent, vector, i, line, current_indent};
        } else {
          return std::nullopt;
        }
      } else if (line <= vector[i]->source_code_range.end.line) {
        return find_insert_location_in_base(line, *vector[i],
                                            current_indent + 1);
      }
    }
    if constexpr (node_type == element_type) {
      return location{parent, vector, vector.size(), line, current_indent};
    } else {
      return std::nullopt;
    }
  } else {
    return std::nullopt;
  }
}

template std::optional<block_inserter::location>
block_inserter::find_insert_location_in_vector<pasteboard_t::block>(
    size_t line, ast::base& parent, ast::subnode::vector_view<ast::base> vector,
    size_t current_indent);
template std::optional<block_inserter::location>
block_inserter::find_insert_location_in_vector<pasteboard_t::statement>(
    size_t line, ast::base& parent, ast::subnode::vector_view<ast::base> vector,
    size_t current_indent);
template std::optional<statement_inserter::location>
statement_inserter::find_insert_location_in_vector<pasteboard_t::block>(
    size_t line, ast::base& parent, ast::subnode::vector_view<ast::base> vector,
    size_t current_indent);
template std::optional<statement_inserter::location>
statement_inserter::find_insert_location_in_vector<pasteboard_t::statement>(
    size_t line, ast::base& parent, ast::subnode::vector_view<ast::base> vector,
    size_t current_indent);

}  // namespace marlin::control
