#include "line_inserter.hpp"

#include "store.hpp"

namespace marlin::control {

template <line_node_type node_type>
std::optional<source_update> line_inserter<node_type>::insert(
    store::data_view data) {
  assert(_loc.has_value());

  std::optional<store::reconstruction_result> try_result;
  try {
    try_result = store::read(data, _loc->line, *_loc->parent, _doc->_functions);
  } catch (const store::read_error&) {
    // Leave try_result as std::nullopt
  }

  if (try_result.has_value()) {
    auto& result{*try_result};
    assert(result.nodes.size() > 0);
    auto line_offset{static_cast<ptrdiff_t>(
                         result.nodes.back()->source_code_range.end.line) +
                     1 - static_cast<ptrdiff_t>(_loc->line)};

    for (auto& node : result.nodes) {
      _loc->block.emplace(_loc->index, std::move(node));
    }
    _doc->update_source_line_after_node(*_loc->block[_loc->index], line_offset);

    return source_update{{{_loc->line, 1}, {_loc->line, 1}},
                         std::move(result.display)};
  } else {
    return std::nullopt;
  }
}

template std::optional<source_update> block_inserter::insert(
    store::data_view data);
template std::optional<source_update> statement_inserter::insert(
    store::data_view data);

template <line_node_type node_type>
template <line_node_type element_type>
std::optional<typename line_inserter<node_type>::location>
line_inserter<node_type>::find_insert_location_in_vector(
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
block_inserter::find_insert_location_in_vector<line_node_type::block>(
    size_t line, ast::base& parent, ast::subnode::vector_view<ast::base> vector,
    size_t current_indent);
template std::optional<block_inserter::location>
block_inserter::find_insert_location_in_vector<line_node_type::statement>(
    size_t line, ast::base& parent, ast::subnode::vector_view<ast::base> vector,
    size_t current_indent);
template std::optional<statement_inserter::location>
statement_inserter::find_insert_location_in_vector<line_node_type::block>(
    size_t line, ast::base& parent, ast::subnode::vector_view<ast::base> vector,
    size_t current_indent);
template std::optional<statement_inserter::location>
statement_inserter::find_insert_location_in_vector<line_node_type::statement>(
    size_t line, ast::base& parent, ast::subnode::vector_view<ast::base> vector,
    size_t current_indent);

}  // namespace marlin::control
