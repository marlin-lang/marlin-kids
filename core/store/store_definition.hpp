#ifndef marlin_store_store_definition_hpp
#define marlin_store_store_definition_hpp

#include <string>
#include <vector>

#include "byte_span.hpp"
#include "formatter.hpp"
#include "function_definition.hpp"
#include "node.hpp"
#include "store_errors.hpp"
#include "utils.hpp"

namespace marlin::store {

struct user_function_table_interface {
  [[nodiscard]] virtual const bool has_function(
      const std::string& name) const = 0;
  [[nodiscard]] virtual const function_definition& get_function(
      const std::string& name) const = 0;

  virtual void add_function(function_definition signature) = 0;
};

enum struct type_expectation {
  program,
  block,
  function_signature,
  statement,
  lvalue,
  rvalue,
  any
};

struct reconstruction_result {
  std::vector<ast::node> nodes;
  format::display display;

  reconstruction_result(std::vector<ast::node> _nodes, format::display _display)
      : nodes{std::move(_nodes)}, display{std::move(_display)} {}
};

struct base_store {
  template <typename store_type>
  struct impl;

  static base_store* corresponding_store(data_view data) {
    for (auto* s : get_stores()) {
      if (s->recognize(data)) {
        return s;
      }
    }
    throw read_error{"Unrecognized data format!"};
  }

  virtual ~base_store() noexcept = default;

  virtual bool recognize(data_view data) = 0;
  virtual std::vector<ast::node> read(data_view data, type_expectation type,
                                      user_function_table_interface& table) = 0;

  // The latest version also needs to implement
  // data_vector write(std::vector<const ast::base*> node);

 private:
  [[nodiscard]] static std::vector<base_store*>& get_stores() {
    static std::vector<base_store*> _stores;
    return _stores;
  }
};

template <typename store_type>
struct base_store::impl : base_store {
  friend data_vector write(std::vector<const ast::base*> nodes);

 protected:
  impl() { get_stores().emplace_back(&_singleton); }

 private:
  template <store_type&>
  struct store_ref {};

  inline static store_type _singleton;
  inline static store_ref<_singleton> _ref;
};

}  // namespace marlin::store

#endif  // marlin_store_store_definition_hpp