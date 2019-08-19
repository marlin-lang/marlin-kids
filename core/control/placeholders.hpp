#ifndef marlin_control_placeholders_hpp
#define marlin_control_placeholders_hpp

#include <array>
#include <optional>
#include <string_view>

#include "ast.hpp"
#include "node.hpp"

namespace marlin::control {

struct placeholder {
  inline static constexpr std::string_view default_text{"value"};

  template <typename node_type>
  static std::string_view get(size_t subnode_index, size_t node_index = 0) {
    return default_text;
  }

  template <typename node_type>
  static std::string_view get(const node_type& parent, size_t subnode_index,
                              size_t node_index = 0) {
    return get<node_type>(subnode_index, node_index);
  }

  static std::string_view get_replacing_node(const ast::base& node) {
    if (node.has_parent()) {
      return node.parent().apply<std::string_view>(
          [&node](const auto& n) { return get_from_parent<0>(n, node); });
    } else {
      return default_text;
    }
  }

 private:
  template <size_t index, typename node_type, typename... subnode_type>
  static std::string_view get_from_parent(
      const ast::base::impl<node_type, subnode_type...>& parent,
      const ast::base& node) {
    if constexpr (index < sizeof...(subnode_type)) {
      auto result{try_get_at_subnode_index(
          reinterpret_cast<const node_type&>(parent), index,
          parent.template get_subnode<index>(), node)};
      if (result.has_value()) {
        return *std::move(result);
      } else {
        return get_from_parent<index + 1>(parent, node);
      }
    } else {
      return default_text;
    }
  }

  template <typename node_type>
  static std::optional<std::string_view> try_get_at_subnode_index(
      const node_type& parent, size_t subnode_index,
      ast::subnode::const_concrete_view<ast::base> view,
      const ast::base& node) {
    if (view.get() == &node) {
      return get(parent, subnode_index, 0);
    } else {
      return std::nullopt;
    }
  }

  template <typename node_type>
  static std::optional<std::string_view> try_get_at_subnode_index(
      const node_type& parent, size_t subnode_index,
      ast::subnode::const_vector_view<ast::base> view, const ast::base& node) {
    for (size_t i{0}; i < view.size(); i++) {
      if (view[i].get() == &node) {
        return get(parent, subnode_index, i);
      }
    }
    return std::nullopt;
  }
};

template <>
inline std::string_view placeholder::get<ast::assignment>(size_t subnode_index,
                                                          size_t node_index) {
  static constexpr std::array<std::string_view, 2> subnodes{"variable",
                                                            "value"};
  return subnodes[subnode_index];
}

template <>
inline std::string_view placeholder::get<ast::print_statement>(
    size_t subnode_index, size_t node_index) {
  return "value";
}

inline std::string_view _get_if_placeholder(size_t subnode_index) {
  if (subnode_index == 0) {
    return "condition";
  } else {
    return placeholder::default_text;
  }
}
template <>
inline std::string_view placeholder::get<ast::if_statement>(
    size_t subnode_index, size_t node_index) {
  return _get_if_placeholder(subnode_index);
}
template <>
inline std::string_view placeholder::get<ast::if_else_statement>(
    size_t subnode_index, size_t node_index) {
  return _get_if_placeholder(subnode_index);
}

template <>
inline std::string_view placeholder::get<ast::while_statement>(
    size_t subnode_index, size_t node_index) {
  if (subnode_index == 0) {
    return "condition";
  } else {
    return default_text;
  }
}

template <>
inline std::string_view placeholder::get<ast::for_statement>(
    size_t subnode_index, size_t node_index) {
  static constexpr std::array<std::string_view, 3> subnodes{"variable", "list",
                                                            default_text};
  return subnodes[subnode_index];
}

template <>
inline std::string_view placeholder::get<ast::unary_expression>(
    size_t subnode_index, size_t node_index) {
  return "argument";
}

template <>
inline std::string_view placeholder::get<ast::binary_expression>(
    size_t subnode_index, size_t node_index) {
  static constexpr std::array<std::string_view, 2> subnodes{"left", "right"};
  return subnodes[subnode_index];
}

struct placeholder_system_procedure_args {
 private:
  static const auto& placeholders() {
    static const std::array _placeholders{
        std::vector<std::string_view>{"start_x", "start_y", "end_x",
                                      "end_y"} /* draw_line */,
    };
    return _placeholders;
  }

 public:
  static const std::vector<std::string_view>& args(ast::system_procedure func) {
    return placeholders()[static_cast<size_t>(func)];
  }
};

template <>
inline std::string_view placeholder::get<ast::system_procedure_call>(
    const ast::system_procedure_call& parent, size_t subnode_index,
    size_t node_index) {
  return placeholder_system_procedure_args::args(parent.proc)[node_index];
}

struct placeholder_system_function_args {
 private:
  static const auto& placeholders() {
    static const std::array _placeholders{
        std::vector<std::string_view>{"end"} /* range1 */,
        std::vector<std::string_view>{"begin", "end"} /* range2 */,
        std::vector<std::string_view>{"begin", "end", "step"} /* range3 */,
        std::vector<std::string_view>{} /* time */
    };
    return _placeholders;
  }

 public:
  static const std::vector<std::string_view>& args(ast::system_function func) {
    return placeholders()[static_cast<size_t>(func)];
  }
};

template <>
inline std::string_view placeholder::get<ast::system_function_call>(
    const ast::system_function_call& parent, size_t subnode_index,
    size_t node_index) {
  return placeholder_system_function_args::args(parent.func)[node_index];
}

}  // namespace marlin::control

#endif  // marlin_control_placeholders_hpp
