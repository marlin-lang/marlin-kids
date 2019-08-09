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
  auto update = inserter.insert(marlin::control::statement_prototypes[0]);
  CHECK(update.source == "  let @variable = @value;\n");
}

TEST_CASE("control::Insert number literal to placeholder", "[control]") {
  auto [document, init_data] =
      marlin::control::document::make_document(nullptr, 0);
  auto inserter = marlin::control::statement_inserter{document};
  inserter.move_to_line(2);
  REQUIRE(inserter.can_insert());
  inserter.insert(marlin::control::statement_prototypes[0]);

  auto &placeholder = document.locate({2, 20});
  REQUIRE(placeholder.is<marlin::ast::expression_placeholder>());
  auto update =
      document.replace_placeholder_with_number_literal(placeholder, "12");
  REQUIRE(update.source == "12");
  REQUIRE(update.range.begin.line == 2);
  REQUIRE(update.range.begin.column == 19);
  REQUIRE(update.range.begin.line == 2);
  REQUIRE(update.range.begin.column == 21);

  auto &declaration = document.locate({2, 3});
  REQUIRE(declaration.is<marlin::ast::variable_declaration>());
  REQUIRE(declaration.source_code_range.end.column == 22);
}
