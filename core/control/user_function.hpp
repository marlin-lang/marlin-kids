#ifndef marlin_control_user_function_hpp
#define marlin_control_user_function_hpp

#include <memory>
#include <unordered_map>
#include <utility>

#include "function_definition.hpp"
#include "prototypes.hpp"
#include "store_definition.hpp"
#include "toolbox.hpp"

namespace marlin::control {

struct user_function_table {
  struct entry {
    function_definition definition;
    prototype ptype;

    entry(function_definition _definition)
        : definition{std::move(_definition)},
          ptype{user_function_prototype(definition)} {}
  };

  [[nodiscard]] const auto& map() const { return _map; }

  [[nodiscard]] const bool has_function(const std::string& name) const {
    return _map.find(name) != _map.end();
  }

  const function_definition& add_function(function_definition signature) {
    assert(!has_function(signature.name));
    auto& ref{_map[signature.name]};
    ref = std::make_unique<entry>(std::move(signature));

    if (auto ptr{_toolbox.lock()}) {
      ptr->add_user_functions({&ref->ptype});
    }

    return ref->definition;
  }

  const function_definition& replace_function(
      const std::string& name, function_definition new_signature) {
    assert(has_function(name));
    auto original{std::move(_map[name])};
    _map.erase(name);
    auto& ref{_map[new_signature.name]};
    ref = std::make_unique<entry>(std::move(new_signature));

    if (auto ptr{_toolbox.lock()}) {
      ptr->replace_user_function(&original->ptype, &ref->ptype);
    }

    return ref->definition;
  }

  void remove_function(const std::string& name) {
    assert(has_function(name));
    auto original{std::move(_map[name])};
    _map.erase(name);

    if (auto ptr{_toolbox.lock()}) {
      ptr->remove_user_function(&original->ptype);
    }
  }

  void set_toolbox(std::weak_ptr<toolbox> model) {
    _toolbox = std::move(model);

    if (auto ptr{_toolbox.lock()}) {
      std::vector<const prototype*> prototypes;
      for (auto& entry : _map) {
        prototypes.emplace_back(&entry.second->ptype);
      }
      ptr->add_user_functions(std::move(prototypes));
    }
  }

 private:
  std::unordered_map<std::string, std::unique_ptr<entry>> _map;

  std::weak_ptr<toolbox> _toolbox;
};

struct temporary_user_function_table_holder final
    : store::user_function_table_interface {
  [[nodiscard]] user_function_table get() && { return std::move(_functions); }

  [[nodiscard]] const bool has_function(
      const std::string& name) const override {
    return _functions.has_function(name);
  }

  [[nodiscard]] const function_definition& get_function(
      const std::string& name) const override {
    assert(has_function(name));
    return _functions.map().at(name)->definition;
  }

  void add_function(function_definition signature) override {
    _functions.add_function(std::move(signature));
  }

 private:
  user_function_table _functions;
};

}  // namespace marlin::control

#endif  // marlin_control_user_function_hpp
