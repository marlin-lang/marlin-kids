#ifndef marlin_ast_base_impl
#define marlin_ast_base_impl

#include <optional>
#include <tuple>
#include <type_traits>
#include <vector>

#include "node.hpp"
#include "subnode_views.hpp"
#include "subnodes.hpp"
#include "utils.hpp"

namespace marlin {

namespace exec {
struct generator;
}

namespace ast {

struct base {
  friend exec::generator;

  friend subnode::concrete_view<base>;
  friend subnode::vector_view<base>;

  template <typename node_type, typename... subnode_types>
  struct impl;

  struct child_index {
    size_t subnode_index;
    size_t node_index;

    child_index(size_t _subnode_index, size_t _node_index = 0)
        : subnode_index{_subnode_index}, node_index{_node_index} {}
  };

  source_range source_code_range;

  // Only work with heap allocation and pointers
  base(base &&) = delete;
  base(const base &) = delete;
  base &operator=(base &&) = delete;
  base &operator=(const base &) = delete;

  [[nodiscard]] size_t type() const { return _typeid; }

  template <typename node_type>
  [[nodiscard]] bool is() const {
    return type() == get_typeid<node_type>();
  }
  template <typename node_type>
  [[nodiscard]] node_type &as() {
    return *static_cast<node_type *>(this);
  }
  template <typename node_type>
  [[nodiscard]] const node_type &as() const {
    return *static_cast<const node_type *>(this);
  }

  template <typename return_type, typename callable_type>
  return_type apply(callable_type callable);
  template <typename return_type, typename callable_type>
  return_type apply(callable_type callable) const;

  template <typename super_type>
  [[nodiscard]] bool inherits() const {
    return apply<bool>([](const auto &n) {
      return std::is_base_of_v<super_type, std::decay_t<decltype(n)>>;
    });
  }

  [[nodiscard]] bool contains(source_loc loc) const noexcept {
    return source_code_range.contains(loc);
  }

  [[nodiscard]] base &locate(source_loc loc);
  [[nodiscard]] const base &locate(source_loc loc) const;

  [[nodiscard]] base &locate_js(source_loc loc);
  [[nodiscard]] const base &locate_js(source_loc loc) const;

  [[nodiscard]] bool has_parent() const noexcept { return _parent != nullptr; }
  [[nodiscard]] base &parent() { return *_parent; }
  [[nodiscard]] const base &parent() const { return *_parent; }

  [[nodiscard]] utils::vector_view<std::vector<node>> children() {
    return _children;
  }
  [[nodiscard]] const std::vector<node> &children() const { return _children; }

  node replace_child(base &existing, node replacement) {
    size_t i{0};
    while (i < _children.size() && _children[i].get() != &existing) {
      i++;
    }
    if (i < _children.size()) {
      replacement->_parent = this;
      auto result{std::exchange(_children[i], std::move(replacement))};
      result->_parent = nullptr;
      return result;
    } else {
      /* should not occur */
      assert(false);
      return replacement;
    }
  }

  child_index index_for_child(const ast::base &child) const {
    return apply<child_index>(
        [&child](auto &n) { return n.index_for_child(child); });
  }

 private:
  size_t _typeid;

  std::vector<node> _children;
  base *_parent{nullptr};

  source_range _js_range;

  explicit base(size_t tid, size_t subnode_count) : _typeid{tid} {
    _children.reserve(subnode_count);
  }

  void apply_update_subnode_refs() {
    apply<void>([](auto &n) { n.update_subnode_refs(); });
  }

  subnode::concrete_view<base> get_subnode_by_ref(subnode::concrete &r) {
    return {*this, r};
  }
  subnode::const_concrete_view<base> get_subnode_by_ref(
      const subnode::concrete &r) const {
    return {*this, r};
  }
  subnode::vector_view<base> get_subnode_by_ref(subnode::vector &v) {
    return {*this, v};
  }
  subnode::const_vector_view<base> get_subnode_by_ref(
      const subnode::vector &v) const {
    return {*this, v};
  }

  template <typename node_type>
  [[nodiscard]] static constexpr size_t get_typeid() noexcept;
};

namespace base_utils {

template <typename subnode_type>
struct type_config {};

template <>
struct type_config<subnode::concrete> {
  using store = node;
};

template <>
struct type_config<subnode::vector> {
  using store = std::vector<node>;
};

template <typename... subnode_types>
struct update_checker {
  static constexpr bool needs_update = false;
};

template <typename... types>
struct update_checker<subnode::concrete, types...> {
  static constexpr bool needs_update = update_checker<types...>::needs_update;
};

template <typename t0, typename... types>
struct update_checker<subnode::vector, t0, types...> {
  static constexpr bool needs_update = true;
};

}  // namespace base_utils

template <typename node_type, typename... subnode_types>
struct base::impl : base {
  friend base;

