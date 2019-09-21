#ifndef marlin_control_source_inserters_hpp
#define marlin_control_source_inserters_hpp

#include <optional>
#include <vector>

#include "expr_inserter.hpp"
#include "line_inserter.hpp"
#include "prototypes.hpp"
#include "source_selection.hpp"

namespace marlin::control {

template <typename document_getter>
struct source_inserters {
  source_inserters(document_getter document) : _document{std::move(document)} {}

  bool move_to_loc(pasteboard_t type, source_loc loc,
                   const source_selection* exclusion = nullptr) {
    return perform_on_inserter(type, [this, &loc, &exclusion](auto& inserter) {
      if (!inserter.has_value()) {
        inserter = {_document()};
      }
      inserter->move_to_loc(loc, exclusion);
      return inserter->can_insert();
    });
  }

  auto insert(pasteboard_t type, store::data_view data) {
    return perform_on_inserter(type, [&data](auto& inserter) {
      if (inserter.has_value() && inserter->can_insert()) {
        return (*std::exchange(inserter, std::nullopt)).insert(data);
      } else {
        return std::vector<source_update>{};
      }
    });
  }

  void update_lines(line_update update) {
    if (update.start_line > 0) {
      update_lines(_block_inserter, update);
      update_lines(_statement_inserter, update);
      update_lines(_expression_inserter, update);
      update_lines(_reference_inserter, update);
    }
  }

  std::vector<source_loc> line_insert_locations() const {
    std::vector<source_loc> result;
    if (_block_inserter.has_value() && _block_inserter->can_insert()) {
      result.emplace_back(_block_inserter->get_insert_location());
    }
    if (_statement_inserter.has_value() && _statement_inserter->can_insert()) {
      result.emplace_back(_statement_inserter->get_insert_location());
    }
    return result;
  }

  std::vector<source_range> expr_insert_ranges() const {
    std::vector<source_range> result;
    if (_expression_inserter.has_value() &&
        _expression_inserter->can_insert()) {
      result.emplace_back(_expression_inserter->get_range());
    }
    if (_reference_inserter.has_value() && _reference_inserter->can_insert()) {
      result.emplace_back(_reference_inserter->get_range());
    }
    return result;
  }

  void reset_all() {
    _block_inserter.reset();
    _statement_inserter.reset();
    _expression_inserter.reset();
    _reference_inserter.reset();
  }

 private:
  document_getter _document;

  std::optional<block_inserter> _block_inserter;
  std::optional<statement_inserter> _statement_inserter;
  std::optional<expression_inserter> _expression_inserter;
  std::optional<reference_inserter> _reference_inserter;

  template <typename callable_type>
  auto perform_on_inserter(pasteboard_t type, callable_type&& callable) {
    switch (type) {
      case pasteboard_t::block:
        return callable(_block_inserter);
      case pasteboard_t::statement:
        return callable(_statement_inserter);
      case pasteboard_t::expression:
        return callable(_expression_inserter);
      case pasteboard_t::reference:
        return callable(_reference_inserter);
    }
  }

  template <typename inserter_type>
  void update_lines(std::optional<inserter_type>& inserter,
                    const line_update& update) {
    if (inserter.has_value() && inserter->can_insert()) {
      source_loc location;
      if constexpr (std::is_same_v<inserter_type, block_inserter> ||
                    std::is_same_v<inserter_type, statement_inserter>) {
        location = inserter->get_insert_location();
      } else {
        location = inserter->get_selection_loc();
      }
      if (location.line >= update.start_line) {
        inserter->move_to_loc({location.line + update.offset, location.column});
        assert(inserter->can_insert());
      }
    }
  }
};

}  // namespace marlin::control

#endif  // marlin_control_source_inserters_hpp
