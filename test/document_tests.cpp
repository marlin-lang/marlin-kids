#include <catch2/catch.hpp>

#include "document.hpp"
#include "statement_inserter.hpp"

TEST_CASE("control::Insert statement in empty document", "[control]") {
  auto [document, init_data] =
      marlin::control::document::make_document(nullptr, 0);
  auto inserter = marlin::control::statement_inserter{document};

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
      inserter.insert(*marlin::control::statement_prototypes
                          [marlin::control::assignment_prototype::index()]);
  CHECK(update.source == "  @variable = @value;\n");
}

TEST_CASE("control::Insert number literal at placeholder", "[control]") {
  auto [document, init_data] =
      marlin::control::document::make_document(nullptr, 0);
  auto inserter = marlin::control::statement_inserter{document};
  inserter.move_to_line(2);
  REQUIRE(inserter.can_insert());
  inserter.insert(*marlin::control::statement_prototypes
                      [marlin::control::assignment_prototype::index()]);

  auto &placeholder = document.locate({2, 20});
  REQUIRE(placeholder.is<marlin::ast::expression_placeholder>());
  auto update =
      document
          .replace_with_literal_prototype<marlin::control::number_prototype>(
              placeholder, "12");
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
  auto inserter = marlin::control::statement_inserter{document};
  inserter.move_to_line(2);
  REQUIRE(inserter.can_insert());
  inserter.insert(*marlin::control::statement_prototypes
                      [marlin::control::assignment_prototype::index()]);

  auto &placeholder = document.locate({2, 20});
  REQUIRE(placeholder.is<marlin::ast::expression_placeholder>());
  auto update =
      document
          .replace_with_literal_prototype<marlin::control::string_prototype>(
              placeholder, "12");
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
  auto inserter = marlin::control::statement_inserter{document};
  inserter.move_to_line(2);
  REQUIRE(inserter.can_insert());
  inserter.insert(*marlin::control::statement_prototypes
                      [marlin::control::assignment_prototype::index()]);

  auto &placeholder = document.locate({2, 20});
  REQUIRE(placeholder.is<marlin::ast::expression_placeholder>());
  auto update = document.replace_with_expression_prototype(
      placeholder,
      *marlin::control::expression_prototypes[marlin::control::binary_prototype<
          marlin::ast::binary_op::add>::index()]);
  REQUIRE(update.source == "@left + @right");
  REQUIRE(update.range.begin.line == 2);
  REQUIRE(update.range.begin.column == 15);
  REQUIRE(update.range.end.line == 2);
  REQUIRE(update.range.end.column == 21);

  auto &inner_placeholder = document.locate({2, 27});
  REQUIRE(inner_placeholder.is<marlin::ast::expression_placeholder>());
  auto inner_update = document.replace_with_expression_prototype(
      inner_placeholder,
      *marlin::control::expression_prototypes[marlin::control::binary_prototype<
          marlin::ast::binary_op::subtract>::index()]);
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
  auto inserter = marlin::control::statement_inserter{document};
  inserter.move_to_line(2);
  REQUIRE(inserter.can_insert());
  inserter.insert(*marlin::control::statement_prototypes
                      [marlin::control::assignment_prototype::index()]);

  auto &placeholder = document.locate({2, 20});
  REQUIRE(placeholder.is<marlin::ast::expression_placeholder>());
  auto update = document.replace_with_expression_prototype(
      placeholder,
      *marlin::control::expression_prototypes[marlin::control::unary_prototype<
          marlin::ast::unary_op::negative>::index()]);
  REQUIRE(update.source == "-@argument");
  REQUIRE(update.range.begin.line == 2);
  REQUIRE(update.range.begin.column == 15);
  REQUIRE(update.range.end.line == 2);
  REQUIRE(update.range.end.column == 21);

  auto &inner_placeholder = document.locate({2, 19});
  REQUIRE(inner_placeholder.is<marlin::ast::expression_placeholder>());
  auto inner_update = document.replace_with_expression_prototype(
      inner_placeholder,
      *marlin::control::expression_prototypes[marlin::control::binary_prototype<
          marlin::ast::binary_op::multiply>::index()]);
  REQUIRE(inner_update.source == "(@left * @right)");
  REQUIRE(inner_update.range.begin.line == 2);
  REQUIRE(inner_update.range.begin.column == 16);
  REQUIRE(inner_update.range.end.line == 2);
  REQUIRE(inner_update.range.end.column == 25);

  auto &declaration = document.locate({2, 13});
  REQUIRE(declaration.is<marlin::ast::assignment>());
  REQUIRE(declaration.source_code_range.end.column == 33);
}
