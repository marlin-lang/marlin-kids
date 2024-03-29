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
    size_t arg_check_count{_func->parameters.size()};
    if (args.size() < _func->parameters.size()) {
      changed = true;
      arg_check_count = args.size();
      for (auto it{_func->parameters.begin() + args.size()};
           it != _func->parameters.end(); it++) {
        args.emplace_back(ast::make<expression_placeholder>(*it));
      }
    } else if (args.size() > _func->parameters.size()) {
      for (auto i{args.size() - 1}; args.size() > _func->parameters.size();
           i--) {
        if (args[i]->is<ast::expression_placeholder>()) {
          changed = true;
          args.pop(i);
        }
        if (i == 0) {
          break;
        }
      }
    }
    for (auto i{0}; i < arg_check_count; i++) {
      if (args[i]->is<ast::expression_placeholder>()) {
        auto& placeholder{args[i]->as<ast::expression_placeholder>()};
        if (placeholder.name != _func->parameters[i]) {
          changed = true;
          placeholder.name = _func->parameters[i];
        }
      }
    }

    return changed;
  }
}

}  // namespace marlin::ast
