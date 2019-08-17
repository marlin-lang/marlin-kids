#ifndef marlin_store_store_definition_hpp
#define marlin_store_store_definition_hpp

#include <string>
#include <string_view>
#include <vector>

#include "node.hpp"
#include "store_errors.hpp"
#include "utils.hpp"

namespace marlin::store {

enum class type_expectation { program, block, statement, lvalue, rvalue, any };

enum class highlight_token_type {
  keyword,
  op,
  boolean,
  number,
  string,
  placeholder
};

struct highlight_token {
  highlight_token_type type;
  size_t offset;
  size_t length;

  highlight_token(highlight_token_type _type, size_t _offset, size_t _length)
      : type(_type), offset(_offset), length(_length) {}
};

struct reconstruction_result {
  std::vector<ast::node> nodes;
  std::string source;
  std::vector<highlight_token> highlights;

  reconstruction_result(std::vector<ast::node> _nodes, std::string _source,
                        std::vector<highlight_token> _highlights)
      : nodes{std::move(_nodes)},
        source{std::move(_source)},
        highlights{std::move(_highlights)} {}
};

struct base_store {
  template <typename store_type>
  struct impl;

  static base_store* corresponding_store(std::string_view data) {
    for (auto* s : get_stores()) {
      if (s->recognize(data)) {
        return s;
      }
    }
    throw read_error{"Unrecognized data format!"};
  }

  virtual ~base_store() noexcept = default;

  virtual bool recognize(std::string_view data) = 0;
  virtual reconstruction_result read(std::string_view data, source_loc start,
                                     size_t indent, type_expectation type,
                                     size_t paren_precedence = 0) = 0;

  // The latest version also needs to implement
  // std::string write(std::vector<const ast::base*> node);

 private:
  [[nodiscard]] static std::vector<base_store*>& get_stores() {
    static std::vector<base_store*> _stores;
    return _stores;
  }
};

template <typename store_type>
struct base_store::impl : base_store {
  friend std::string write(std::vector<const ast::base*> nodes);

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