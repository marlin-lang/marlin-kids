#ifndef marlin_store_store_definition_hpp
#define marlin_store_store_definition_hpp

#include <string>
#include <vector>

#include "node.hpp"

namespace marlin::store {

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
};

struct base_store {
  friend reconstruction_result read(const std::string& data,
                                    const ast::base* parent);

  template <typename store_type>
  struct impl;

  virtual ~base_store() noexcept = default;

  virtual bool recognize(const std::string& data) = 0;
  virtual reconstruction_result read(const std::string& data,
                                     const ast::base* parent) = 0;

  // The latest version also needs to implement
  // std::string write(std::vector<const ast::base*> node);

 private:
  inline static std::vector<base_store*> _stores;
};

template <typename store_type>
struct base_store::impl : base_store {
  friend std::string write(std::vector<const ast::base*> nodes);

 protected:
  impl() { _stores.emplace_back(&_singleton); }

 private:
  template <store_type&>
  struct store_ref {};

  inline static store_type _singleton;
  inline static store_ref<_singleton> _ref;
};

}  // namespace marlin::store

#endif  // marlin_store_store_definition_hpp