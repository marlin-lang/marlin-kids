#ifndef marlin_control_toolbox_hpp
#define marlin_control_toolbox_hpp

#include <array>
#include <string>
#include <string_view>
#include <vector>

#include "prototypes.hpp"

namespace marlin::control {

struct toolbox {
  struct category {
    friend toolbox;

    enum struct category_type { recent, shared, local };

    std::string name;
    category_type type;

    category(std::string _name, const std::vector<prototype>& _prototypes)
        : name{std::move(_name)},
          type{category_type::shared},
          prototypes{&_prototypes} {}

    category(std::string _name, category_type _type)
        : name{std::move(_name)}, type{_type}, prototypes{nullptr} {}

   private:
    const std::vector<prototype>* prototypes;
  };

  toolbox() {
    static const std::array _default_prototypes{
        std::vector{function_prototype(), if_prototype(), if_else_prototype(),
                    while_prototype(), for_prototype(), break_prototype(),
                    continue_prototype(),
                    system_procedure_prototype(ast::system_procedure::sleep)},
        std::vector{
            assignment_prototype(), use_global_prototype(),
            system_procedure_prototype(ast::system_procedure::print),
            system_procedure_prototype(ast::system_procedure::draw_line),
            system_procedure_prototype(ast::system_procedure::logo_forward),
            system_procedure_prototype(ast::system_procedure::logo_backward),
            system_procedure_prototype(ast::system_procedure::logo_turn_left),
            system_procedure_prototype(ast::system_procedure::logo_turn_right),
            system_procedure_prototype(ast::system_procedure::logo_pen_up),
            system_procedure_prototype(ast::system_procedure::logo_pen_down)},
        std::vector{binary_prototype(ast::binary_op::add),
                    binary_prototype(ast::binary_op::subtract),
                    binary_prototype(ast::binary_op::multiply),
                    binary_prototype(ast::binary_op::divide),
                    binary_prototype(ast::binary_op::equal),
                    binary_prototype(ast::binary_op::not_equal),
                    binary_prototype(ast::binary_op::less),
                    binary_prototype(ast::binary_op::less_equal),
                    binary_prototype(ast::binary_op::greater),
                    binary_prototype(ast::binary_op::greater_equal),
                    binary_prototype(ast::binary_op::logical_and),
                    binary_prototype(ast::binary_op::logical_or)},
        std::vector{system_function_prototype(ast::system_function::range1),
                    system_function_prototype(ast::system_function::range2),
                    system_function_prototype(ast::system_function::range3),
                    system_function_prototype(ast::system_function::time),
                    system_function_prototype(ast::system_function::sin),
                    system_function_prototype(ast::system_function::cos)}};
    static const std::array<
        std::string_view, std::tuple_size<decltype(_default_prototypes)>::value>
        _default_names{"control flow", "statement", "expression", "function"};

    _categories.emplace_back("recent", category::category_type::recent);
    for (size_t i{0}; i < _default_prototypes.size(); i++) {
      _categories.emplace_back(std::string{_default_names[i]},
                               _default_prototypes[i]);
    }
  }

  const std::vector<category>& categories() const { return _categories; }
  const category& current_category() const {
    assert(_current_category < _categories.size());
    return _categories[_current_category];
  }
  size_t current_category_index() const { return _current_category; }

  void set_current_category(size_t index) {
    assert(index < _categories.size());
    _current_category = index;
  }

  size_t current_category_size() const {
    auto& category{current_category()};
    switch (category.type) {
      case category::category_type::recent:
        return _recent.size();
      case category::category_type::shared:
        return category.prototypes->size();
      case category::category_type::local:
        return 0;
    }
  }

  [[nodiscard]] const prototype& current_category_prototype(
      size_t index) const {
    assert(index < current_category_size());

    auto& category{current_category()};
    switch (category.type) {
      case category::category_type::recent:
        return *_recent[index];
      case category::category_type::shared:
        return (*category.prototypes)[index];
      case category::category_type::local:
        // TODO: to be implemented
        assert(false);
        throw std::exception{};
    }
  }

  const prototype& use_current_category_prototype(size_t index);

 private:
  std::vector<category> _categories;
  std::vector<const prototype*> _recent;

  // Default value: the first category that is not recent
  size_t _current_category{1};
};

}  // namespace marlin::control

#endif  // marlin_control_toolbox_hpp
