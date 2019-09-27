#ifndef marlin_control_literal_content_hpp
#define marlin_control_literal_content_hpp

#include <string>
#include <utility>

#include "ast.hpp"
#include "node.hpp"
#include "placeholders.hpp"

namespace marlin::control {

enum struct literal_data_type {
  parameter,
  variable_name,
  identifier,
  number,
  string
};

struct literal_content {
  static constexpr auto any_default_type{literal_data_type::number};
  static constexpr auto number_default_type{literal_data_type::number};
  static constexpr auto boolean_default_type{literal_data_type::identifier};
  static constexpr auto array_default_type{literal_data_type::identifier};
  static constexpr auto color_default_type{literal_data_type::identifier};

  template <typename node_type>
  static literal_data_type get_default_type(ast::base::child_index) {
    return any_default_type;
  }

  template <typename node_type>
  static literal_data_type get_default_type(const node_type& parent,
                                            ast::base::child_index index) {
    return get_default_type<node_type>(std::move(index));
  }

  static literal_data_type get_default_type_for_node(const ast::base& node) {
    if (node.has_parent()) {
      auto& parent{node.parent()};
      return parent.apply<literal_data_type>([&node](auto& n) {
        return get_default_type(n, n.index_for_child(node));
      });
    } else {
      return any_default_type;
    }
  }

  literal_data_type type;
  std::string content;

  literal_content(literal_data_type _type, std::string _content)
      : type{_type}, content{std::move(_content)} {}
};

inline literal_data_type _get_control_flow_default_type(
    ast::base::child_index index) {
  assert(index.subnode_index == 0 && index.node_index == 0);
  return literal_content::boolean_default_type;
}
template <>
inline literal_data_type literal_content::get_default_type<ast::if_statement>(
    ast::base::child_index index) {
  return _get_control_flow_default_type(index);
}
template <>
inline literal_data_type
literal_content::get_default_type<ast::if_else_statement>(
    ast::base::child_index index) {
  return _get_control_flow_default_type(index);
}
template <>
inline literal_data_type
literal_content::get_default_type<ast::while_statement>(
    ast::base::child_index index) {
  return _get_control_flow_default_type(index);
}

template <>
inline literal_data_type literal_content::get_default_type<ast::for_statement>(
    ast::base::child_index index) {
  assert(index.subnode_index == 1 && index.node_index == 0);
  return array_default_type;
}

template <>
inline literal_data_type
literal_content::get_default_type<ast::unary_expression>(
    const ast::unary_expression& parent, ast::base::child_index index) {
  static constexpr auto unary_op_literal_type_map{
      make_array(literal_content::number_default_type /* negative */,
                 literal_content::boolean_default_type /* logical_not */)};
  assert(index.subnode_index == 0 && index.node_index == 0);
  return unary_op_literal_type_map[raw_value(parent.op)];
}

template <>
inline literal_data_type
literal_content::get_default_type<ast::binary_expression>(
    const ast::binary_expression& parent, ast::base::child_index index) {
  static constexpr auto binary_op_literal_type_map{
      make_array(literal_content::number_default_type /* add */,
                 literal_content::number_default_type /* subtract */,
                 literal_content::number_default_type /* multiply */,
                 literal_content::number_default_type /* divide */,
                 literal_content::any_default_type /* equal */,
                 literal_content::any_default_type /* not_equal */,
                 literal_content::number_default_type /* less */,
                 literal_content::number_default_type /* less_equal */,
                 literal_content::number_default_type /* greater */,
                 literal_content::number_default_type /* greater_equal */,
                 literal_content::boolean_default_type /* logical_and */,
                 literal_content::boolean_default_type /* logical_or */)};
  assert(index.subnode_index < 2 && index.node_index == 0);
  return binary_op_literal_type_map[raw_value(parent.op)];
}

inline literal_data_type _get_subscript_default_type(
    ast::base::child_index index) {
  static constexpr auto subnodes{
      make_array(literal_content::array_default_type,
                 literal_content::number_default_type)};
  assert(index.subnode_index < subnodes.size() && index.node_index == 0);
  return subnodes[index.subnode_index];
}
template <>
inline literal_data_type literal_content::get_default_type<ast::subscript_set>(
    ast::base::child_index index) {
  return _get_subscript_default_type(index);
}
template <>
inline literal_data_type literal_content::get_default_type<ast::subscript_get>(
    ast::base::child_index index) {
  return _get_subscript_default_type(index);
}

struct array_modification_args_literal_type {
 private:
  static const auto& literal_types() {
    static const auto _literal_types{make_array(
        std::vector{literal_content::any_default_type} /* append */,
        std::vector{literal_content::number_default_type,
                    literal_content::any_default_type} /* insert */,
        std::vector{literal_content::number_default_type} /* remove */)};
    return _literal_types;
  }

