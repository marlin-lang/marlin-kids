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
std::vector<source_update> expr_inserter<node_type, enable_type>::insert(
    store::data_view data) const&& {
  assert(_selection != nullptr);
  assert(placeholder_test(*_selection));

  std::vector<source_update> updates;
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

    auto original{_selection->source_code_range};
    _doc->replace_expression(*_selection, std::move(result.nodes[0]));
    updates.emplace_back(original, std::move(result.display));
  }

  _doc->gather_side_effects(updates);
  return updates;
}

template std::vector<source_update> expression_inserter::insert(
    store::data_view data) const&&;
template std::vector<source_update> reference_inserter::insert(
    store::data_view data) const&&;

}  // namespace marlin::control
