#ifndef marlin_control_prototype_definition_hpp
#define marlin_control_prototype_definition_hpp

#include <array>
#include <string_view>
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
    element() {
      _index = mutable_elements().size();
      mutable_elements().emplace_back(&_singleton);
    }

   private:
    template <concrete_type&>
    struct proto_ref {};

    inline static size_t _index;
    inline static concrete_type _singleton;
    inline static proto_ref<_singleton> _ref;
  };

  [[nodiscard]] static const std::vector<const element_type*>& elements() {
    return mutable_elements();
  }

 private:
  [[nodiscard]] static std::vector<const element_type*>& mutable_elements() {
    static std::vector<const element_type*> _elements;
    return _elements;
  }
};

struct statement_prototype
    : prototype_container<statement_prototype, statement_prototype> {
  template <typename prototype>
  struct impl;

  virtual ~statement_prototype() noexcept = default;

  [[nodiscard]] virtual std::string_view name() const = 0;
  [[nodiscard]] virtual std::pair<ast::node, source_insertion> construct(
      const ast::base& parent, size_t line) const = 0;
};

template <typename prototype>
struct statement_prototype::impl : statement_prototype,
                                   statement_prototype::element<prototype> {
  [[nodiscard]] std::pair<ast::node, source_insertion> construct(
      const ast::base& parent, size_t line) const override {
    // store::read throws exceptions,
    // but we assert that reading prototype data does not
    auto result{store::read(prototype::data, parent, line)};
    assert(result.nodes.size() == 1);
    return std::make_pair(
        std::move(result.nodes[0]),
        source_insertion{
            {line, 1}, std::move(result.source), std::move(result.highlights)});
  }
};

struct expression_prototype
    : prototype_container<expression_prototype, expression_prototype> {
  template <typename prototype>
  struct impl;

  virtual ~expression_prototype() noexcept = default;

  [[nodiscard]] virtual std::string_view name() const = 0;
  [[nodiscard]] virtual std::pair<ast::node, source_replacement> construct(
      const ast::base& target) const = 0;
};

template <typename prototype>
struct expression_prototype::impl : expression_prototype,
                                    expression_prototype::element<prototype> {
  [[nodiscard]] std::pair<ast::node, source_replacement> construct(
      const ast::base& target) const override {
    const auto original{target.source_code_range};
    // store::read throws exceptions,
    // but we assert that reading prototype data does not
    auto result{store::read(prototype::data, target)};
    assert(result.nodes.size() == 1);
    return std::make_pair(std::move(result.nodes[0]),
                          source_replacement{original, std::move(result.source),
                                             std::move(result.highlights)});
  }
};

inline static auto& statement_prototypes{statement_prototype::elements()};

inline static auto& expression_prototypes{expression_prototype::elements()};

}  // namespace marlin::control

#endif  // marlin_control_prototype_definition_hpp
