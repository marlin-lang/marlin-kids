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
  auto literal = marlin::ast::make<marlin::ast::number_literal>("12");
  literal->source_code_range = {{2, 19}, {2, 21}};
  auto replaced = document.replace_expression(placeholder, std::move(literal));
  REQUIRE(replaced->is<marlin::ast::expression_placeholder>());

  auto &declaration = document.locate({2, 3});
  REQUIRE(declaration.is<marlin::ast::variable_declaration>());
  REQUIRE(declaration.source_code_range.end.column == 22);
}
