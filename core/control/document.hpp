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
  friend struct statement_inserter;

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

  [[nodiscard]] ast::base& locate(source_loc loc) {
    return _program->locate(loc);
  }
  [[nodiscard]] const ast::base& locate(source_loc loc) const {
    return _program->locate(loc);
  }

  const auto& output() const noexcept { return _output; }

  void execute() { _output.clear(); }

 private:
  ast::node _program;
  std::string _output;

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
