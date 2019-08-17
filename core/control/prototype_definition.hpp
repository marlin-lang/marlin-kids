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

struct base_prototype {
  virtual ~base_prototype() noexcept = default;

  [[nodiscard]] virtual std::string_view name() const = 0;
  [[nodiscard]] virtual store::data_view data() const = 0;
};

struct statement_prototype
    : base_prototype,
      prototype_container<statement_prototype, statement_prototype> {
  template <typename prototype>
  struct impl;
};

template <typename prototype>
struct statement_prototype::impl : statement_prototype,
                                   statement_prototype::element<prototype> {
  [[nodiscard]] store::data_view data() const override {
    return prototype::_data;
  }
};

struct expression_prototype
    : base_prototype,
      prototype_container<expression_prototype, expression_prototype> {
  template <typename prototype>
  struct impl;
};

template <typename prototype>
struct expression_prototype::impl : expression_prototype,
                                    expression_prototype::element<prototype> {
  [[nodiscard]] store::data_view data() const override {
    return prototype::_data;
  }
};

inline static auto& statement_prototypes{statement_prototype::elements()};

inline static auto& expression_prototypes{expression_prototype::elements()};

}  // namespace marlin::control

#endif  // marlin_control_prototype_definition_hpp
