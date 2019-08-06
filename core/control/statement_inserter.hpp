#ifndef marlin_control_statement_inserter_hpp
#define marlin_control_statement_inserter_hpp

#include "document.hpp"

namespace marlin::control {

struct statement_inserter {
  statement_inserter(document& doc) : _doc{doc} {}

  bool can_insert() const noexcept { return _loc.has_value(); }

  void move_to_line(size_t line) {
    if (_line == 0 || line != _line) {
      _loc = _doc.find_statement_insert_location(line);
    }
  }

  source_insertion insert(const statement_prototype& prototype) {
    assert(_loc.has_value());
    return _doc.insert_statement(*_loc, prototype);
  }

 private:
  document& _doc;
  size_t _line{0};
  std::optional<document::statement_insert_location> _loc;
};

}  // namespace marlin::control

#endif  // marlin_control_statement_inserter_hpp