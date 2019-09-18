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
      auto it{_func->parameters.begin() + args.size()};
      while (it != _func->parameters.end()) {
        args.emplace_back(ast::make<expression_placeholder>(*it));
        it++;
      }
    } else if (args.size() > _func->parameters.size()) {
      auto i{args.size() - 1};
      while (args.size() > _func->parameters.size()) {
        if (args[i]->is<ast::expression_placeholder>()) {
          changed = true;
          args.pop(i);
        }
        if (i == 0) {
          break;
        }
        i--;
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
