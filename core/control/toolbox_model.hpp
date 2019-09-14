#ifndef marlin_control_toolbox_model_hpp
#define marlin_control_toolbox_model_hpp

#include <array>
#include <string>
#include <string_view>
#include <vector>

#include "prototypes.hpp"

namespace marlin::control {

enum class pasteboard_t : uint8_t { statement = 0, expression };

struct toolbox_model {
  struct item {
    size_t index;
    pasteboard_t type;
  };

  inline static const std::array<std::string_view, 4> sections{
      "control flow", "statement", "expression", "function"};

  inline static const std::array items{
      std::vector{
          item{if_prototype::index(), pasteboard_t::statement},
          item{if_else_prototype::index(), pasteboard_t::statement},
          item{while_prototype::index(), pasteboard_t::statement},
          item{for_prototype::index(), pasteboard_t::statement},
          item{break_prototype::index(), pasteboard_t::statement},
          item{continue_prototype::index(), pasteboard_t::statement},
          item{
              system_procedure_prototype<ast::system_procedure::sleep>::index(),
              pasteboard_t::statement},
      },
      std::vector{
          item{assignment_prototype::index(), pasteboard_t::statement},
          item{use_global_prototype::index(), pasteboard_t::statement},
          item{
              system_procedure_prototype<ast::system_procedure::print>::index(),
              pasteboard_t::statement},
          item{system_procedure_prototype<
                   ast::system_procedure::draw_line>::index(),
               pasteboard_t::statement},
          item{system_procedure_prototype<
                   ast::system_procedure::logo_forward>::index(),
               pasteboard_t::statement},
          item{system_procedure_prototype<
                   ast::system_procedure::logo_backward>::index(),
               pasteboard_t::statement},
          item{system_procedure_prototype<
                   ast::system_procedure::logo_turn_left>::index(),
               pasteboard_t::statement},
          item{system_procedure_prototype<
                   ast::system_procedure::logo_turn_right>::index(),
               pasteboard_t::statement},
          item{system_procedure_prototype<
                   ast::system_procedure::logo_pen_up>::index(),
               pasteboard_t::statement},
          item{system_procedure_prototype<
                   ast::system_procedure::logo_pen_down>::index(),
               pasteboard_t::statement},
      },
      std::vector{
          item{binary_prototype<ast::binary_op::add>::index(),
               pasteboard_t::expression},
          item{binary_prototype<ast::binary_op::subtract>::index(),
               pasteboard_t::expression},
          item{binary_prototype<ast::binary_op::multiply>::index(),
               pasteboard_t::expression},
          item{binary_prototype<ast::binary_op::divide>::index(),
               pasteboard_t::expression},
          item{binary_prototype<ast::binary_op::equal>::index(),
               pasteboard_t::expression},
          item{binary_prototype<ast::binary_op::not_equal>::index(),
               pasteboard_t::expression},
          item{binary_prototype<ast::binary_op::less>::index(),
               pasteboard_t::expression},
          item{binary_prototype<ast::binary_op::less_equal>::index(),
               pasteboard_t::expression},
          item{binary_prototype<ast::binary_op::greater>::index(),
               pasteboard_t::expression},
          item{binary_prototype<ast::binary_op::greater_equal>::index(),
               pasteboard_t::expression},
          item{binary_prototype<ast::binary_op::logical_and>::index(),
               pasteboard_t::expression},
          item{binary_prototype<ast::binary_op::logical_or>::index(),
               pasteboard_t::expression},
      },
      std::vector{
          item{system_function_prototype<ast::system_function::range1>::index(),
               pasteboard_t::expression},
          item{system_function_prototype<ast::system_function::range2>::index(),
               pasteboard_t::expression},
          item{system_function_prototype<ast::system_function::range3>::index(),
               pasteboard_t::expression},
          item{system_function_prototype<ast::system_function::time>::index(),
               pasteboard_t::expression},
          item{system_function_prototype<ast::system_function::sin>::index(),
               pasteboard_t::expression},
          item{system_function_prototype<ast::system_function::cos>::index(),
               pasteboard_t::expression},
      },
  };

  static_assert(sections.size() == items.size());

  static const base_prototype& prototype_at(size_t section, size_t item) {
    switch (items[section][item].type) {
      case pasteboard_t::statement:
        return *statement_prototypes[items[section][item].index];
      case pasteboard_t::expression:
        return *expression_prototypes[items[section][item].index];
    }
  }
};

}  // namespace marlin::control

#endif  // marlin_control_toolbox_model_hpp
