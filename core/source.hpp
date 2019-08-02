#ifndef marlin_source_hpp
#define marlin_source_hpp

#include <cassert>
#include <cstddef>
#include <string>
#include <vector>

namespace marlin {

struct source {
  explicit source(std::string str) noexcept : _str{std::move(str)} {
    update_begin_of_lines();
  }

  const std::string& str() const noexcept { return _str; }

 private:
  void update_begin_of_lines() noexcept {
    _begin_of_lines.push_back(0);
    for (auto it = std::begin(_str); it != std::end(_str);) {
      it = std::find(it, std::end(_str), '\n');
      if (it != std::end(_str)) {
        ++it;
        _begin_of_lines.push_back(std::distance(std::begin(_str), it));
      }
    }
  }

  std::string _str;
  std::vector<size_t> _begin_of_lines;
};

}  // namespace marlin

#endif  // marlin_source_hpp
