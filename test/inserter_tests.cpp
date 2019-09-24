#include <catch2/catch.hpp>

#include "expr_inserter.hpp"
#include "line_inserter.hpp"
#include "source_selection.hpp"

const auto assignment_prototype{marlin::control::assignment_prototype()};
const auto negative_prototype{
    marlin::control::unary_prototype(marlin::ast::unary_op::negative)};
const auto add_prototype{
    marlin::control::binary_prototype(marlin::ast::binary_op::add)};
const auto subtract_prototype{
    marlin::control::binary_prototype(marlin::ast::binary_op::subtract)};
const auto multiply_prototype{
    marlin::control::binary_prototype(marlin::ast::binary_op::multiply)};

TEST_CASE("control::Insert statement in empty document", "[control]") {
  auto result = marlin::control::document::make_document();
  REQUIRE(result.has_value());
  auto [document, init_data] = *std::move(result);
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
  auto update = inserter.insert(assignment_prototype.data);
  CHECK(update.selection_update.has_value());
  REQUIRE(update.source_updates.size() == 1);
  CHECK(update.source_updates[0].display.source == "  @variable = @value;\n");
}

TEST_CASE("control::Insert number literal at placeholder", "[control]") {
  auto result = marlin::control::document::make_document();
  REQUIRE(result.has_value());
  auto [document, init_data] = *std::move(result);
  marlin::control::statement_inserter inserter{document};
  inserter.move_to_line(2);
  REQUIRE(inserter.can_insert());
  inserter.insert(assignment_prototype.data);

  marlin::control::source_selection selection{document, {2, 20}};
  REQUIRE(selection.is_literal());
  auto expr_inserter =
      std::move(selection)
          .as_inserter<marlin::control::pasteboard_t::expression>();
  REQUIRE(expr_inserter.can_insert());
  auto update =
      std::move(expr_inserter)
          .insert_literal(marlin::control::literal_data_type::number, "12");
  CHECK(update.selection_update.has_value());
  REQUIRE(update.source_updates.size() == 1);
  REQUIRE(update.source_updates[0].display.source == "12");
  REQUIRE(update.source_updates[0].range.begin.line == 2);
  REQUIRE(update.source_updates[0].range.begin.column == 15);
  REQUIRE(update.source_updates[0].range.end.line == 2);
  REQUIRE(update.source_updates[0].range.end.column == 21);

  auto &declaration = document.locate({2, 13});
  CHECK(declaration.is<marlin::ast::assignment>());
  CHECK(declaration.source_code_range.end.column == 18);
}

TEST_CASE("control::Insert string literal at placeholder", "[control]") {
  auto result = marlin::control::document::make_document();
  REQUIRE(result.has_value());
  auto [document, init_data] = *std::move(result);
  marlin::control::statement_inserter inserter{document};
  inserter.move_to_line(2);
  REQUIRE(inserter.can_insert());
  inserter.insert(assignment_prototype.data);

  marlin::control::source_selection selection{document, {2, 20}};
  REQUIRE(selection.is_literal());
  auto expr_inserter =
      std::move(selection)
          .as_inserter<marlin::control::pasteboard_t::expression>();
  REQUIRE(expr_inserter.can_insert());
  auto update =
      std::move(expr_inserter)
          .insert_literal(marlin::control::literal_data_type::string, "12");
  CHECK(update.selection_update.has_value());
  REQUIRE(update.source_updates.size() == 1);
  REQUIRE(update.source_updates[0].display.source == "\"12\"");
  REQUIRE(update.source_updates[0].range.begin.line == 2);
  REQUIRE(update.source_updates[0].range.begin.column == 15);
  REQUIRE(update.source_updates[0].range.end.line == 2);
  REQUIRE(update.source_updates[0].range.end.column == 21);

  auto &declaration = document.locate({2, 13});
  REQUIRE(declaration.is<marlin::ast::assignment>());
  REQUIRE(declaration.source_code_range.end.column == 20);
}

