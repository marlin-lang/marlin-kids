#ifndef marlin_control_statement_inserter_hpp
#define marlin_control_statement_inserter_hpp

#include <optional>
#include <vector>

#include "document.hpp"
#include "prototypes.hpp"
#include "source_selection.hpp"

namespace marlin::control {

static constexpr size_t indent_space_count = 2;

enum struct line_node_type : uint8_t { block = 0, statement = 1 };

template <line_node_type node_type>
struct line_inserter {
  line_inserter(document& doc) : _doc{&doc} {}

  bool can_insert() const noexcept { return _loc.has_value(); }
  source_loc get_insert_location() const noexcept {
    assert(_loc.has_value());
    return {_loc->line, _loc->indent * indent_space_count + 1};
  }

  void move_to_loc(source_loc loc,
                   const source_selection* exclusion = nullptr) {
    move_to_line(loc.line, exclusion);
  }

  void move_to_line(size_t line, const source_selection* exclusion = nullptr) {
    if (_line == 0 || line != _line) {
      if (exclusion != nullptr) {
        const auto range{exclusion->get_range()};
        if (line > range.begin.line && line <= range.end.line) {
          _loc = std::nullopt;
          _line = line;
          return;
        }
      }
      _loc = find_insert_location_in_base(line, *_doc->_program, 0);
      _line = line;
    }
  }

  std::vector<source_update> insert(store::data_view data);

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

  std::optional<location> find_insert_location_in_base(size_t line,
                                                       ast::base& node,
                                                       size_t current_indent) {
    return node.apply<std::optional<location>>(
        [this, &line, &current_indent](auto& node) {
          return this->find_insert_location_in_node(line, node, current_indent);
        });
  }

  template <typename container_type, typename = enable_if_ast_t<container_type>>
  std::optional<location> find_insert_location_in_node(size_t line,
                                                       container_type& node,
                                                       size_t current_indent) {
    // Impossible for insert location to be here
    assert(false);
    return std::nullopt;
  }

  std::optional<location> find_insert_location_in_node(size_t line,
                                                       ast::program& node,
                                                       size_t current_indent) {
    return find_insert_location_in_vector<line_node_type::block>(
        line, node, node.blocks(), current_indent);
  }

  std::optional<location> find_insert_location_in_node(size_t line,
                                                       ast::on_start& node,
                                                       size_t current_indent) {
    return find_insert_location_in_vector<line_node_type::statement>(
        line, node, node.statements(), current_indent);
  }

  std::optional<location> find_insert_location_in_node(size_t line,
                                                       ast::function& node,
                                                       size_t current_indent) {
    return find_insert_location_in_vector<line_node_type::statement>(
        line, node, node.statements(), current_indent);
  }

  std::optional<location> find_insert_location_in_node(size_t line,
                                                       ast::if_statement& node,
                                                       size_t current_indent) {
    return find_insert_location_in_vector<line_node_type::statement>(
        line, node, node.statements(), current_indent);
  }

  std::optional<location> find_insert_location_in_node(
      size_t line, ast::if_else_statement& node, size_t current_indent) {
    if (line <= node.else_loc.line) {
      return find_insert_location_in_vector<line_node_type::statement>(
          line, node, node.consequence(), current_indent);
    } else {
      return find_insert_location_in_vector<line_node_type::statement>(
          line, node, node.alternate(), current_indent);
    }
  }

  std::optional<location> find_insert_location_in_node(
      size_t line, ast::while_statement& node, size_t current_indent) {
    return find_insert_location_in_vector<line_node_type::statement>(
        line, node, node.statements(), current_indent);
  }

  std::optional<location> find_insert_location_in_node(size_t line,
                                                       ast::for_statement& node,
                                                       size_t current_indent) {
    return find_insert_location_in_vector<line_node_type::statement>(
        line, node, node.statements(), current_indent);
  }

  template <line_node_type element_type>
  std::optional<location> find_insert_location_in_vector(
      size_t line, ast::base& parent,
      ast::subnode::vector_view<ast::base> vector, size_t current_indent);
};

using block_inserter = line_inserter<line_node_type::block>;
using statement_inserter = line_inserter<line_node_type::statement>;

}  // namespace marlin::control

#endif  // marlin_control_statement_inserter_hpp
