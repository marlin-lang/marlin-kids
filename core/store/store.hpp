#ifndef marlin_store_store_hpp
#define marlin_store_store_hpp

#include "store_definition.hpp"
#include "store_errors.hpp"

// Stores
#include "v1_store.hpp"

namespace marlin::store {

[[nodiscard]] inline reconstruction_result read(
    const std::string& data, const ast::base* parent = nullptr,
    source_loc start = {1, 1}) {
  for (auto* s : base_store::get_stores()) {
    if (s->recognize(data)) {
      return s->read(data, parent, start);
    }
  }
  throw read_error{"Unrecognized data format!"};
}

[[nodiscard]] inline std::string write(std::vector<const ast::base*> nodes) {
  return latest_store::_singleton.write(nodes);
}

}  // namespace marlin::store

#endif  // marlin_store_store_hpp