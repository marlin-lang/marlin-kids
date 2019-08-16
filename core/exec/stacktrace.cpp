#include "stacktrace.hpp"

#include <charconv>
#include <regex>

namespace marlin::exec {

std::vector<source_loc> parse_stacktrace(std::string_view stacktrace,
                                         std::string_view source_url) {
  std::vector<source_loc> locs;
  std::regex regex{"^.*?@(.*):([0-9]*):([0-9]*)$"};
  const char* prev{stacktrace.begin()};
  for (const char* it{stacktrace.begin()}; it <= stacktrace.end(); it++) {
    if (it == stacktrace.end() || *it == '\n') {
      std::cmatch match;
      if (std::regex_match(prev, it, match, regex) && match.size() > 3 &&
          match[1].second - match[1].first >= 0) {
        std::string_view matched_url{
            match[1].first,
            static_cast<size_t>(match[1].second - match[1].first)};
        if (matched_url == source_url) {
          size_t line{0}, column{0};
          std::from_chars(match[2].first, match[2].second, line);
          std::from_chars(match[3].first, match[3].second, column);
          locs.emplace_back(line, column);
        }
      }

      if (it < stacktrace.end()) {
        prev = it + 1;
      }
    }
  }
  return locs;
}

template <typename code_type, typename>
std::vector<code_type*> parse_stacktrace(std::string_view stacktrace,
                                         std::string_view source_url,
                                         code_type& code) {
  std::vector<code_type*> nodes;
  for (const auto& loc : parse_stacktrace(stacktrace, source_url)) {
    // JavaScriptCore reports the location after the error
    // we have to -1 to correct
    if (loc.column > 1) {
      nodes.emplace_back(&code.locate_js({loc.line, loc.column - 1}));
    } else {
      nodes.emplace_back(&code.locate_js(loc));
    }
  }
  return nodes;
}

template std::vector<ast::base*> parse_stacktrace<ast::base>(std::string_view,
                                                             std::string_view,
                                                             ast::base&);
template std::vector<const ast::base*> parse_stacktrace<const ast::base>(
    std::string_view, std::string_view, const ast::base&);

}  // namespace marlin::exec
