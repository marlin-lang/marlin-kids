#ifndef marlin_control_placeholders_hpp
#define marlin_control_placeholders_hpp

#include <array>
#include <optional>
#include <string>

#include "ast.hpp"
#include "node.hpp"

namespace marlin::control {

// To be extracted into common utils file later
namespace placeholder_utils {

template <typename... types>
struct type_counter {
  static constexpr size_t count = 0;
};

template <typename t0, typename... types>
struct type_counter<t0, types...> {
  static constexpr size_t count = 1 + type_counter<types...>::count;
};

}  // namespace placeholder_utils

struct placeholder {
  inline static constexpr const char* default_text{"value"};

  template <typename node_type>
  static std::string get(size_t subnode_index, size_t node_index = 0) {
    return default_text;
  }

  template <typename node_type>
  static std::string get(const node_type& parent, size_t subnode_index,
                         size_t node_index = 0) {
    return get<node_type>(subnode_index, node_index);
  }

  static std::string get_replacing_node(const ast::base& node) {
    if (node.has_parent()) {
      return node.parent().apply<std::string>(
          [&node](const auto& n) { return get_from_parent<0>(n, node); });
    } else {
      return default_text;
    }
  }

 private:
  template <size_t index, typename node_type, typename... subnode_type>
  static std::string get_from_parent(
      const ast::base::impl<node_type, subnode_type...>& parent,
      const ast::base& node) {
    if constexpr (index >=
                  placeholder_utils::type_counter<subnode_type...>::count) {
      return default_text;
    } else {
      auto result{try_get_at_subnode_index(
          reinterpret_cast<const node_type&>(parent), index,
          parent.template get_subnode<index>(), node)};
      if (result.has_value()) {
        return *std::move(result);
      } else {
        return get_from_parent<index + 1>(parent, node);
      }
    }
  }

  template <typename node_type>
  static std::optional<std::string> try_get_at_subnode_index(
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
  static std::optional<std::string> try_get_at_subnode_index(
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
inline std::string placeholder::get<ast::assignment>(size_t subnode_index,
                                                     size_t node_index) {
  static constexpr std::array subnodes{"variable", "value"};
  return subnodes[subnode_index];
}

template <>
inline std::string placeholder::get<ast::print_statement>(size_t subnode_index,
                                                          size_t node_index) {
  return "value";
}

inline std::string _get_if_placeholder(size_t subnode_index) {
  if (subnode_index == 0) {
    return "condition";
  } else {
    return placeholder::default_text;
  }
}
template <>
inline std::string placeholder::get<ast::if_statement>(size_t subnode_index,
                                                       size_t node_index) {
  return _get_if_placeholder(subnode_index);
}
template <>
inline std::string placeholder::get<ast::if_else_statement>(
    size_t subnode_index, size_t node_index) {
  return _get_if_placeholder(subnode_index);
}

template <>
inline std::string placeholder::get<ast::unary_expression>(size_t subnode_index,
                                                           size_t node_index) {
  return "argument";
}

template <>
inline std::string placeholder::get<ast::binary_expression>(
    size_t subnode_index, size_t node_index) {
  static constexpr std::array subnodes{"left", "right"};
  return subnodes[subnode_index];
}

}  // namespace marlin::control

#endif  // marlin_control_placeholders_hpp
