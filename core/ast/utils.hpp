#ifndef marlin_ast_utils_hpp
#define marlin_ast_utils_hpp

#include <utility>
#include <vector>

#include <jsast/jsast.hpp>

namespace marlin {

using jsast::source_loc;
using jsast::source_range;

using jsast::utils::quoted;

namespace utils {

template <typename vector>
struct vector_view {
  using value_type = typename vector::value_type;
  using size_type = typename vector::size_type;
  using reference = typename vector::reference;
  using pointer = typename vector::pointer;
  using iterator = typename vector::iterator;

  vector_view(vector& vec) : _vec{vec} {}

  [[nodiscard]] reference at(size_type pos) const { return _vec.at(pos); }
  [[nodiscard]] reference operator[](size_type pos) const { return _vec[pos]; }

  [[nodiscard]] iterator begin() const noexcept { return _vec.begin(); }
  [[nodiscard]] iterator end() const noexcept { return _vec.end(); }
  [[nodiscard]] reference front() const { return _vec.front(); }
  [[nodiscard]] reference back() const { return _vec.back(); }
  [[nodiscard]] pointer data() const noexcept { return _vec.data(); }

  [[nodiscard]] bool empty() const noexcept { return _vec.empty(); }
  [[nodiscard]] size_type size() const noexcept { return _vec.size(); }

 private:
  vector& _vec;
};

}  // namespace utils

}  // namespace marlin

#endif  // marlin_ast_utils_hpp