 public:
  static const auto& args(ast::array_modification mod) {
    return literal_types()[raw_value(mod)];
  }
};

template <>
inline literal_data_type literal_content::get_default_type<ast::modify_array>(
    const ast::modify_array& parent, ast::base::child_index index) {
  const auto& args{array_modification_args_literal_type::args(parent.mod)};
  assert(index.subnode_index == 1 && index.node_index < args.size());
  return args[index.node_index];
}

struct system_procedure_args_literal_type {
 private:
  static const auto& literal_types() {
    static const auto _literal_types{make_array(
        std::vector{literal_content::any_default_type} /* sleep */,
        std::vector{literal_content::any_default_type} /* print */,
        std::vector{literal_content::number_default_type,
                    literal_content::number_default_type,
                    literal_content::number_default_type,
                    literal_content::number_default_type} /* draw_line */,
        std::vector{literal_content::number_default_type,
                    literal_content::number_default_type,
                    literal_content::number_default_type,
                    literal_content::number_default_type,
                    literal_content::number_default_type} /* draw_arc */,
        std::vector{literal_content::number_default_type,
                    literal_content::number_default_type,
                    literal_content::number_default_type,
                    literal_content::number_default_type} /* draw_rect */,
        std::vector{literal_content::number_default_type,
                    literal_content::number_default_type,
                    literal_content::number_default_type,
                    literal_content::number_default_type} /* draw_ellipse */,
        std::vector{literal_content::color_default_type} /* clear_canvas */,
        std::vector{literal_content::number_default_type} /* set_line_width */,
        std::vector{literal_content::color_default_type} /* set_line_color */,
        std::vector{literal_content::color_default_type} /* set_fill_color */,
        std::vector{literal_content::number_default_type} /* logo_forward */,
        std::vector{literal_content::number_default_type} /* logo_backward */,
        std::vector{literal_content::number_default_type} /* logo_turn_left */,
        std::vector{literal_content::number_default_type} /* logo_turn_right */,
        std::vector<literal_data_type>{} /* logo_pen_up */,
        std::vector<literal_data_type>{} /* logo_pen_down */,
        std::vector<literal_data_type>{} /* logo_go_home */)};
    return _literal_types;
  }

 public:
  static const auto& args(ast::system_procedure func) {
    return literal_types()[raw_value(func)];
  }
};

template <>
inline literal_data_type
literal_content::get_default_type<ast::system_procedure_call>(
    const ast::system_procedure_call& parent, ast::base::child_index index) {
  const auto& args{system_procedure_args_literal_type::args(parent.proc)};
  assert(index.subnode_index == 0 && index.node_index < args.size());
  return args[index.node_index];
}

template <>
inline literal_data_type literal_content::get_default_type<ast::new_color>(
    const ast::new_color& init, ast::base::child_index index) {
  assert(index.subnode_index == 0 &&
         index.node_index < placeholder_new_color_args::args(init.mode).size());
  return number_default_type;
}

struct system_function_args_literal_type {
 private:
  static const auto& literal_types() {
    static const auto _literal_types{make_array(
        std::vector{literal_content::number_default_type} /* range1 */,
        std::vector{literal_content::number_default_type,
                    literal_content::number_default_type} /* range2 */,
        std::vector{literal_content::number_default_type,
                    literal_content::number_default_type,
                    literal_content::number_default_type} /* range3 */,
        std::vector{literal_content::number_default_type,
                    literal_content::number_default_type} /* random */,
        std::vector{literal_content::array_default_type} /* list_length */,
        std::vector<literal_data_type>{} /* time */,
        std::vector{literal_content::number_default_type} /* abs */,
        std::vector{literal_content::number_default_type} /* sqrt */,
        std::vector{literal_content::number_default_type} /* sin */,
        std::vector{literal_content::number_default_type} /* cos */,
        std::vector{literal_content::number_default_type} /* tan */,
        std::vector{literal_content::number_default_type} /* asin */,
        std::vector{literal_content::number_default_type} /* acos */,
        std::vector{literal_content::number_default_type} /* atan */,
        std::vector{literal_content::number_default_type} /* ln */,
        std::vector{literal_content::number_default_type} /* log */,
        std::vector{literal_content::number_default_type} /* round */,
        std::vector{literal_content::number_default_type} /* floor */,
        std::vector{literal_content::number_default_type} /* ceil */)};
    return _literal_types;
  }

 public:
  static const auto& args(ast::system_function func) {
    return literal_types()[raw_value(func)];
  }
};

template <>
inline literal_data_type
literal_content::get_default_type<ast::system_function_call>(
    const ast::system_function_call& parent, ast::base::child_index index) {
  const auto& args{system_function_args_literal_type::args(parent.func)};
  assert(index.subnode_index == 0 && index.node_index < args.size());
  return args[index.node_index];
}

};  // namespace marlin::control

#endif  // marlin_control_literal_content_hpp