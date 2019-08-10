#ifndef marlin_control_document_hpp
#define marlin_control_document_hpp

#include <optional>
#include <string>
#include <utility>

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

  source_replacement replace_expression_with_number_literal(
      ast::base& expression, std::string number) {
    source_range original{expression.source_code_range};
    auto element{ast::make<ast::number_literal>(number)};
    element->source_code_range = {
        {original.begin.line, original.begin.column},
        {original.begin.line, original.begin.column + number.size()}};
    replace_expression(expression, std::move(element));

    std::vector<highlight_token> highlights{
        {highlight_token_type::number, 0, number.size()}};
    return {original, std::move(number), highlights};
  }

  source_replacement replace_expression_with_string_literal(
      ast::base& expression, std::string string) {
    source_range original{expression.source_code_range};
    auto quoted_string = quoted(string);
    auto element{ast::make<ast::string_literal>(std::move(string))};
    element->source_code_range = {
        {original.begin.line, original.begin.column},
        {original.begin.line, original.begin.column + quoted_string.size()}};
    replace_expression(expression, std::move(element));

    std::vector<highlight_token> highlights{
        {highlight_token_type::string, 0, quoted_string.size()}};
    return {original, std::move(quoted_string), highlights};
  }

  ast::node replace_expression(ast::base& existing, ast::node replacement) {
    assert(existing.has_parent());
    assert(existing.source_code_range.end.line ==
           replacement->source_code_range.end.line);

    auto offset =
        static_cast<ptrdiff_t>(replacement->source_code_range.end.column) -
        static_cast<ptrdiff_t>(existing.source_code_range.end.column);
    auto& placed = *replacement;

    auto result{
        existing.parent().replace_child(existing, std::move(replacement))};
    if (offset != 0) {
      update_source_column_after_node(placed, offset);
    }
    return result;
  }

  const auto& output() const noexcept { return _output; }

  void execute() { _output.clear(); }

 private:
  ast::node _program;
  std::string _output;

  void update_source_column_after_node(ast::base& node,
                                       ptrdiff_t column_offset) {
    auto* curr{&node};
    while (curr->has_parent()) {
      auto* target{curr};
      curr = &curr->parent();
      bool target_passed{false};
      for (auto& child : curr->children()) {
        if (child.get() == target) {
          target_passed = true;
        } else if (target_passed) {
          if (child->source_code_range.begin.line ==
              node.source_code_range.begin.line) {
            update_source_column(*child, target_passed);
          } else {
            break;
          }
        }
      }
      curr->source_code_range.end.column += column_offset;
      if (curr->inherits<ast::statement>()) {
        break;
      }
    }
  }

  void update_source_column(ast::base& node, ptrdiff_t column_offset) {
    node.source_code_range.begin.column += column_offset;
    for (auto& child : node.children()) {
      update_source_column(*child, column_offset);
    }
    node.source_code_range.end.column += column_offset;
  }

  void update_source_line_after_node(ast::base& node, ptrdiff_t line_offset) {
    auto* curr{&node};
    while (curr->has_parent()) {
      auto* target{curr};
      curr = &curr->parent();
      update_source_line_after_subnode(*curr, *target, line_offset);
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
    update_source_line_in_vector_after_subnode(node.children(), subnode,
                                               line_offset, target_passed);
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
