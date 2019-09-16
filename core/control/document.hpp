#ifndef marlin_control_document_hpp
#define marlin_control_document_hpp

#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include "base.hpp"
#include "generator.hpp"
#include "prototypes.hpp"
#include "source_update.hpp"
#include "store.hpp"
#include "toolbox.hpp"
#include "user_function.hpp"

namespace marlin::control {

template <typename node_type>
using enable_if_ast_t =
    std::enable_if_t<std::is_base_of_v<ast::base, node_type> &&
                     !std::is_same_v<ast::base, node_type>>;

enum struct line_node_type : uint8_t;

struct document {
  template <line_node_type node_type>
  friend struct line_inserter;
  friend struct expression_inserter;
  friend struct source_selection;

  static store::data_view default_data() {
    static const store::data_vector _data{[]() {
      std::vector<ast::node> blocks;
      blocks.emplace_back(ast::make<ast::on_start>(std::vector<ast::node>{}));
      auto node{ast::make<ast::program>(std::move(blocks))};
      return store::write({node.get()});
    }()};
    return _data;
  }

  static std::optional<std::pair<document, source_update>> make_document(
      store::data_view data = default_data()) {
    try {
      user_function_table table;
      auto result{store::read(data, table, store::type_expectation::program)};
      assert(result.nodes.size() == 1);
      return std::make_pair(
          document{std::move(result.nodes[0]), std::move(table)},
          source_update{{{1, 1}, {1, 1}}, std::move(result.display)});
    } catch (const store::read_error&) {
      return std::nullopt;
    }
  }

  explicit document(ast::node program, user_function_table table) noexcept
      : _program(std::move(program)), _functions{std::move(table)} {}

  void register_toolbox(std::weak_ptr<toolbox> model) {
    _functions.set_toolbox(std::move(model));
  }

  [[nodiscard]] ast::base& locate(source_loc loc) {
    return _program->locate(loc);
  }
  [[nodiscard]] const ast::base& locate(source_loc loc) const {
    return _program->locate(loc);
  }

  [[nodiscard]] std::string generate_executable_code() {
    exec::generator gen;
    return gen.generate(*_program);
  }

  store::data_vector write() const { return store::write({_program.get()}); }

  auto& functions() { return _functions.map(); }

 private:
  ast::node _program;
  user_function_table _functions;

  // Convenient functions to modify _program
  // Implemented for use of friend structs

  ast::node replace_expression(ast::base& existing, ast::node replacement) {
    assert(existing.has_parent());

    // assume there are no multi-line expressions
    assert(existing.source_code_range.end.line ==
           replacement->source_code_range.end.line);

    auto offset{
        static_cast<ptrdiff_t>(replacement->source_code_range.end.column) -
        static_cast<ptrdiff_t>(existing.source_code_range.end.column)};
    auto& placed{*replacement};

    auto result{
        existing.parent().replace_child(existing, std::move(replacement))};
    if (offset != 0) {
      update_source_column_after_node(placed, offset);
    }
    return result;
  }

  ast::node remove_line(ast::base& target) {
    // Statement must be in a vector of statements
    assert(target.has_parent());

    return target.parent().apply<ast::node>([this, &target](auto& n) {
      return remove_line_from_parent<0>(n, target);
    });
  }

  template <size_t index, typename node_type, typename... subnode_type>
  ast::node remove_line_from_parent(
      ast::base::impl<node_type, subnode_type...>& parent, ast::base& target) {
    if constexpr (index < sizeof...(subnode_type)) {
      if (auto result{try_remove_line_from_subnode(
              parent.template get_subnode<index>(), target)}) {
        return *std::move(result);
      } else {
        return remove_line_from_parent<index + 1>(parent, target);
      }
    } else {
      assert(false);
      return ast::make<ast::program>(std::vector<ast::node>{});
    }
  }

  std::optional<ast::node> try_remove_line_from_subnode(
      ast::subnode::concrete_view<ast::base>, ast::base&) {
    return std::nullopt;
  }

  std::optional<ast::node> try_remove_line_from_subnode(
      ast::subnode::vector_view<ast::base> view, ast::base& target) {
    for (size_t i{0}; i < view.size(); i++) {
      if (view[i].get() == &target) {
        return view.pop(i);
      }
    }
    return std::nullopt;
  }

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
            update_source_column(*child, column_offset);
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

  void add_function(function_definition signature) {
    _functions.add_function(std::move(signature));
    // Adding functions must not have any side effect on the document
    // This is required so that the store module can read functions properly
  }

  void replace_function(const std::string& name,
                        function_definition new_signature) {
    _functions.replace_function(name, std::move(new_signature));

    // TODO: Change corresponding call signatures
  }

  void remove_function(const std::string& name) {
    _functions.remove_function(name);

    // TODO: Change corresponding call signatures
  }
};

}  // namespace marlin::control

#endif  // marlin_control_document_hpp
