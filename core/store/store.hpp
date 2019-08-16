#ifndef marlin_store_store_hpp
#define marlin_store_store_hpp

#include "store_definition.hpp"
#include "store_errors.hpp"

// Stores
#include "v1_store.hpp"

namespace marlin::store {

[[nodiscard]] inline reconstruction_result read(std::string_view data,
                                                const ast::base* parent,
                                                size_t start_line) {
  for (auto* s : base_store::get_stores()) {
    if (s->recognize(data)) {
      return s->read(std::move(data), parent, start_line);
    }
  }
  throw read_error{"Unrecognized data format!"};
}

[[nodiscard]] inline reconstruction_result read(std::string_view data,
                                                const ast::base* target) {
  for (auto* s : base_store::get_stores()) {
    if (s->recognize(data)) {
      return s->read(std::move(data), target);
    }
  }
  throw read_error{"Unrecognized data format!"};
}

[[nodiscard]] inline reconstruction_result read(std::string_view data) {
  return read(std::move(data), nullptr, 1);
}

[[nodiscard]] inline std::string write(std::vector<const ast::base*> nodes) {
  return latest_store::_singleton.write(nodes);
}

}  // namespace marlin::store

#endif  // marlin_store_store_hpp