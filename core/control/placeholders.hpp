#ifndef marlin_control_placeholders_hpp
#define marlin_control_placeholders_hpp

#include <array>
#include <optional>
#include <string>
#include <string_view>

#include "ast.hpp"
#include "node.hpp"

namespace marlin::control {

struct placeholder {
  static std::string empty() {
    static constexpr std::string_view _empty{""};
    static_assert(_empty.size() == 0);

    return std::string{_empty};
  }

  static bool is_empty(const std::string& str) { return str.size() == 0; }

  static std::string default_text() { return "value"; }

  template <typename node_type>
  static std::string get(ast::base::child_index) {
    return default_text();
  }

  template <typename node_type>
  static std::string get(const node_type& parent,
                         ast::base::child_index index) {
    return get<node_type>(std::move(index));
  }

  static std::string get_replacing_node(const ast::base& node) {
    if (node.has_parent()) {
      auto& parent{node.parent()};
      return parent.apply<std::string>(
          [&node](auto& n) { return get(n, n.index_for_child(node)); });
    } else {
      return default_text();
    }
  }
};

template <>
inline std::string placeholder::get<ast::function>(
    ast::base::child_index index) {
  assert(index.subnode_index == 0 && index.node_index == 0);
  return "name";
}

template <>
inline std::string placeholder::get<ast::eval_statement>(
    ast::base::child_index index) {
  assert(index.subnode_index == 0 && index.node_index == 0);
  return "expression";
}

template <>
inline std::string placeholder::get<ast::assignment>(
    ast::base::child_index index) {
  static constexpr std::array<std::string_view, 2> subnodes{"variable",
                                                            "value"};
  assert(index.node_index == 0);
  return std::string{subnodes[index.subnode_index]};
}

template <>
inline std::string placeholder::get<ast::use_global>(
    ast::base::child_index index) {
  assert(index.subnode_index == 0 && index.node_index == 0);
  return "variable";
}

inline std::string _get_if_placeholder(ast::base::child_index index) {
  assert(index.subnode_index == 0 && index.node_index == 0);
  return "condition";
}
template <>
inline std::string placeholder::get<ast::if_statement>(
    ast::base::child_index index) {
  return _get_if_placeholder(index);
}
template <>
inline std::string placeholder::get<ast::if_else_statement>(
    ast::base::child_index index) {
  return _get_if_placeholder(index);
}

template <>
inline std::string placeholder::get<ast::while_statement>(
    ast::base::child_index index) {
  assert(index.subnode_index == 0 && index.node_index == 0);
  return "condition";
}

template <>
inline std::string placeholder::get<ast::for_statement>(
    ast::base::child_index index) {
  static constexpr std::array<std::string_view, 2> subnodes{"variable",
                                                            "array"};
  assert(index.subnode_index < 2 && index.node_index == 0);
  return std::string{subnodes[index.subnode_index]};
}

template <>
inline std::string placeholder::get<ast::return_result_statement>(
    ast::base::child_index index) {
  assert(index.subnode_index == 0 && index.node_index == 0);
  return "result";
}

template <>
inline std::string placeholder::get<ast::unary_expression>(
    ast::base::child_index index) {
  assert(index.subnode_index == 0 && index.node_index == 0);
  return "argument";
}

template <>
inline std::string placeholder::get<ast::binary_expression>(
    ast::base::child_index index) {
  static constexpr std::array<std::string_view, 2> subnodes{"left", "right"};
  assert(index.subnode_index < subnodes.size() && index.node_index == 0);
  return std::string{subnodes[index.subnode_index]};
}

inline std::string _get_subscript_placeholder(ast::base::child_index index) {
  static constexpr std::array<std::string_view, 2> subnodes{"array", "index"};
  assert(index.subnode_index < subnodes.size() && index.node_index == 0);
  return std::string{subnodes[index.subnode_index]};
}
template <>
inline std::string placeholder::get<ast::subscript_set>(
    ast::base::child_index index) {
  return _get_subscript_placeholder(index);
}
template <>
inline std::string placeholder::get<ast::subscript_get>(
    ast::base::child_index index) {
  return _get_subscript_placeholder(index);
}

template <>
inline std::string placeholder::get<ast::new_array>(
    ast::base::child_index index) {
  assert(index.subnode_index == 0);
  return "elem" + std::to_string(index.node_index);
}

struct placeholder_system_procedure_args {
 private:
  static const auto& placeholders() {
    static const std::array _placeholders{
        std::vector<std::string_view>{"seconds"} /* sleep */,
        std::vector<std::string_view>{"message"} /* print */,
        std::vector<std::string_view>{"array", "element"} /* list_append */,
        std::vector<std::string_view>{"array", "index",
                                      "element"} /* list_insert */,
        std::vector<std::string_view>{"array", "index"} /* list_remove */,
        std::vector<std::string_view>{"start_x", "start_y", "end_x",
                                      "end_y"} /* draw_line */,
        std::vector<std::string_view>{"width"} /* set_line_width */,
        std::vector<std::string_view>{"length"} /* logo_forward */,
        std::vector<std::string_view>{"length"} /* logo_backward */,
        std::vector<std::string_view>{"degree"} /* logo_turn_left */,
        std::vector<std::string_view>{"degree"} /* logo_turn_right */,
        std::vector<std::string_view>{} /* logo_pen_up */,
        std::vector<std::string_view>{} /* logo_pen_down */
    };
    return _placeholders;
  }

