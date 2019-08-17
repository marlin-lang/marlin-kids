#include "expression_inserter.hpp"

#include "store.hpp"

namespace marlin::control {

std::optional<source_replacement> expression_inserter::insert(
    store::data_view data) {
  assert(_selection != nullptr);

  std::optional<store::reconstruction_result> try_result;
  try {
    try_result = store::read(data, *_selection);
  } catch (const store::read_error&) {
    // Leave try_result as std::nullopt
  }

  if (try_result.has_value()) {
    auto& result{*try_result};
    assert(result.nodes.size() == 1);

    auto original{_selection->source_code_range};
    _doc->replace_expression(*_selection, std::move(result.nodes[0]));
    return source_replacement{original, std::move(result.source),
                              std::move(result.highlights)};
  } else {
    return std::nullopt;
  }
}

}  // namespace marlin::control