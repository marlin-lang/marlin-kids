#ifndef marlin_control_source_selection_hpp
#define marlin_control_source_selection_hpp

#include <algorithm>
#include <utility>
#include <vector>

#include "ast.hpp"
#include "document.hpp"
#include "expression_inserter.hpp"
#include "formatter.hpp"
#include "function_definition.hpp"
#include "placeholders.hpp"
#include "store.hpp"

namespace marlin::control {

using selection_promotion_rule = ast::base& (*)(ast::base&);

constexpr selection_promotion_rule default_rule =
    [](ast::base& node) -> ast::base& {
  auto current{&node};
  ast::base* function_signature{nullptr};
  while (current->has_parent()) {
    current = &current->parent();
    if (current->is<ast::function_signature>()) {
      function_signature = current;
    }
  }
  if (function_signature != nullptr) {
    return *function_signature;
  } else {
    return node;
  }
};

constexpr selection_promotion_rule dragging_rule =
    [](ast::base& node) -> ast::base& {
  auto current{&node};
  ast::base* function_signature{nullptr};
  while (current->has_parent()) {
    current = &current->parent();
    if (current->is<ast::function_signature>()) {
      function_signature = current;
    }
  }
  if (function_signature != nullptr) {
    if (function_signature->has_parent()) {
      return function_signature->parent();
    } else {
      // Should not occur!
      assert(false);
      return *function_signature;
    }
  } else {
    return node;
  }
};

struct source_selection {
  struct literal_content {
    literal_data_type type;
    std::string content;

    literal_content(literal_data_type _type, std::string _content)
        : type{_type}, content{std::move(_content)} {}
  };

  source_selection(document& doc, source_loc loc,
                   selection_promotion_rule rule = default_rule)
      : source_selection{doc, loc, doc.locate(loc), rule} {}

  [[nodiscard]] source_range get_range() const noexcept {
    return _selection->source_code_range;
  }

  [[nodiscard]] store::data_vector get_data() const {
    return store::write({_selection});
  }

  [[nodiscard]] literal_content get_literal_content() const {
    assert(is_literal());
    return _selection->apply<literal_content>(
        [this](const auto& n) { return get_literal_content(n); });
  }

  [[nodiscard]] function_definition get_function_signature() const {
    assert(is_function_signature());

    function_definition signature;
    if (_selection->is<ast::function_signature>()) {
      auto& signature_node{_selection->as<ast::function_signature>()};
      signature.name = signature_node.name;
      for (auto& child : signature_node.parameters()) {
        if (child->is<ast::variable_name>()) {
          auto& variable{child->as<ast::variable_name>()};
          signature.parameters.emplace_back(variable.name);
        } else {
          // Should not occur!
          assert(false);
        }
      }
    }
    return signature;
  }

  [[nodiscard]] bool is_block() const {
    return _selection->inherits<ast::block>();
  }
  [[nodiscard]] bool is_statement() const {
    return _selection->inherits<ast::statement>();
  }
  [[nodiscard]] bool is_expression() const {
    return is_literal() || _selection->inherits<ast::expression>();
  }

  [[nodiscard]] bool is_function_signature() const {
    return _selection->is<ast::function_signature>() ||
           _selection->is<ast::function_placeholder>();
  }

  [[nodiscard]] bool is_literal() const {
    return _selection->is<marlin::ast::variable_placeholder>() ||
           _selection->is<marlin::ast::expression_placeholder>() ||
           _selection->is<marlin::ast::variable_name>() ||
           _selection->is<marlin::ast::number_literal>() ||
           _selection->is<marlin::ast::string_literal>() ||
           _selection->is<marlin::ast::identifier>();
  }

  [[nodiscard]] bool is_removable() const {
    return is_block() || is_statement() || is_expression();
  }

  [[nodiscard]] source_selection as_dragging_range() const&& {
    return {*_doc, _loc, *_selection, dragging_rule};
  }

  [[nodiscard]] expression_inserter as_expression_inserter() const&& {
    return {*_doc, _loc, *_selection};
  }

  std::vector<source_update> remove_from_document() const&&;

  std::vector<source_update> replace_function_signature(
      function_definition signature) const&&;

 private:
  document* _doc;
  source_loc _loc;
  ast::base* _selection;

  source_selection(document& doc, source_loc loc, ast::base& selection,
                   selection_promotion_rule rule)
      : _doc{&doc}, _loc{loc}, _selection{&rule(selection)} {}

  template <typename node_type>
  [[nodiscard]] literal_content get_literal_content(
      const node_type& node) const {
    // Not literal
    assert(false);
    return {literal_data_type::number, ""};
  }

  source_update remove_line() const&& {
    assert(_selection->has_parent());

    source_range line_range{{_selection->source_code_range.begin.line, 1},
                            {_selection->source_code_range.end.line + 1, 1}};
    auto line_offset{static_cast<ptrdiff_t>(line_range.begin.line) -
                     static_cast<ptrdiff_t>(line_range.end.line)};
    _doc->update_source_line_after_node(*_selection, line_offset);
    _doc->remove_line(*_selection);
    return source_update{line_range, {"", {}}};
  }

  source_update remove_expression() const&& {
    assert(_selection->has_parent());

    auto original_range{_selection->source_code_range};

    auto placeholder_name{placeholder::get_replacing_node(*_selection)};
    auto placeholder{ast::make<ast::expression_placeholder>(
        std::string{std::move(placeholder_name)})};

    // TODO: for functions, actually remove expression when there are excess
    // arguments originally

    format::formatter formatter;
    auto display{formatter.format(placeholder, *_selection)};
    _doc->replace_expression(*_selection, std::move(placeholder));
    return source_update{original_range, std::move(display)};
  }
};

template <>
[[nodiscard]] inline source_selection::literal_content
source_selection::get_literal_content<ast::variable_placeholder>(
    const ast::variable_placeholder& node) const {
  return {literal_data_type::variable_name, ""};
}

template <>
[[nodiscard]] inline source_selection::literal_content
source_selection::get_literal_content<ast::variable_name>(
    const ast::variable_name& node) const {
  return {literal_data_type::variable_name, node.name};
}

template <>
[[nodiscard]] inline source_selection::literal_content
source_selection::get_literal_content<ast::expression_placeholder>(
    const ast::expression_placeholder& node) const {
  return {literal_data_type::number, ""};
}

template <>
[[nodiscard]] inline source_selection::literal_content
source_selection::get_literal_content<ast::number_literal>(
    const ast::number_literal& node) const {
  return {literal_data_type::number, node.value};
}

template <>
[[nodiscard]] inline source_selection::literal_content
source_selection::get_literal_content<ast::string_literal>(
    const ast::string_literal& node) const {
  return {literal_data_type::string, node.value};
}

template <>
[[nodiscard]] inline source_selection::literal_content
source_selection::get_literal_content<ast::identifier>(
    const ast::identifier& node) const {
  return {literal_data_type::identifier, node.name};
}

}  // namespace marlin::control

#endif  // marlin_control_source_selection_hpp
