#ifndef marlin_ast_utils_hpp
#define marlin_ast_utils_hpp

#include <utility>
#include <vector>

namespace marlin {

// using jsast::source_loc;
// using jsast::source_range;

struct source_loc {
  size_t line;
  size_t column;

  source_loc() noexcept : source_loc{0, 0} {}
  source_loc(size_t _line, size_t _column) noexcept
      : line{_line}, column{_column} {}

  bool operator==(const source_loc& other) const noexcept {
    return cmp(other) == 0;
  }
  bool operator!=(const source_loc& other) const noexcept {
    return cmp(other) != 0;
  }
  bool operator<(const source_loc& other) const noexcept {
    return cmp(other) < 0;
  }
  bool operator>(const source_loc& other) const noexcept {
    return cmp(other) > 0;
  }
  bool operator<=(const source_loc& other) const noexcept {
    return cmp(other) <= 0;
  }
  bool operator>=(const source_loc& other) const noexcept {
    return cmp(other) >= 0;
  }

 private:
  // TODO : Use c++ 20 std::strong_ordering
  // TODO: Use c++20 3-way compare operator (and make this public)
  [[nodiscard]] int cmp(const source_loc& other) const noexcept {
    if (line < other.line) {
      return -1;
    } else if (line > other.line) {
      return 1;
    } else if (column < other.column) {
      return -1;
    } else if (column > other.column) {
      return 1;
    } else {
      return 0;
    }
  }
};

struct source_range {
  source_loc begin;
  source_loc end;

  source_range() noexcept : source_range{{}, {}} {}
  source_range(source_loc _begin, source_loc _end) noexcept
      : begin{_begin}, end{_end} {}

  [[nodiscard]] bool contains(const source_loc& loc) const noexcept {
    return loc >= begin && loc < end;
  }
};

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