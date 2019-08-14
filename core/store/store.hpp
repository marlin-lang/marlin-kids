#ifndef marlin_store_store_hpp
#define marlin_store_store_hpp

#include "store_definition.hpp"

// Stores
#include "store_errors.hpp"
#include "v1_store.hpp"

namespace marlin::store {

[[nodiscard]] inline reconstruction_result read(
    const std::string& data, const ast::base* parent = nullptr) {
  for (auto* s : base_store::_stores) {
    if (s->recognize(data)) {
      return s->read(data, parent);
    }
  }
  throw read_error{"Unrecognized data format!"};
}

[[nodiscard]] inline std::string write(std::vector<const ast::base*> nodes) {
  return latest_store::_singleton.write(nodes);
}

}  // namespace marlin::store

#endif  // marlin_store_store_hpp