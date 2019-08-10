#ifndef marlin_control_prototypes_hpp
#define marlin_control_prototypes_hpp

#include <array>
#include <utility>

#include "node.hpp"
#include "source_modifications.hpp"

namespace marlin::control {

static constexpr size_t indent_space_count{2};

struct statement_prototype {
  std::string name;
  std::pair<ast::node, source_insertion> (*construct)(size_t line,
                                                      size_t indent);

  statement_prototype(std::string _name,
                      std::pair<ast::node, source_insertion> (*_construct)(
                          size_t line, size_t indent))
      : name{std::move(_name)}, construct{_construct} {}
};

extern const std::array<statement_prototype, 4> statement_prototypes;

template <typename container_type, typename element_type>
struct prototype_container {
  friend element_type;

  static const std::vector<const element_type*> elements() { return _elements; }

 private:
  inline static std::vector<const element_type*> _elements;

  static void register_elem(const element_type& elem) {
    _elements.emplace_back(&elem);
  }
};

struct new_statement_prototypes
    : prototype_container<new_statement_prototypes, statement_prototype> {};

}  // namespace marlin::control

#endif  // marlin_control_prototypes_hpp