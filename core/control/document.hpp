#ifndef marlin_control_document_hpp
#define marlin_control_document_hpp

#include <optional>
#include <string>
#include <utility>

// Testing
#include <iostream>

#include "base.hpp"
#include "prototypes.hpp"
#include "source_modifications.hpp"

namespace marlin::control {

template <typename node_type>
using enable_if_ast_t =
    std::enable_if_t<std::is_base_of_v<ast::base, node_type> &&
                     !std::is_same_v<ast::base, node_type>>;

struct document {
  struct statement_insert_location {
    ast::base* parent;
    ast::subnode::vector_view<ast::base> block;
    size_t index;
    size_t line;
    size_t indent;

    statement_insert_location(ast::base& _parent,
                              ast::subnode::vector_view<ast::base> _block,
                              size_t _index, size_t _line, size_t _indent)
        : parent{&_parent},
          block{_block},
          index{_index},
          line{_line},
          indent{_indent} {}
  };

  static std::pair<document, source_initialization> make_document(
      const char* data, size_t size) {
    // TODO: parse data
    std::vector<ast::node> statements;
    std::vector<ast::node> blocks;

    blocks.emplace_back(ast::make<ast::on_start>(std::move(statements)));
    blocks[0]->source_code_range = {{1, 1}, {2, 1}};
    auto program = ast::make<ast::program>(std::move(blocks));
    program->source_code_range = {{1, 1}, {2, 1}};
    return {document{std::move(program)},
            source_initialization{
                "on start {\n}\n",
                {highlight_token{highlight_token_type::keyword, 0, 8}}}};
  }

  explicit document(ast::node program) noexcept
      : _program(std::move(program)) {}

  std::optional<statement_insert_location> find_statement_insert_location(
      size_t line) {
    return find_statement_insert_location_in_node(line, *_program, 0);
  }

  source_insertion insert_statement(statement_insert_location loc,
                                    const statement_prototype& prototype) {
    auto [node, source_update]{prototype.construct(loc.line, loc.indent)};
    auto line_offset{static_cast<ptrdiff_t>(node->source_code_range.end.line) +
                     1 - static_cast<ptrdiff_t>(loc.line)};

    loc.block.emplace(loc.index, std::move(node));

    update_source_line_after_node(*loc.block[loc.index], line_offset);

    return source_update;
  }

  const auto& output() const noexcept { return _output; }

  void execute() { _output.clear(); }

 private:
  ast::node _program;
  std::string _output;

  std::optional<statement_insert_location>
  find_statement_insert_location_in_node(size_t line, ast::base& node,
                                         size_t current_indent) {
    return node.apply<std::optional<statement_insert_location>>(
        [this, &line, &current_indent](auto& node) {
          return find_statement_insert_location_in_node(line, node,
                                                        current_indent);
        });
  }

  template <typename node_type, typename = enable_if_ast_t<node_type>>
  std::optional<statement_insert_location>
  find_statement_insert_location_in_node(size_t line, node_type& node,
                                         size_t current_indent) {
    return std::nullopt;
  }

  std::optional<statement_insert_location>
  find_statement_insert_location_in_node(size_t line, ast::program& node,
                                         size_t current_indent) {
    return find_statement_insert_location_in_vector<false>(
        line, node, node.blocks(), current_indent);
  }

  std::optional<statement_insert_location>
  find_statement_insert_location_in_node(size_t line, ast::on_start& node,
                                         size_t current_indent) {
    return find_statement_insert_location_in_vector<true>(
        line, node, node.statements(), current_indent);
  }

  std::optional<statement_insert_location>
  find_statement_insert_location_in_node(size_t line, ast::if_statement& node,
                                         size_t current_indent) {
    return find_statement_insert_location_in_vector<true>(
        line, node, node.statements(), current_indent);
  }

  std::optional<statement_insert_location>
  find_statement_insert_location_in_node(size_t line,
                                         ast::if_else_statement& node,
                                         size_t current_indent) {
    if (line <= node.else_loc.line) {
      return find_statement_insert_location_in_vector<true>(
          line, node, node.consequence(), current_indent);
    } else {
      return find_statement_insert_location_in_vector<true>(
          line, node, node.alternate(), current_indent);
    }
  }