TEST_CASE("control::Insert binary expressions at placeholder", "[control]") {
  auto result = marlin::control::document::make_document();
  REQUIRE(result.has_value());
  auto [document, init_data] = *std::move(result);
  marlin::control::statement_inserter inserter{document};
  inserter.move_to_line(2);
  REQUIRE(inserter.can_insert());
  inserter.insert(assignment_prototype.data);

  marlin::control::expression_inserter expr_inserter{document};
  expr_inserter.move_to_loc({2, 20});
  REQUIRE(expr_inserter.can_insert());
  auto update = std::move(expr_inserter).insert(add_prototype.data);
  CHECK(update.selection_update.has_value());
  REQUIRE(update.source_updates.size() == 1);
  REQUIRE(update.source_updates[0].display.source == "@left + @right");
  REQUIRE(update.source_updates[0].range.begin.line == 2);
  REQUIRE(update.source_updates[0].range.begin.column == 15);
  REQUIRE(update.source_updates[0].range.end.line == 2);
  REQUIRE(update.source_updates[0].range.end.column == 21);

  expr_inserter = {document};
  expr_inserter.move_to_loc({2, 27});
  REQUIRE(expr_inserter.can_insert());
  auto inner_update = std::move(expr_inserter).insert(subtract_prototype.data);
  CHECK(inner_update.selection_update.has_value());
  REQUIRE(inner_update.source_updates.size() == 1);
  REQUIRE(inner_update.source_updates[0].display.source == "(@left - @right)");
  REQUIRE(inner_update.source_updates[0].range.begin.line == 2);
  REQUIRE(inner_update.source_updates[0].range.begin.column == 23);
  REQUIRE(inner_update.source_updates[0].range.end.line == 2);
  REQUIRE(inner_update.source_updates[0].range.end.column == 29);

  auto &declaration = document.locate({2, 13});
  REQUIRE(declaration.is<marlin::ast::assignment>());
  REQUIRE(declaration.source_code_range.end.column == 40);
}

TEST_CASE("control::Insert unary expressions at placeholder", "[control]") {
  auto result = marlin::control::document::make_document();
  REQUIRE(result.has_value());
  auto [document, init_data] = *std::move(result);
  marlin::control::statement_inserter inserter{document};
  inserter.move_to_line(2);
  REQUIRE(inserter.can_insert());
  inserter.insert(assignment_prototype.data);

  marlin::control::expression_inserter expr_inserter{document};
  expr_inserter.move_to_loc({2, 20});
  REQUIRE(expr_inserter.can_insert());
  auto update = std::move(expr_inserter).insert(negative_prototype.data);
  CHECK(update.selection_update.has_value());
  REQUIRE(update.source_updates.size() == 1);
  REQUIRE(update.source_updates[0].display.source == "-@argument");
  REQUIRE(update.source_updates[0].range.begin.line == 2);
  REQUIRE(update.source_updates[0].range.begin.column == 15);
  REQUIRE(update.source_updates[0].range.end.line == 2);
  REQUIRE(update.source_updates[0].range.end.column == 21);

  expr_inserter = {document};
  expr_inserter.move_to_loc({2, 19});
  REQUIRE(expr_inserter.can_insert());
  auto inner_update = std::move(expr_inserter).insert(multiply_prototype.data);
  CHECK(inner_update.selection_update.has_value());
  REQUIRE(inner_update.source_updates.size() == 1);
  REQUIRE(inner_update.source_updates[0].display.source == "(@left * @right)");
  REQUIRE(inner_update.source_updates[0].range.begin.line == 2);
  REQUIRE(inner_update.source_updates[0].range.begin.column == 16);
  REQUIRE(inner_update.source_updates[0].range.end.line == 2);
  REQUIRE(inner_update.source_updates[0].range.end.column == 25);

  auto &declaration = document.locate({2, 13});
  REQUIRE(declaration.is<marlin::ast::assignment>());
  REQUIRE(declaration.source_code_range.end.column == 33);
}
