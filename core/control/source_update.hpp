#ifndef marlin_control_source_update_hpp
#define marlin_control_source_update_hpp

#include <string>
#include <vector>

#include "store.hpp"
#include "utils.hpp"

namespace marlin::control {

using highlight_token_type = store::highlight_token_type;
using highlight_token = store::highlight_token;

struct source_update {
  source_range range;
  std::string source;
  std::vector<highlight_token> highlights;

  source_update(source_range _range, std::string _source,
                std::vector<highlight_token> _highlights)
      : range{_range},
        source{std::move(_source)},
        highlights{std::move(_highlights)} {}
};

}  // namespace marlin::control

#endif  // marlin_control_source_update_hpp