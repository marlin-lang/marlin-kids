#ifndef marlin_control_statement_inserter_hpp
#define marlin_control_statement_inserter_hpp

#include <optional>

#include "document.hpp"
#include "prototypes.hpp"

namespace marlin::control {

struct statement_inserter {
  statement_inserter(document& doc) : _doc{&doc} {}

  bool can_insert() const noexcept { return _loc.has_value(); }
  source_loc get_location() const noexcept {
    assert(_loc.has_value());
    return {_loc->line, _loc->indent * indent_space_count + 1};
  }

  void move_to_line(size_t line) {
    if (_line == 0 || line != _line) {
      _loc = find_statement_insert_location_in_node(line, *_doc->_program, 0);
      _line = line;
    }
  }

  std::optional<source_insertion> insert(store::data_view data);

 private:
  struct location {
    ast::base* parent;
    ast::subnode::vector_view<ast::base> block;
    size_t index;
    size_t line;
    size_t indent;

    location(ast::base& _parent, ast::subnode::vector_view<ast::base> _block,
             size_t _index, size_t _line, size_t _indent)
        : parent{&_parent},
          block{_block},
          index{_index},
          line{_line},
          indent{_indent} {}
  };

  document* _doc;
  size_t _line{0};
  std::optional<location> _loc;

  std::optional<location> find_statement_insert_location_in_node(
      size_t line, ast::base& node, size_t current_indent) {
    return node.apply<std::optional<location>>([this, &line,
                                                &current_indent](auto& node) {
      return find_statement_insert_location_in_node(line, node, current_indent);
    });
  }

  template <typename node_type, typename = enable_if_ast_t<node_type>>
  std::optional<location> find_statement_insert_location_in_node(
      size_t line, node_type& node, size_t current_indent) {
    return std::nullopt;
  }

  std::optional<location> find_statement_insert_location_in_node(
      size_t line, ast::program& node, size_t current_indent) {
    return find_statement_insert_location_in_vector<false>(
        line, node, node.blocks(), current_indent);
  }

  std::optional<location> find_statement_insert_location_in_node(
      size_t line, ast::on_start& node, size_t current_indent) {
    return find_statement_insert_location_in_vector<true>(
        line, node, node.statements(), current_indent);
  }

  std::optional<location> find_statement_insert_location_in_node(
      size_t line, ast::if_statement& node, size_t current_indent) {
    return find_statement_insert_location_in_vector<true>(
        line, node, node.statements(), current_indent);
  }

  std::optional<location> find_statement_insert_location_in_node(
      size_t line, ast::if_else_statement& node, size_t current_indent) {
    if (line <= node.else_loc.line) {
      return find_statement_insert_location_in_vector<true>(
          line, node, node.consequence(), current_indent);
    } else {
      return find_statement_insert_location_in_vector<true>(
          line, node, node.alternate(), current_indent);
    }
  }

  std::optional<location> find_statement_insert_location_in_node(
      size_t line, ast::while_statement& node, size_t current_indent) {
    return find_statement_insert_location_in_vector<true>(
        line, node, node.statements(), current_indent);
  }

  std::optional<location> find_statement_insert_location_in_node(
      size_t line, ast::for_statement& node, size_t current_indent) {
    return find_statement_insert_location_in_vector<true>(
        line, node, node.statements(), current_indent);
  }

  template <bool vector_is_block>
  std::optional<location> find_statement_insert_location_in_vector(
      size_t line, ast::base& parent,
      ast::subnode::vector_view<ast::base> vector, size_t current_indent);
};

}  // namespace marlin::control

#endif  // marlin_control_statement_inserter_hpp
