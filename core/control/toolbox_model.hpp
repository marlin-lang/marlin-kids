#ifndef marlin_control_toolbox_model_hpp
#define marlin_control_toolbox_model_hpp

#include <array>
#include <string>

#include "prototypes.hpp"

namespace marlin::control {

struct toolbox_model {
  struct item {
    enum type_t { statement = 0, expression };

    size_t index;
    type_t type;
  };

  inline static const char* pasteboard_type() noexcept { return "marlin"; }

  inline static auto& sections() noexcept {
    static std::array _sections = {"statement", "expression"};
    return _sections;
  }

  inline static auto& items() noexcept {
    static std::array _items = {
        std::array{
            item{assignment_prototype::index(), item::statement},
            item{if_prototype::index(), item::statement},
            item{if_else_prototype::index(), item::statement},
            item{print_prototype::index(), item::statement},
        },
        std::array{
            item{binary_prototype<ast::binary_op::add>::index(),
                 item::expression},
            item{binary_prototype<ast::binary_op::subtract>::index(),
                 item::expression},
            item{binary_prototype<ast::binary_op::multiply>::index(),
                 item::expression},
            item{binary_prototype<ast::binary_op::divide>::index(),
                 item::expression},
        },
    };
    return _items;
  }

  inline static std::string nameOfItemAt(size_t section, size_t item) {
    switch (items()[section][item].type) {
      case item::statement:
        return statement_prototypes[items()[section][item].index]->name();
      case item::expression:
        return expression_prototypes[items()[section][item].index]->name();
    }
  }
};

}  // namespace marlin::control

#endif  // marlin_control_toolbox_model_hpp
