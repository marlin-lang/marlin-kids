#ifndef marlin_control_source_selection_hpp
#define marlin_control_source_selection_hpp

#include "ast.hpp"
#include "document.hpp"
#include "expression_inserter.hpp"

namespace marlin::control {

struct source_selection {
  struct literal_content {
    literal_data_type type;
    std::string content;

    literal_content(literal_data_type _type, std::string _content)
        : type{_type}, content{std::move(_content)} {}
  };

  source_selection(document& doc, source_loc loc)
      : _doc{&doc}, _loc{loc}, _selection{&_doc->locate(loc)} {}

  source_range get_range() const noexcept {
    return _selection->source_code_range;
  }

  bool is_literal() const {
    return _selection->is<marlin::ast::variable_placeholder>() ||
           _selection->is<marlin::ast::expression_placeholder>() ||
           _selection->is<marlin::ast::variable_name>() ||
           _selection->is<marlin::ast::number_literal>() ||
           _selection->is<marlin::ast::string_literal>() ||
           _selection->is<marlin::ast::identifier>();
  }
  literal_content get_literal_content() const {
    assert(is_literal());
    return _selection->apply<literal_content>(
        [this](const auto& n) { return get_literal_content(n); });
  }

  expression_inserter as_expression_inserter() const&& {
    // For now only placeholders and literals can be directly overridden
    assert(is_literal());
    return {*_doc, _loc, *_selection};
  }

 private:
  document* _doc;
  source_loc _loc;
  ast::base* _selection;

  template <typename node_type>
  literal_content get_literal_content(const node_type& node) const {
    // Not literal
    assert(false);
    return {literal_data_type::number, ""};
  }
};

template <>
inline source_selection::literal_content
source_selection::get_literal_content<ast::variable_placeholder>(
    const ast::variable_placeholder& node) const {
  return {literal_data_type::variable_name, ""};
}

template <>
inline source_selection::literal_content
source_selection::get_literal_content<ast::variable_name>(
    const ast::variable_name& node) const {
  return {literal_data_type::variable_name, node.name};
}

template <>
inline source_selection::literal_content
source_selection::get_literal_content<ast::expression_placeholder>(
    const ast::expression_placeholder& node) const {
  return {literal_data_type::number, ""};
}

template <>
inline source_selection::literal_content
source_selection::get_literal_content<ast::number_literal>(
    const ast::number_literal& node) const {
  return {literal_data_type::number, node.value};
}

template <>
inline source_selection::literal_content
source_selection::get_literal_content<ast::string_literal>(
    const ast::string_literal& node) const {
  return {literal_data_type::string, node.value};
}

template <>
inline source_selection::literal_content
source_selection::get_literal_content<ast::identifier>(
    const ast::identifier& node) const {
  return {literal_data_type::identifier, node.name};
}

};  // namespace marlin::control

#endif  // marlin_control_source_selection_hpp
