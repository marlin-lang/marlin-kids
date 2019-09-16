#ifndef marlin_ast_function_definition_hpp
#define marlin_ast_function_definition_hpp

#include <string>
#include <vector>

namespace marlin {

struct function_definition {
  std::string name;
  std::vector<std::string> parameters;

  function_definition() {}
  function_definition(std::string _name) : name{std::move(_name)} {}
  function_definition(std::string _name, std::vector<std::string> _params)
      : name{std::move(_name)}, parameters{std::move(_params)} {}
};

}  // namespace marlin

#endif  // marlin_ast_function_definition_hpp