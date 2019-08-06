#include <catch2/catch.hpp>

#include "document.hpp"
#include "statement_inserter.hpp"

TEST_CASE("control::Manual insert statement", "[control]") {
  auto [document, init_data] =
      marlin::control::document::make_document(nullptr, 0);

  auto loc1 = document.find_statement_insert_location(1);
  CHECK_FALSE(loc1.has_value());

  auto loc2 = document.find_statement_insert_location(2);
  REQUIRE(loc2.has_value());
  auto loc = *std::move(loc2);
  REQUIRE(loc.index == 0);
  REQUIRE(loc.line == 2);

  auto loc3 = document.find_statement_insert_location(3);
  CHECK_FALSE(loc3.has_value());

  auto update =
      document.insert_statement(loc, marlin::control::statement_prototypes[0]);
  CHECK(update.source == "  let @variable = @value;\n");

  // After successful insertion
  auto loc4 = document.find_statement_insert_location(3);
  CHECK(loc4.has_value());
}

TEST_CASE("control::Insert statement via inserter", "[control]") {
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
