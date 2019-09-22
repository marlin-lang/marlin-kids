#ifndef marlin_control_source_selection_hpp
#define marlin_control_source_selection_hpp

#include <algorithm>
#include <utility>
#include <vector>

#include "ast.hpp"
#include "document.hpp"
#include "expr_inserter.hpp"
#include "formatter.hpp"
#include "function_definition.hpp"
#include "placeholders.hpp"
#include "store.hpp"

namespace marlin::control {

namespace details {

template <pasteboard_t node_type>
struct ast_tag {};

template <>
struct ast_tag<pasteboard_t::block> {
  using type = ast::block;
};

template <>
struct ast_tag<pasteboard_t::statement> {
  using type = ast::statement;
};

template <>
struct ast_tag<pasteboard_t::expression> {
  using type = ast::expression;
};

template <>
struct ast_tag<pasteboard_t::reference> {
  using type = ast::reference;
};

}  // namespace details

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

struct line_update {
  size_t start_line;
  size_t offset;

  line_update() : line_update{0, 0} {}
  line_update(size_t _start_line, size_t _offset)
      : start_line{_start_line}, offset{_offset} {}
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

  [[nodiscard]] store::data_vector get_data(
      bool erase_function_names = false) const {
    if (erase_function_names) {
      return store::write({_selection}, placeholder::get<ast::function>(0));
    } else {
      return store::write({_selection});
    }
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
      fetch_parameters(signature, signature_node);
    } else if (_selection->is<ast::function_placeholder>()) {
      auto& signature_node{_selection->as<ast::function_placeholder>()};
      fetch_parameters(signature, signature_node);
    }
    return signature;
  }

  template <pasteboard_t node_type>
  [[nodiscard]] bool is() const {
    return _selection->inherits<typename details::ast_tag<node_type>::type>();
  }

  [[nodiscard]] bool is_function_signature() const {
    return _selection->is<ast::function_signature>() ||
           _selection->is<ast::function_placeholder>();
  }

  [[nodiscard]] bool is_literal() const {
    return _selection->is<ast::variable_placeholder>() ||
           _selection->is<ast::expression_placeholder>() ||
           _selection->is<ast::variable_name>() ||
           _selection->is<ast::number_literal>() ||
           _selection->is<ast::string_literal>() ||
           _selection->is<ast::identifier>();
  }

  [[nodiscard]] bool is_removable() const {
    return dragging_type().has_value();
  }

  [[nodiscard]] source_selection as_dragging_selection() const&& {
    return {*_doc, _loc, *_selection, dragging_rule};
  }

  [[nodiscard]] std::optional<pasteboard_t> dragging_type() const {
    if (is<pasteboard_t::block>()) {
      if (_selection->is<ast::on_start>()) {
        return std::nullopt;
      } else {
        return pasteboard_t::block;
      }
    } else if (is<pasteboard_t::statement>()) {
      return pasteboard_t::statement;
    } else if (is<pasteboard_t::reference>()) {
      // Must check reference before expression, because ast::identifier etc.
      // are both, and needs to be considered as reference
      return pasteboard_t::reference;
    } else if (is<pasteboard_t::expression>()) {
      return pasteboard_t::expression;
    } else {
      return std::nullopt;
    }
  }

  template <pasteboard_t node_type, typename = expr_enable_t<node_type>>
  [[nodiscard]] expr_inserter<node_type> as_inserter() const&& {
    if constexpr (node_type == pasteboard_t::expression) {
      assert(_selection->is<ast::expression_placeholder>() ||
             _selection->is<ast::number_literal>() ||
             _selection->is<ast::string_literal>() ||
             _selection->is<ast::identifier>());
    } else if constexpr (node_type == pasteboard_t::reference) {
      // is_literal includes the case of being placeholders
      assert(is_literal());
    } else {
      // Should not happen
      static_assert(details::dependent_false<node_type>::value);
    }
    return {*_doc, _loc, *_selection};
  }

  // Use this to update source_inserters, so that we can remove first and then
  // insert when moving parts of the code
  [[nodiscard]] line_update removal_line_update() const {
    assert(dragging_type().has_value());

    if (is<pasteboard_t::block>() || is<pasteboard_t::statement>()) {
      const auto range{_selection->source_code_range};
      return {range.end.line + 1, range.begin.line - range.end.line - 1};
    } else {
      return {};
    }
  }

  std::vector<source_update> remove_from_document() const&&;

  source_update insert_literal(literal_data_type type,
                               std::string_view literal) const&& {
    switch (type) {
      case literal_data_type::variable_name:
        [[fallthrough]];
      case literal_data_type::identifier:
        return std::move(*this)
            .as_inserter<pasteboard_t::reference>()
            .insert_literal(type, std::move(literal));
      case literal_data_type::number:
        [[fallthrough]];
      case literal_data_type::string:
        return std::move(*this)
            .as_inserter<pasteboard_t::expression>()
            .insert_literal(type, std::move(literal));
    }
  }

  std::vector<source_update> replace_function_signature(
      function_definition signature) const&&;

 private:
  document* _doc;
  source_loc _loc;
  ast::base* _selection;

  template <typename node_type>
  static void fetch_parameters(function_definition& signature,
                               const node_type& node) {
    for (auto& child : node.parameters()) {
      if (child->template is<ast::variable_name>()) {
        auto& variable{child->template as<ast::variable_name>()};
        signature.parameters.emplace_back(variable.name);
      } else {
        // Should not occur!
        assert(false);
      }
    }
  }

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
    assert(is<pasteboard_t::block>() || is<pasteboard_t::statement>());
    assert(_selection->has_parent());

    source_range line_range{{_selection->source_code_range.begin.line, 1},
                            {_selection->source_code_range.end.line + 1, 1}};
    auto line_offset{static_cast<ptrdiff_t>(line_range.begin.line) -
                     static_cast<ptrdiff_t>(line_range.end.line)};
    _doc->update_source_line_after_node(*_selection, line_offset);
    _doc->remove_line(*_selection);
    return source_update{line_range, {"", {}}};
  }

  source_update remove_expression() const&&;
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
