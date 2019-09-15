#ifndef marlin_control_source_inserters_hpp
#define marlin_control_source_inserters_hpp

#include <optional>

#include "expression_inserter.hpp"
#include "line_inserter.hpp"
#include "toolbox_model.hpp"

namespace marlin::control {

template <typename document_getter>
struct source_inserters {
  source_inserters(document_getter document) : _document{std::move(document)} {}

  bool move_to_loc(pasteboard_t type, source_loc loc) {
    return perform_on_inserter(type, [this, &loc](auto& inserter) {
      if (!inserter.has_value()) {
        inserter = {_document()};
      }
      inserter->move_to_loc(loc);
      return inserter->can_insert();
    });
  }

  auto insert(pasteboard_t type, store::data_view data) {
    return perform_on_inserter(type, [&data](auto& inserter) {
      if (inserter.has_value() && inserter->can_insert()) {
        if (auto update{inserter->insert(data)}) {
          inserter.reset();
          return update;
        }
      }
      return static_cast<std::optional<source_update>>(std::nullopt);
    });
  }

  std::optional<source_loc> block_insert_location() const {
    if (_block_inserter.has_value() && _block_inserter->can_insert()) {
      return _block_inserter->get_location();
    } else {
      return std::nullopt;
    }
  }

  std::optional<source_loc> statement_insert_location() const {
    if (_statement_inserter.has_value() && _statement_inserter->can_insert()) {
      return _statement_inserter->get_location();
    } else {
      return std::nullopt;
    }
  }

  std::optional<source_range> expression_insert_range() const {
    if (_expression_inserter.has_value() &&
        _expression_inserter->can_insert()) {
      return _expression_inserter->get_range();
    } else {
      return std::nullopt;
    }
  }

  void reset_all() {
    _block_inserter.reset();
    _statement_inserter.reset();
    _expression_inserter.reset();
  }

 private:
  document_getter _document;

  std::optional<block_inserter> _block_inserter;
  std::optional<statement_inserter> _statement_inserter;
  std::optional<expression_inserter> _expression_inserter;

  template <typename callable_type>
  auto perform_on_inserter(pasteboard_t type, callable_type&& callable) {
    switch (type) {
      case pasteboard_t::block:
        return callable(_block_inserter);
      case pasteboard_t::statement:
        return callable(_statement_inserter);
      case pasteboard_t::expression:
        return callable(_expression_inserter);
    }
  }
};

}  // namespace marlin::control

#endif  // marlin_control_source_inserters_hpp
