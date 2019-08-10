#ifndef marlin_control_proto_gen_hpp
#define marlin_control_proto_gen_hpp

#include <string>
#include <tuple>
#include <type_traits>

#include "ast.hpp"
#include "prototype_definition.hpp"

namespace marlin::control {

namespace proto_gen {

struct base {
  template <size_t _nodes>
  struct impl;
};

template <size_t _nodes>
struct base::impl : base {
  static constexpr size_t nodes = _nodes;
};

struct raw : base::impl<0> {
  raw(const char* str) : _str{str} {}
  raw(std::string str) : _str{std::move(str)} {}

  std::string construct_all(size_t, source_loc& curr_loc, size_t& curr_offset,
                            ast::node*&, std::vector<highlight_token>&) const {
    curr_loc.column += _str.size();
    curr_offset += _str.size();
    return _str;
  }

 private:
  std::string _str;
};

struct newline : base::impl<0> {
  std::string construct_all(size_t indent, source_loc& curr_loc,
                            size_t& curr_offset, ast::node*&,
                            std::vector<highlight_token>&) const {
    const auto indent_length{indent * indent_space_count};
    curr_loc.line++;
    curr_loc.column = indent_length + 1;
    curr_offset += indent_length + 1;
    return "\n" + std::string(indent_length, ' ');
  }
};

struct store_loc : base::impl<0> {
  store_loc(source_loc& loc) : _loc{loc} {}

  std::string construct_all(size_t indent, source_loc& curr_loc,
                            size_t& curr_offset, ast::node*&,
                            std::vector<highlight_token>&) const {
    _loc = curr_loc;
    return "";
  }

 private:
  source_loc& _loc;
};

template <typename inner_type,
          typename = std::enable_if_t<std::is_base_of_v<base, inner_type>>>
struct highlight : base::impl<inner_type::nodes> {
  highlight(highlight_token_type type, inner_type inner)
      : _type{type}, _inner{std::move(inner)} {}

  std::string construct_all(size_t indent, source_loc& curr_loc,
                            size_t& curr_offset, ast::node*& target,
                            std::vector<highlight_token>& vec) const {
    auto start{curr_offset};
    auto result{
        _inner.construct_all(indent, curr_loc, curr_offset, target, vec)};
    vec.emplace_back(_type, start, curr_offset - start);
    return result;
  }

 private:
  highlight_token_type _type;
  inner_type _inner;
};

highlight(highlight_token_type, std::string)->highlight<raw>;

template <typename callable_type, typename inner_type,
          typename = std::enable_if_t<std::is_base_of_v<base, inner_type>>>
struct node : base::impl<1> {
  node(callable_type callable, inner_type inner)
      : _callable{callable}, _inner{std::move(inner)} {}

  std::string construct_all(size_t indent, source_loc& curr_loc,
                            size_t& curr_offset, ast::node*& target,
                            std::vector<highlight_token>& vec) const {
    std::array<ast::node, inner_type::nodes> nodes;
    auto start{curr_loc};
    auto inner_target{nodes.data()};
    auto inner_result{
        _inner.construct_all(indent, curr_loc, curr_offset, inner_target, vec)};
    *target = _callable(std::move(nodes));
    (*target)->source_code_range = {start, curr_loc};
    target++;
    return inner_result;
  }

 private:
  callable_type _callable;
  inner_type _inner;
};

template <typename callable_type>
node(callable_type, std::string)->node<callable_type, raw>;

template <typename left_type, typename right_type,
          typename = std::enable_if_t<std::is_base_of_v<base, left_type>>,
          typename = std::enable_if_t<std::is_base_of_v<base, right_type>>>
struct combine : base::impl<left_type::nodes + right_type::nodes> {
  combine(left_type left, right_type right)
      : _left{std::move(left)}, _right{std::move(right)} {}

