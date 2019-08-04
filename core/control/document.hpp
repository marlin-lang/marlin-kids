#ifndef marlin_control_document_hpp
#define marlin_control_document_hpp

#include <optional>
#include <string>
#include <utility>

#include "base.hpp"
#include "source_modifications.hpp"

#include "source.hpp"

namespace marlin::control {

struct document {
  static std::pair<document, source_initialization> make_document(
      const char* data, size_t size) {
    // TODO: parse data
    return {document{},
            source_initialization{
                "on start {\n}\n",
                {highlight_token{highlight_token_type::keyword, 0, 8}}}};
  }

  explicit document(std::string str = "") noexcept : _source{std::move(str)} {}

  const auto& source_str() const noexcept { return _source.str(); }

  const auto& output() const noexcept { return _output; }

  void execute() { _output.clear(); }

 private:
  source _source;
  std::string _output;
};

}  // namespace marlin::control

#endif  // marlin_control_document_hpp
