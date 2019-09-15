#ifndef marlin_control_source_update_hpp
#define marlin_control_source_update_hpp

#include <string>
#include <vector>

#include "formatter.hpp"
#include "utils.hpp"

namespace marlin::control {

struct source_update {
  source_range range;
  format::display display;

  source_update(source_range _range, format::display _display)
      : range{_range}, display{std::move(_display)} {}
};

}  // namespace marlin::control

#endif  // marlin_control_source_update_hpp