  std::string construct_all(size_t indent, source_loc& curr_loc,
                            size_t& curr_offset, ast::node*& target,
                            std::vector<highlight_token>& vec) const {
    auto left_result{
        _left.construct_all(indent, curr_loc, curr_offset, target, vec)};
    auto right_result{
        _right.construct_all(indent, curr_loc, curr_offset, target, vec)};
    return std::move(left_result) + std::move(right_result);
  }

 private:
  left_type _left;
  right_type _right;
};

template <typename left_type, typename right_type,
          typename = std::enable_if_t<std::is_base_of_v<base, left_type>>,
          typename = std::enable_if_t<std::is_base_of_v<base, right_type>>>
combine<left_type, right_type> operator+(left_type left, right_type right) {
  return {std::move(left), std::move(right)};
}

template <typename left_type,
          typename = std::enable_if_t<std::is_base_of_v<base, left_type>>>
combine<left_type, raw> operator+(left_type left, std::string right) {
  return left + raw{right};
}

template <typename right_type,
          typename = std::enable_if_t<std::is_base_of_v<base, right_type>>>
combine<raw, right_type> operator+(std::string left, right_type right) {
  return raw{left} + right;
}

template <typename inner_type,
          typename = std::enable_if_t<std::is_base_of_v<base, inner_type> &&
                                      inner_type::nodes == 1>>
struct statement_generator {
  statement_generator(inner_type inner) : _inner{std::move(inner)} {}

  std::pair<ast::node, source_insertion> construct(size_t line,
                                                   size_t indent) const {
    const auto indent_length{indent * indent_space_count};
    source_loc loc{line, indent_length + 1};
    size_t offset{indent_length};
    std::vector<highlight_token> highlights;
    ast::node node;
    ast::node* target{&node};
    auto result = _inner.construct_all(indent, loc, offset, target, highlights);
    return std::make_pair(std::move(node),
                          source_insertion{{line, 1},
                                           std::string(indent_length, ' ') +
                                               std::move(result) + "\n",
                                           std::move(highlights)});
  }

 private:
  inner_type _inner;
};

template <typename inner_type,
          typename = std::enable_if_t<std::is_base_of_v<base, inner_type> &&
                                      inner_type::nodes == 1>>
struct expression_generator {
  expression_generator(inner_type inner) : _inner{std::move(inner)} {}

  std::pair<ast::node, source_replacement> construct(
      source_range original) const {
    source_loc loc{original.begin};
    size_t offset{0};
    std::vector<highlight_token> highlights;
    ast::node node;
    ast::node* target{&node};
    auto result = _inner.construct_all(0, loc, offset, target, highlights);
    return std::make_pair(
        std::move(node),
        source_replacement{original, std::move(result), std::move(highlights)});
  }

 private:
  inner_type _inner;
};

}  // namespace proto_gen

template <typename inner_type>
inline auto keyword(inner_type inner) {
  return proto_gen::highlight{highlight_token_type::keyword, std::move(inner)};
}
template <typename inner_type>
inline auto number(inner_type inner) {
  return proto_gen::highlight{highlight_token_type::number, std::move(inner)};
}
template <typename inner_type>
inline auto string(inner_type inner) {
  return proto_gen::highlight{highlight_token_type::string, std::move(inner)};
}
template <typename inner_type>
inline auto op(inner_type inner) {
  return proto_gen::highlight{highlight_token_type::op, std::move(inner)};
}

inline auto variable_placeholder(std::string name) {
  return proto_gen::node(
      [name](auto) { return ast::make<ast::variable_placeholder>(name); },
      proto_gen::highlight(highlight_token_type::placeholder,
                           "@" + std::move(name)));
}
inline auto expression_placeholder(std::string name) {
  return proto_gen::node(
      [name](auto) { return ast::make<ast::expression_placeholder>(name); },
      proto_gen::highlight(highlight_token_type::placeholder,
                           "@" + std::move(name)));
}

inline auto newline() { return proto_gen::newline{}; }

inline auto store_loc(source_loc& loc) { return proto_gen::store_loc{loc}; }

}  // namespace marlin::control

#endif  // marlin_control_proto_gen_hpp
