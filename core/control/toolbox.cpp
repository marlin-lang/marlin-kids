#include "toolbox.hpp"

namespace marlin::control {

const prototype& toolbox::use_current_category_prototype(size_t index) {
  static constexpr size_t RECENT_MAX_SIZE{20};

  if (current_category().type != category::category_type::recent) {
    auto& prototype{current_category_prototype(index)};
    auto it{std::find(_recent.begin(), _recent.end(), &prototype)};
    if (it != _recent.end()) {
      _recent.erase(it);
    }
    _recent.insert(_recent.begin(), &prototype);
    if (_recent.size() > RECENT_MAX_SIZE) {
      _recent.erase(_recent.begin() + RECENT_MAX_SIZE, _recent.end());
    }
    return prototype;
  } else {
    return current_category_prototype(index);
  }
}

}  // namespace marlin::control