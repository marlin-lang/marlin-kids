#ifndef marlin_store_byte_span_hpp
#define marlin_store_byte_span_hpp

#include <cstddef>
#include <string_view>
#include <vector>

namespace marlin::store {

using data_vector = std::vector<std::byte>;

struct data_view {
  using pointer = const std::byte*;

  constexpr data_view(pointer begin, size_t count)
      : _begin{begin}, _end{begin + count} {}

  constexpr data_view(pointer begin, pointer end) : _begin{begin}, _end{end} {}

  constexpr data_view(const data_vector& cont)
      : _begin{cont.data()}, _end{cont.data() + cont.size()} {}

  constexpr pointer begin() const noexcept { return _begin; }
  constexpr pointer end() const noexcept { return _end; }

  constexpr size_t size() const noexcept {
    assert(_begin <= _end);
    return static_cast<size_t>(_end - _begin);
  }

  operator std::string_view() const noexcept {
    if (size() == 0) {
      return "";
    } else {
      return {reinterpret_cast<const char*>(_begin), size()};
    }
  }

 private:
  pointer _begin;
  pointer _end;
};

}  // namespace marlin::store

#endif  // marlin_store_byte_span_hpp