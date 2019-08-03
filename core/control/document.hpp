#ifndef marlin_control_document_hpp
#define marlin_control_document_hpp

#include <optional>
#include <string>

#include "ast.hpp"

#include "source.hpp"

namespace marlin::control {

struct document {
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
