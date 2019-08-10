#ifndef marlin_control_prototypes_hpp
#define marlin_control_prototypes_hpp

#include "proto_gen.hpp"
#include "prototype_definition.hpp"

namespace marlin::control {

struct assignment_prototype : statement_prototype::impl<assignment_prototype> {
  [[nodiscard]] std::string name() const override { return "assign"; }

  inline static const proto_gen::statement_generator generator{
      proto_gen::node{[](auto array) {
                        return ast::make<ast::assignment>(std::move(array[0]),
                                                          std::move(array[1]));
                      },
                      variable_placeholder("variable") + " = " +
                          expression_placeholder("value") + ";"}};
};

struct print_prototype : statement_prototype::impl<print_prototype> {
  [[nodiscard]] std::string name() const override { return "print"; }

  inline static const proto_gen::statement_generator generator{proto_gen::node{
      [](auto array) {
        return ast::make<ast::print_statement>(std::move(array[0]));
      },
      "print(" + expression_placeholder("value") + ");"}};
};

struct if_prototype : statement_prototype::impl<if_prototype> {
  [[nodiscard]] std::string name() const override { return "if"; }

  inline static const proto_gen::statement_generator generator{proto_gen::node{
      [](auto array) {
        return ast::make<ast::if_statement>(std::move(array[0]),
                                            std::vector<ast::node>{});
      },
      keyword("if") + " (" + expression_placeholder("condition") + ") {" +
          newline() + "}"}};
};

struct if_else_prototype : statement_prototype::impl<if_else_prototype> {
  [[nodiscard]] std::string name() const override { return "if-else"; }

  inline static source_loc else_loc;
  inline static const proto_gen::statement_generator generator{proto_gen::node{
      [](auto array) {
        ast::node node{ast::make<ast::if_else_statement>(
            std::move(array[0]), std::vector<ast::node>{},
            std::vector<ast::node>{})};
        node->as<ast::if_else_statement>().else_loc = else_loc;
        return node;
      },
      keyword("if") + " (" + expression_placeholder("condition") + ") {" +
          newline() + "} " + store_loc(else_loc) + keyword("else") + " {" +
          newline() + "}"}};
};

}  // namespace marlin::control

#endif  // marlin_control_prototypes_hpp