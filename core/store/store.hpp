#ifndef marlin_store_store_hpp
#define marlin_store_store_hpp

#include "store_definition.hpp"

namespace marlin::store {

[[nodiscard]] reconstruction_result read(std::string_view data,
                                         const ast::base* parent,
                                         size_t start_line);

[[nodiscard]] reconstruction_result read(std::string_view data,
                                         const ast::base& target);

[[nodiscard]] inline reconstruction_result read(std::string_view data) {
  return read(std::move(data), nullptr, 1);
}

[[nodiscard]] std::string write(std::vector<const ast::base*> nodes);

}  // namespace marlin::store

#endif  // marlin_store_store_hpp