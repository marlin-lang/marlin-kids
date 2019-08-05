#ifndef marlin_control_prototypes_hpp
#define marlin_control_prototypes_hpp

#include <array>
#include <utility>

#include "node.hpp"
#include "source_modifications.hpp"

namespace marlin::control {

struct statement_prototype {
  std::string name;
  std::pair<ast::node, source_insertion> (*construct)(size_t line,
                                                      size_t indent);

  statement_prototype(std::string _name,
                      std::pair<ast::node, source_insertion> (*_construct)(
                          size_t line, size_t indent))
      : name{std::move(_name)}, construct{_construct} {}
};

extern const std::array<statement_prototype, 1> statement_prototypes;

}  // namespace marlin::control

#endif  // marlin_control_prototypes_hpp