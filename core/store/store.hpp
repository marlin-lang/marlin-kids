#ifndef marlin_store_store_hpp
#define marlin_store_store_hpp

#include "store_definition.hpp"

namespace marlin::store {

[[nodiscard]] reconstruction_result read(data_view data, size_t start_line,
                                         const ast::base& parent);

[[nodiscard]] reconstruction_result read(data_view data,
                                         const ast::base& target);

[[nodiscard]] reconstruction_result read(
    data_view data, type_expectation type = type_expectation::any);

[[nodiscard]] data_vector write(std::vector<const ast::base*> nodes);

}  // namespace marlin::store

#endif  // marlin_store_store_hpp