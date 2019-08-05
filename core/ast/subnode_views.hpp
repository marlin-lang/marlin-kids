#ifndef marlin_ast_subnode_views_hpp
#define marlin_ast_subnode_views_hpp

#include <vector>

#include "node.hpp"
#include "subnodes.hpp"

namespace marlin::ast::subnode {

using data_vector = std::vector<node>;

template <typename base_type>
struct vector_view {
  using value_type = typename data_vector::value_type;
  using size_type = typename data_vector::size_type;
  using reference = typename data_vector::reference;
  using pointer = typename data_vector::pointer;
  using iterator = typename data_vector::iterator;

  vector_view(base_type& base, vector& vec) : _base{base}, _vec{vec} {}

  [[nodiscard]] reference at(size_type pos) const {
    return _data().at(_vec.index + pos);
  }
  [[nodiscard]] reference operator[](size_type pos) const {
    return _data()[_vec.index + pos];
  }

  [[nodiscard]] iterator begin() const noexcept {
    return _data().begin() + _vec.index;
  }
  [[nodiscard]] iterator end() const noexcept {
    return _data().begin() + _vec.index + _vec.size;
  }
  [[nodiscard]] reference front() const { return *begin(); }
  [[nodiscard]] reference back() const { return *(end() - 1); }
  [[nodiscard]] pointer data() const noexcept {
    return _data().data() + _vec.index;
  }

  [[nodiscard]] bool empty() const noexcept { return _vec.size == 0; }
  [[nodiscard]] size_type size() const noexcept { return _vec.size; }

  template <class... arg_type>
  void emplace(size_type pos, arg_type&&... args) const {
    _data().emplace(_data().begin() + _vec.index + pos,
                    std::forward<arg_type>(args)...);
    (*this)[pos]->_parent = &_base;
    _vec.size++;
    _base.apply_update_subnode_refs();
  }

  template <class... arg_type>
  reference emplace_back(arg_type&&... args) const {
    return emplace(_vec.size, std::forward<arg_type>(args)...);
  }

  value_type pop(size_type pos) const {
    auto it = _data().begin() + _vec.index + pos;
    auto item = std::move(*it);
    item->_parent = nullptr;
    _data().erase(it);
    _vec.size--;
    _base.apply_update_subnode_refs();
    return item;
  }

 private:
  base_type& _base;
  vector& _vec;

  data_vector& _data() const noexcept { return _base._children; }
};

template <typename base_type>
struct const_vector_view {
  using value_type = typename data_vector::value_type;
  using size_type = typename data_vector::size_type;
  using const_reference = typename data_vector::const_reference;
  using const_pointer = typename data_vector::const_pointer;
  using const_iterator = typename data_vector::const_iterator;

  const_vector_view(const base_type& base, const vector& vec)
      : _base{base}, _vec{vec} {}

  [[nodiscard]] const_reference at(size_type pos) const {
    return _data().at(_vec.index + pos);
  }
  [[nodiscard]] const_reference operator[](size_type pos) const {
    return _data()[_vec.index + pos];
  }

  [[nodiscard]] const_iterator begin() const noexcept {
    return _data().begin() + _vec.index;
  }
  [[nodiscard]] const_iterator end() const noexcept {
    return _data().begin() + _vec.index + _vec.size;
  }
  [[nodiscard]] const_reference front() const { return *begin(); }
  [[nodiscard]] const_reference back() const { return *(end() - 1); }
  [[nodiscard]] const_pointer data() const noexcept {
    return _data().data() + _vec.index;
  }

  [[nodiscard]] bool empty() const noexcept { return _vec.size == 0; }
  [[nodiscard]] size_type size() const noexcept { return _vec.size; }

 private:
  const base_type& _base;
  const vector& _vec;

  const data_vector& _data() const noexcept { return _base._children; }
};

}  // namespace marlin::ast::subnode

#endif  // marlin_ast_subnode_views_hpp