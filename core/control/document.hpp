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

struct document {
  struct statement_insert_location {
    ast::base& parent;
    ast::subnode::vector_view<ast::base> block;
    size_t index;
    size_t line;

    statement_insert_location(ast::base& _parent,
                              ast::subnode::vector_view<ast::base> _block,
                              size_t _index, size_t _line)
        : parent{_parent}, block{_block}, index{_index}, line{_line} {}
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
    return find_statement_insert_location_in_node(line, *_program);
  }

  source_insertion insert_statement(statement_insert_location loc,
                                    const statement_prototype& prototype) {
    auto indent{0};
    auto* curr{&loc.parent};
    while (curr->has_parent()) {
      indent += 1;
      curr = &curr->parent();
    }

    auto [node, source_update]{prototype.construct(loc.line, indent)};
    auto line_offset{static_cast<long>(node->source_code_range.end.line) + 1 -
                     static_cast<long>(loc.line)};

    loc.block.emplace(loc.index, std::move(node));

    // Update statements below insert location
    curr = &loc.parent;
    auto* target{loc.block[loc.index].get()};
    while (curr) {
      bool target_passed{false};
      for (auto& child : curr->children()) {
        if (child.get() == target) {
          target_passed = true;
        } else if (target_passed) {
          update_source_line(*child, line_offset);
        }
      }
      curr->source_code_range.end.line += line_offset;
      target = curr;
      curr = &curr->parent();
    }

    return source_update;
  }

  const auto& output() const noexcept { return _output; }

  void execute() { _output.clear(); }

 private:
  ast::node _program;
  std::string _output;

  std::optional<statement_insert_location>
  find_statement_insert_location_in_node(size_t line, ast::base& node) {
    return node.apply<std::optional<statement_insert_location>>(
        [this, &line](auto& node) {
          return find_statement_insert_location_in_node(line, node);
        });
  }

  template <typename node_type, typename = std::enable_if_t<
                                    std::is_base_of_v<ast::base, node_type> &&
                                    !std::is_same_v<ast::base, node_type>>>
  std::optional<statement_insert_location>
  find_statement_insert_location_in_node(size_t line, node_type& node) {
    return std::nullopt;
  }

  std::optional<statement_insert_location>
  find_statement_insert_location_in_node(size_t line, ast::program& node) {
    return find_statement_insert_location_in_vector<false>(line, node,
                                                           node.blocks());
  }

  std::optional<statement_insert_location>
  find_statement_insert_location_in_node(size_t line, ast::on_start& node) {
    return find_statement_insert_location_in_vector<true>(line, node,
                                                          node.statements());
  }

  template <bool vector_is_block>
  std::optional<statement_insert_location>
  find_statement_insert_location_in_vector(
      size_t line, ast::base& parent,
      ast::subnode::vector_view<ast::base> vector) {
    for (size_t i = 0; i < vector.size(); i++) {
      if (line <= vector[i]->source_code_range.begin.line) {
        if constexpr (vector_is_block) {
          return statement_insert_location{parent, vector, i, line};
        } else {
          return std::nullopt;
        }
      } else if (line <= vector[i]->source_code_range.end.line) {
        return find_statement_insert_location_in_node(line, *vector[i]);
      }
    }
    if constexpr (vector_is_block) {
      return statement_insert_location{parent, vector, vector.size(), line};
    } else {
      return std::nullopt;
    }
  }

  void update_source_line(ast::base& node, long line_offset) {
    node.source_code_range.begin.line += line_offset;
    for (auto& child : node.children()) {
      update_source_line(*child, line_offset);
    }
    node.source_code_range.end.line += line_offset;
  }
};

}  // namespace marlin::control

#endif  // marlin_control_document_hpp
