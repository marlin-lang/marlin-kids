#include <catch2/catch.hpp>

#include "document.hpp"

TEST_CASE("control::Insert statement in empty document", "[control]") {
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