  template <bool vector_is_block>
  std::optional<statement_insert_location>
  find_statement_insert_location_in_vector(
      size_t line, ast::base& parent,
      ast::subnode::vector_view<ast::base> vector, size_t current_indent) {
    for (size_t i{0}; i < vector.size(); i++) {
      if (line <= vector[i]->source_code_range.begin.line) {
        if constexpr (vector_is_block) {
          return statement_insert_location{parent, vector, i, line,
                                           current_indent};
        } else {
          return std::nullopt;
        }
      } else if (line <= vector[i]->source_code_range.end.line) {
        return find_statement_insert_location_in_node(line, *vector[i],
                                                      current_indent + 1);
      }
    }
    if constexpr (vector_is_block) {
      return statement_insert_location{parent, vector, vector.size(), line,
                                       current_indent};
    } else {
      return std::nullopt;
    }
  }

  void update_source_line_after_node(ast::base& node, ptrdiff_t line_offset) {
    auto* curr{&node.parent()};
    auto* target{&node};
    while (true) {
      update_source_line_after_subnode(*curr, *target, line_offset);
      if (curr->has_parent()) {
        target = curr;
        curr = &curr->parent();
      } else {
        return;
      }
    }
  }

  void update_source_line_after_subnode(ast::base& node, ast::base& subnode,
                                        ptrdiff_t line_offset) {
    node.apply<void>([this, &subnode, &line_offset](auto& n) {
      update_source_line_after_subnode(n, subnode, line_offset);
    });
  }

  template <typename node_type, typename = enable_if_ast_t<node_type>>
  void update_source_line_after_subnode(node_type& node, ast::base& subnode,
                                        ptrdiff_t line_offset) {
    bool target_passed{false};
    for (auto& child : node.children()) {
      if (child.get() == &subnode) {
        target_passed = true;
      } else if (target_passed) {
        update_source_line(*child, line_offset);
      }
    }
    node.source_code_range.end.line += line_offset;
  }

  void update_source_line_after_subnode(ast::if_else_statement& node,
                                        ast::base& subnode,
                                        ptrdiff_t line_offset) {
    bool target_passed{false};
    update_source_line_in_vector_after_subnode(node.consequence(), subnode,
                                               line_offset, target_passed);
    if (target_passed) {
      node.else_loc.line += line_offset;
      update_source_line_in_vector(node.alternate(), line_offset);
    } else {
      update_source_line_in_vector_after_subnode(node.alternate(), subnode,
                                                 line_offset, target_passed);
    }
    node.source_code_range.end.line += line_offset;
  }

  template <typename vector_type>
  void update_source_line_in_vector_after_subnode(vector_type&& vector,
                                                  ast::base& subnode,
                                                  ptrdiff_t line_offset,
                                                  bool& found) {
    assert(!found);
    for (auto& child : vector) {
      if (child.get() == &subnode) {
        found = true;
      } else if (found) {
        update_source_line(*child, line_offset);
      }
    }
  }

  void update_source_line(ast::base& node, ptrdiff_t line_offset) {
    node.apply<void>(
        [this, &line_offset](auto& n) { update_source_line(n, line_offset); });
  }

  template <typename node_type, typename = enable_if_ast_t<node_type>>
  void update_source_line(node_type& node, ptrdiff_t line_offset) {
    node.source_code_range.begin.line += line_offset;
    update_source_line_in_vector(node.children(), line_offset);
    node.source_code_range.end.line += line_offset;
  }

  void update_source_line(ast::if_else_statement& node, ptrdiff_t line_offset) {
    node.source_code_range.begin.line += line_offset;
    node.else_loc.line += line_offset;
    update_source_line_in_vector(node.children(), line_offset);
    node.source_code_range.end.line += line_offset;
  }

  template <typename vector_type>
  void update_source_line_in_vector(vector_type&& vector,
                                    ptrdiff_t line_offset) {
    for (auto& child : vector) {
      update_source_line(*child, line_offset);
    }
  }
};

}  // namespace marlin::control

#endif  // marlin_control_document_hpp
