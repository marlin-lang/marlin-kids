#ifndef marlin_control_source_selection_hpp
#define marlin_control_source_selection_hpp

#include <optional>
#include <utility>

#include "ast.hpp"
#include "document.hpp"
#include "expression_inserter.hpp"
#include "placeholders.hpp"
#include "store.hpp"

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

  bool is_statement() const { return _selection->inherits<ast::statement>(); }
  bool is_expression() const {
    return is_literal() || _selection->inherits<ast::expression>();
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

  store::data_vector get_data() const { return store::write({_selection}); }

  std::optional<source_update> remove_from_document() const {
    if (is_statement()) {
      return remove_statement();
    } else if (is_expression()) {
      return remove_expression();
    } else {
      // For now we are not dragging blocks
      assert(false);
      return std::nullopt;
    }
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

  source_update remove_statement() const {
    assert(_selection->has_parent());

    source_range statement_range{
        {_selection->source_code_range.begin.line, 1},
        {_selection->source_code_range.end.line + 1, 1}};
    auto line_offset{static_cast<ptrdiff_t>(statement_range.begin.line) -
                     static_cast<ptrdiff_t>(statement_range.end.line)};
    _doc->update_source_line_after_node(*_selection, line_offset);
    _doc->remove_line(*_selection);
    return source_update{statement_range, "", {}};
  }

  source_update remove_expression() const {
    assert(_selection->has_parent());

    auto placeholder_name{placeholder::get_replacing_node(*_selection)};
    std::string source{"@"};
    source.append(placeholder_name);
    auto placeholder{ast::make<ast::expression_placeholder>(
        std::string{std::move(placeholder_name)})};
    auto original_range{_selection->source_code_range};
    placeholder->source_code_range = {
        original_range.begin,
        {original_range.begin.line,
         original_range.begin.column + source.size()}};
    _doc->replace_expression(*_selection, std::move(placeholder));
    return source_update{
        original_range,
        std::move(source),
        {highlight_token{highlight_token_type::placeholder, 0, source.size()}}};
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
