#include "statement_inserter.hpp"

#include "store.hpp"

namespace marlin::control {

std::optional<source_insertion> statement_inserter::insert(
    store::data_view data) {
  assert(_loc.has_value());

  std::optional<store::reconstruction_result> try_result;
  try {
    try_result = store::read(data, *_loc->parent, _loc->line);
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

    return source_insertion{{_loc->line, 1},
                            std::move(result.source),
                            std::move(result.highlights)};
  } else {
    return std::nullopt;
  }
}

template <bool vector_is_block>
std::optional<statement_inserter::location>
statement_inserter::find_statement_insert_location_in_vector(
    size_t line, ast::base& parent, ast::subnode::vector_view<ast::base> vector,
    size_t current_indent) {
  for (size_t i{0}; i < vector.size(); i++) {
    if (line <= vector[i]->source_code_range.begin.line) {
      if constexpr (vector_is_block) {
        return location{parent, vector, i, line, current_indent};
      } else {
        return std::nullopt;
      }
    } else if (line <= vector[i]->source_code_range.end.line) {
      return find_statement_insert_location_in_node(line, *vector[i],
                                                    current_indent + 1);
    }
  }
  if constexpr (vector_is_block) {
    return location{parent, vector, vector.size(), line, current_indent};
  } else {
    return std::nullopt;
  }
}

template std::optional<statement_inserter::location>
statement_inserter::find_statement_insert_location_in_vector<true>(
    size_t line, ast::base& parent, ast::subnode::vector_view<ast::base> vector,
    size_t current_indent);
template std::optional<statement_inserter::location>
statement_inserter::find_statement_insert_location_in_vector<false>(
    size_t line, ast::base& parent, ast::subnode::vector_view<ast::base> vector,
    size_t current_indent);

}  // namespace marlin::control
