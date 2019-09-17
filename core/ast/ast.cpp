#include "ast.hpp"

namespace marlin::ast {

bool user_function_call::assign_definition(const function_definition* func) {
  _func = func;

  if (_func == nullptr) {
    return false;
  } else {
    bool changed{false};

    if (name != _func->name) {
      changed = true;
      name = _func->name;
    }

    auto args{arguments()};
    if (args.size() < _func->parameters.size()) {
      changed = true;
      auto it{_func->parameters.begin() + args.size()};
      while (it != _func->parameters.end()) {
        args.emplace_back(ast::make<expression_placeholder>(*it));
        it++;
      }
    } else if (args.size() > _func->parameters.size()) {
      // TODO: remove excess arguments
    }

    return changed;
  }
}

}  // namespace marlin::ast