  using base_type = impl<node_type, subnode_types...>;

  explicit impl(
      typename base_utils::type_config<subnode_types>::store... stores)
      : base{get_typeid<node_type>(), count_subnodes(stores...)} {
    init<0>(std::move(stores)...);
    for (auto &child : children()) {
      child->_parent = this;
    }
  }

  template <size_t index>
  decltype(auto) get_subnode() {
    return get_subnode_by_ref(std::get<index>(_subs));
  }
  template <size_t index>
  decltype(auto) get_subnode() const {
    return get_subnode_by_ref(std::get<index>(_subs));
  }

  child_index index_for_child(const ast::base &child) const {
    return test_subsequent_child_indices<0>(child);
  }

 private:
  std::tuple<subnode_types...> _subs;

  [[nodiscard]] size_t count_subnodes() { return 0; }
  template <typename... arg_type>
  [[nodiscard]] size_t count_subnodes(const node &, arg_type &&... args) {
    return 1 + count_subnodes(std::forward<arg_type>(args)...);
  }
  template <typename... arg_type>
  [[nodiscard]] size_t count_subnodes(const std::optional<node> &value,
                                      arg_type &&... args) {
    return (value.has_value() ? 1 : 0) +
           count_subnodes(std::forward<arg_type>(args)...);
  }
  template <typename... arg_type>
  [[nodiscard]] size_t count_subnodes(const std::vector<node> &value,
                                      arg_type &&... args) {
    return value.size() + count_subnodes(std::forward<arg_type>(args)...);
  }

  template <size_t index>
  void init() const noexcept {}
  template <size_t index, typename... types>
  void init(node store0, types... stores) {
    std::get<index>(_subs).index = _children.size();
    _children.emplace_back(std::move(store0));
    init<index + 1>(std::move(stores)...);
  }
  template <size_t index, typename... types>
  void init(std::optional<node> store0, types... stores) {
    std::get<index>(_subs).index = _children.size();
    if (store0.has_value()) {
      std::get<index>(_subs).has_value = true;
      _children.emplace_back(*std::move(store0));
    } else {
      std::get<index>(_subs).has_value = false;
    }
    init<index + 1>(std::move(stores)...);
  }
  template <size_t index, typename... types>
  void init(std::vector<node> store0, types... stores) {
    std::get<index>(_subs).index = _children.size();
    std::get<index>(_subs).size = store0.size();
    std::move(store0.begin(), store0.end(), std::back_inserter(_children));
    init<index + 1>(std::move(stores)...);
  }

  template <size_t index>
  child_index test_subsequent_child_indices(const ast::base &child) const {
    if constexpr (index < sizeof...(subnode_types)) {
      if (auto result{test_child_index(get_subnode<index>(), child)}) {
        return {index, *std::move(result)};
      } else {
        return test_subsequent_child_indices<index + 1>(child);
      }
    } else {
      assert(false);
      return {0, 0};
    }
  }

  static std::optional<size_t> test_child_index(
      ast::subnode::const_concrete_view<ast::base> view,
      const ast::base &child) {
    if (view.get() == &child) {
      return 0;
    } else {
      return std::nullopt;
    }
  }

  static std::optional<size_t> test_child_index(
      ast::subnode::const_vector_view<ast::base> view, const ast::base &child) {
    for (size_t i{0}; i < view.size(); i++) {
      if (view[i].get() == &child) {
        return i;
      }
    }
    return std::nullopt;
  }

  void update_subnode_refs() noexcept {
    if constexpr (base_utils::update_checker<subnode_types...>::needs_update) {
      update_subnodes<0>(0);
    }
  }

  template <size_t index>
  void update_subnodes(size_t target) noexcept {
    if constexpr (index < sizeof...(subnode_types)) {
      update_subnodes<index + 1>(
          update_subnode(std::get<index>(_subs), target));
    }
  }

  static size_t update_subnode(subnode::concrete &var, size_t target) noexcept {
    var.index = target;
    return target + 1;
  }

  static size_t update_subnode(subnode::vector &var, size_t target) noexcept {
    var.index = target;
    return target + var.size;
  }
};

}  // namespace ast

}  // namespace marlin

#endif  // marlin_ast_base_impl
