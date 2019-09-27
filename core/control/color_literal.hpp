#ifndef marlin_controL_color_literal_hpp
#define marlin_controL_color_literal_hpp

#include <algorithm>
#include <array>
#include <optional>
#include <type_traits>
#include <utility>

#include "specs.hpp"

namespace marlin::control {

namespace details {

template <typename element_type, size_t max_count>
static constexpr size_t count_dimension(
    std::array<std::optional<element_type>, max_count> arr) {
  size_t count{max_count};
  while (!arr[count - 1].has_value()) {
    count--;
  }
  return count;
}

template <size_t... indices, typename array_type>
static constexpr auto make_counts(std::index_sequence<indices...>,
                                  const array_type& array) {
  return make_array(count_dimension(array[indices])...);
}

}  // namespace details

struct color_literal {
 private:
  using restriction_t = std::optional<std::pair<double, double>>;
  static constexpr auto _data_restrictions{make_array(
      make_array(restriction_t{{0, 255}}, restriction_t{{0, 255}},
                 restriction_t{{0, 255}}, std::nullopt) /* rgb */,
      make_array(restriction_t{{0, 255}}, restriction_t{{0, 255}},
                 restriction_t{{0, 255}}, restriction_t{{0, 1}}) /* rgba */,
      make_array(restriction_t{{0, 255}}, restriction_t{{0, 1}},
                 restriction_t{{0, 1}}, std::nullopt) /* hsl */,
      make_array(restriction_t{{0, 255}}, restriction_t{{0, 1}},
                 restriction_t{{0, 1}}, restriction_t{{0, 1}}) /* hsla */)};

  static constexpr auto _data_dimension{details::make_counts(
      std::make_index_sequence<_data_restrictions.size()>{},
      _data_restrictions)};

  static constexpr size_t get_data_dimension(ast::color_mode mode) {
    return _data_dimension[raw_value(mode)];
  }

 public:
  static constexpr size_t data_dimension_max{
      std::tuple_size<decltype(_data_restrictions)::value_type>::value};

  ast::color_mode mode;

  color_literal() {}
  color_literal(ast::color_mode _mode) : mode{_mode} {}

  [[nodiscard]] double operator[](size_t i) const { return _data[i]; }

  constexpr size_t data_dimension() const { return get_data_dimension(mode); }

  void set(size_t index, double value) {
    assert(index < data_dimension());
    const auto& restrction{*_data_restrictions[raw_value(mode)][index]};
    _data[index] = std::clamp(value, restrction.first, restrction.second);
  }

  template <typename... data_types,
            typename = std::enable_if_t<std::conjunction_v<
                std::is_same<std::decay_t<data_types>, double>...>>>
  void set(data_types&&... data) {
    assert(sizeof...(data_types) == data_dimension());
    set<0>(data...);
  }

 private:
  std::array<double, data_dimension_max> _data;

  template <size_t index, typename... data_types>
  void set(data_types&&... data) {}

  template <size_t index, typename... data_types>
  void set(double first, data_types&&... data) {
    set(index, first);
    set<index + 1>(data...);
  }
};

}  // namespace marlin::control

#endif  // marlin_controL_color_literal_hpp