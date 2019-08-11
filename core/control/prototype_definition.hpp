#ifndef marlin_control_prototype_definition_hpp
#define marlin_control_prototype_definition_hpp

#include <array>
#include <utility>

#include "node.hpp"
#include "source_modifications.hpp"

namespace marlin::control {

static constexpr size_t indent_space_count{2};

template <typename container_type, typename element_type>
struct prototype_container {
  template <typename concrete_type>
  struct element {
    static size_t index() { return _index; }

   protected:
    element() { _index = container_type::register_elem(_singleton); }

   private:
    template <concrete_type&>
    struct proto_ref {};

    inline static size_t _index;
    inline static concrete_type _singleton;
    inline static proto_ref<_singleton> _ref;
  };

  [[nodiscard]] static constexpr std::vector<const element_type*>& elements() {
    return _elements;
  }

 private:
  inline static std::vector<const element_type*> _elements;

  static size_t register_elem(const element_type& elem) {
    const auto index{_elements.size()};
    _elements.emplace_back(&elem);
    return index;
  }
};

struct statement_prototype
    : prototype_container<statement_prototype, statement_prototype> {
  template <typename prototype>
  struct impl;

  virtual ~statement_prototype() noexcept = default;

  [[nodiscard]] virtual std::string name() const = 0;
  [[nodiscard]] virtual std::pair<ast::node, source_insertion> construct(
      size_t line, size_t indent) const = 0;
};

template <typename prototype>
struct statement_prototype::impl : statement_prototype,
                                   statement_prototype::element<prototype> {
  [[nodiscard]] std::pair<ast::node, source_insertion> construct(
      size_t line, size_t indent) const override {
    return prototype::generator.construct(line, indent);
  }
};

struct expression_prototype
    : prototype_container<expression_prototype, expression_prototype> {
  template <typename prototype>
  struct impl;

  virtual ~expression_prototype() noexcept = default;

  [[nodiscard]] virtual std::string name() const = 0;
  [[nodiscard]] virtual std::pair<ast::node, source_replacement> construct(
      const ast::base& target) const = 0;
};

template <typename prototype>
struct expression_prototype::impl : expression_prototype,
                                    expression_prototype::element<prototype> {
  /*[[nodiscard]] std::pair<ast::node, source_insertion> construct(
      size_t line, size_t indent) const override {
    return prototype::generator.construct(line, indent);
  }*/
};

inline static constexpr auto& statement_prototypes =
    statement_prototype::elements();

inline static constexpr auto& expression_prototypes =
    expression_prototype::elements();

}  // namespace marlin::control

#endif  // marlin_control_prototype_definition_hpp
