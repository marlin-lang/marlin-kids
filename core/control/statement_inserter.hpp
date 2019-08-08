#ifndef marlin_control_statement_inserter_hpp
#define marlin_control_statement_inserter_hpp

#include "document.hpp"

namespace marlin::control {

struct statement_inserter {
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

  statement_inserter(document& doc) : _doc{&doc} {}

  bool can_insert() const noexcept { return _loc.has_value(); }
  source_loc get_insert_location() const noexcept {
    return {_loc->line, _loc->indent * indent_space_count};
  }

  void move_to_line(size_t line) {
    if (_line == 0 || line != _line) {
      _loc = find_statement_insert_location_in_node(line, *_doc->_program, 0);
      _line = line;
    }
  }

  source_insertion insert(const statement_prototype& prototype) {
    assert(_loc.has_value());
    auto [node, source_update]{prototype.construct(_loc->line, _loc->indent)};
    auto line_offset{static_cast<ptrdiff_t>(node->source_code_range.end.line) +
                     1 - static_cast<ptrdiff_t>(_loc->line)};

    _loc->block.emplace(_loc->index, std::move(node));

    _doc->update_source_line_after_node(*_loc->block[_loc->index], line_offset);

    return source_update;
  }

 private:
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

  template <bool vector_is_block>
  std::optional<location> find_statement_insert_location_in_vector(
      size_t line, ast::base& parent,
      ast::subnode::vector_view<ast::base> vector, size_t current_indent) {
    for (size_t i{0}; i < vector.size(); i++) {
      if (line <= vector[i]->source_code_range.begin.line) {
        if constexpr (vector_is_block) {
          return location{parent, vector, i, line, current_indent};
        } else {
          return std::nullopt;
        }
      } else if (line <= vector[i]->source_code_range.end.line) {
        return find_statement_insert_location_in_node(line, *vector[i],
                                                      current_indent + 1);
      }
    }
    if constexpr (vector_is_block) {
      return location{parent, vector, vector.size(), line, current_indent};
    } else {
      return std::nullopt;
    }
  }
};

}  // namespace marlin::control

#endif  // marlin_control_statement_inserter_hpp