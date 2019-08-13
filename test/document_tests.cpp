#include <catch2/catch.hpp>

#include "expression_inserter.hpp"
#include "source_selection.hpp"
#include "statement_inserter.hpp"

TEST_CASE("control::Insert statement in empty document", "[control]") {
  auto [document, init_data] =
      marlin::control::document::make_document(nullptr, 0);
  marlin::control::statement_inserter inserter{document};

  CHECK_FALSE(inserter.can_insert());
  inserter.move_to_line(1);
  CHECK_FALSE(inserter.can_insert());
  inserter.move_to_line(2);
  CHECK(inserter.can_insert());
  inserter.move_to_line(3);
  CHECK_FALSE(inserter.can_insert());

  inserter.move_to_line(2);
  REQUIRE(inserter.can_insert());
  auto update =
      inserter.insert_prototype(marlin::control::assignment_prototype::index());
  CHECK(update.source == "  @variable = @value;\n");
}

TEST_CASE("control::Insert number literal at placeholder", "[control]") {
  auto [document, init_data] =
      marlin::control::document::make_document(nullptr, 0);
  marlin::control::statement_inserter inserter{document};
  inserter.move_to_line(2);
  REQUIRE(inserter.can_insert());
  inserter.insert_prototype(marlin::control::assignment_prototype::index());

  marlin::control::source_selection selection{document, {2, 20}};
  REQUIRE(selection.is_literal());
  auto expr_inserter = std::move(selection).as_expression_inserter();
  REQUIRE(expr_inserter.can_insert());
  auto update = expr_inserter.insert_literal(
      marlin::control::literal_data_type::number, "12");
  REQUIRE(update.source == "12");
  REQUIRE(update.range.begin.line == 2);
  REQUIRE(update.range.begin.column == 15);
  REQUIRE(update.range.end.line == 2);
  REQUIRE(update.range.end.column == 21);

  auto &declaration = document.locate({2, 13});
  REQUIRE(declaration.is<marlin::ast::assignment>());
  REQUIRE(declaration.source_code_range.end.column == 18);
}

TEST_CASE("control::Insert string literal at placeholder", "[control]") {
  auto [document, init_data] =
      marlin::control::document::make_document(nullptr, 0);
  marlin::control::statement_inserter inserter{document};
  inserter.move_to_line(2);
  REQUIRE(inserter.can_insert());
  inserter.insert_prototype(marlin::control::assignment_prototype::index());

  marlin::control::source_selection selection{document, {2, 20}};
  REQUIRE(selection.is_literal());
  auto expr_inserter = std::move(selection).as_expression_inserter();
  REQUIRE(expr_inserter.can_insert());
  auto update = expr_inserter.insert_literal(
      marlin::control::literal_data_type::string, "12");
  REQUIRE(update.source == "\"12\"");
  REQUIRE(update.range.begin.line == 2);
  REQUIRE(update.range.begin.column == 15);
  REQUIRE(update.range.end.line == 2);
  REQUIRE(update.range.end.column == 21);

  auto &declaration = document.locate({2, 13});
  REQUIRE(declaration.is<marlin::ast::assignment>());
  REQUIRE(declaration.source_code_range.end.column == 20);
}

TEST_CASE("control::Insert binary expressions at placeholder", "[control]") {
  auto [document, init_data] =
      marlin::control::document::make_document(nullptr, 0);
  marlin::control::statement_inserter inserter{document};
  inserter.move_to_line(2);
  REQUIRE(inserter.can_insert());
  inserter.insert_prototype(marlin::control::assignment_prototype::index());

  marlin::control::expression_inserter expr_inserter{document};
  expr_inserter.move_to_loc({2, 20});
  REQUIRE(expr_inserter.can_insert());
  auto update = expr_inserter.insert_prototype(
      marlin::control::binary_prototype<marlin::ast::binary_op::add>::index());
  REQUIRE(update.source == "@left + @right");
  REQUIRE(update.range.begin.line == 2);
  REQUIRE(update.range.begin.column == 15);
  REQUIRE(update.range.end.line == 2);
  REQUIRE(update.range.end.column == 21);

  expr_inserter = {document};
  expr_inserter.move_to_loc({2, 27});
  REQUIRE(expr_inserter.can_insert());
  auto inner_update = expr_inserter.insert_prototype(
      marlin::control::binary_prototype<
          marlin::ast::binary_op::subtract>::index());
  REQUIRE(inner_update.source == "(@left - @right)");
  REQUIRE(inner_update.range.begin.line == 2);
  REQUIRE(inner_update.range.begin.column == 23);
  REQUIRE(inner_update.range.end.line == 2);
  REQUIRE(inner_update.range.end.column == 29);

  auto &declaration = document.locate({2, 13});
  REQUIRE(declaration.is<marlin::ast::assignment>());
  REQUIRE(declaration.source_code_range.end.column == 40);
}

TEST_CASE("control::Insert unary expressions at placeholder", "[control]") {
  auto [document, init_data] =
      marlin::control::document::make_document(nullptr, 0);
  marlin::control::statement_inserter inserter{document};
  inserter.move_to_line(2);
  REQUIRE(inserter.can_insert());
  inserter.insert_prototype(marlin::control::assignment_prototype::index());

  marlin::control::expression_inserter expr_inserter{document};
  expr_inserter.move_to_loc({2, 20});
  REQUIRE(expr_inserter.can_insert());
  auto update =
      expr_inserter.insert_prototype(marlin::control::unary_prototype<
                                     marlin::ast::unary_op::negative>::index());
  REQUIRE(update.source == "-@argument");
  REQUIRE(update.range.begin.line == 2);
  REQUIRE(update.range.begin.column == 15);
  REQUIRE(update.range.end.line == 2);
  REQUIRE(update.range.end.column == 21);

  expr_inserter = {document};
  expr_inserter.move_to_loc({2, 19});
  REQUIRE(expr_inserter.can_insert());
  auto inner_update = expr_inserter.insert_prototype(
      marlin::control::binary_prototype<
          marlin::ast::binary_op::multiply>::index());
  REQUIRE(inner_update.source == "(@left * @right)");
  REQUIRE(inner_update.range.begin.line == 2);
  REQUIRE(inner_update.range.begin.column == 16);
  REQUIRE(inner_update.range.end.line == 2);
  REQUIRE(inner_update.range.end.column == 25);

  auto &declaration = document.locate({2, 13});
  REQUIRE(declaration.is<marlin::ast::assignment>());
  REQUIRE(declaration.source_code_range.end.column == 33);
}

TEST_CASE("control::Remove expressions", "[control]") {
  auto [document, init_data] =
      marlin::control::document::make_document(nullptr, 0);
  marlin::control::statement_inserter inserter{document};
  inserter.move_to_line(2);
  REQUIRE(inserter.can_insert());
  inserter.insert_prototype(marlin::control::if_prototype::index());

  marlin::control::expression_inserter expr_inserter{document};
  expr_inserter.move_to_loc({2, 7});
  REQUIRE(expr_inserter.can_insert());
  auto update = expr_inserter.insert_literal(
      marlin::control::literal_data_type::number, "12");

  auto &literal = document.locate({2, 7});
  REQUIRE(literal.is<marlin::ast::number_literal>());
  auto [node, remove_update] = document.remove_expression(literal);
  REQUIRE(remove_update.source == "@condition");
  REQUIRE(remove_update.range.begin.line == 2);
  REQUIRE(remove_update.range.begin.column == 7);
  REQUIRE(remove_update.range.end.line == 2);
  REQUIRE(remove_update.range.end.column == 9);
}