 public:
  static const auto& args(ast::system_procedure func) {
    return placeholders()[static_cast<size_t>(func)];
  }
};

template <>
inline std::string placeholder::get<ast::system_procedure_call>(
    const ast::system_procedure_call& parent, ast::base::child_index index) {
  const auto& args{placeholder_system_procedure_args::args(parent.proc)};
  assert(index.subnode_index == 0 && index.node_index < args.size());
  return std::string{args[index.node_index]};
}

struct placeholder_system_function_args {
 private:
  static const auto& placeholders() {
    static const std::array _placeholders{
        std::vector<std::string_view>{"end"} /* range1 */,
        std::vector<std::string_view>{"begin", "end"} /* range2 */,
        std::vector<std::string_view>{"begin", "end", "step"} /* range3 */,
        std::vector<std::string_view>{"list"} /* list_length */,
        std::vector<std::string_view>{} /* time */,
        std::vector<std::string_view>{"value"} /* abs */,
        std::vector<std::string_view>{"value"} /* sqrt */,
        std::vector<std::string_view>{"degree"} /* sin */,
        std::vector<std::string_view>{"degree"} /* cos */,
        std::vector<std::string_view>{"degree"} /* tan */,
        std::vector<std::string_view>{"value"} /* asin */,
        std::vector<std::string_view>{"value"} /* acos */,
        std::vector<std::string_view>{"value"} /* atan */,
        std::vector<std::string_view>{"value"} /* ln */,
        std::vector<std::string_view>{"value"} /* log */,
        std::vector<std::string_view>{"value"} /* round */,
        std::vector<std::string_view>{"value"} /* floor */,
        std::vector<std::string_view>{"value"} /* ceil */};
    return _placeholders;
  }

 public:
  static const auto& args(ast::system_function func) {
    return placeholders()[static_cast<size_t>(func)];
  }
};

template <>
inline std::string placeholder::get<ast::system_function_call>(
    const ast::system_function_call& parent, ast::base::child_index index) {
  const auto& args{placeholder_system_function_args::args(parent.func)};
  assert(index.subnode_index == 0 && index.node_index < args.size());
  return std::string{args[index.node_index]};
}

template <>
inline std::string placeholder::get<ast::user_function_call>(
    const ast::user_function_call& parent, ast::base::child_index index) {
  assert(index.subnode_index == 0);
  if (parent.func() != nullptr) {
    auto& definition = *parent.func();
    if (index.node_index < definition.parameters.size()) {
      return definition.parameters[index.node_index];
    } else {
      return empty();
    }
  } else {
    // Unknown user function
    return default_text();
  }
}

}  // namespace marlin::control

#endif  // marlin_control_placeholders_hpp
