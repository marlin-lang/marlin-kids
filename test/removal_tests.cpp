#include <catch2/catch.hpp>

#include "expr_inserter.hpp"
#include "line_inserter.hpp"
#include "source_selection.hpp"

const auto if_prototype{marlin::control::if_prototype()};

TEST_CASE("control::Remove expressions", "[control]") {
  auto result = marlin::control::document::make_document();
  REQUIRE(result.has_value());
  auto [document, init_data] = *std::move(result);
  marlin::control::statement_inserter inserter{document};
  inserter.move_to_line(2);
  REQUIRE(inserter.can_insert());
  inserter.insert(if_prototype.data);

  marlin::control::expression_inserter expr_inserter{document};
  expr_inserter.move_to_loc({2, 7});
  REQUIRE(expr_inserter.can_insert());
  std::move(expr_inserter)
      .insert_literal(marlin::control::literal_data_type::number, "12");

  marlin::control::source_selection literal_selection{document, {2, 7}};
  REQUIRE(literal_selection.is_literal());
  auto removal = std::move(literal_selection).remove_from_document();
  REQUIRE(removal.selection_update.has_value());
  REQUIRE(removal.source_updates.size() == 1);
  auto remove_update = std::move(removal.source_updates[0]);
  REQUIRE(remove_update.display.source == "@condition");
  REQUIRE(remove_update.range.begin.line == 2);
  REQUIRE(remove_update.range.begin.column == 7);
  REQUIRE(remove_update.range.end.line == 2);
  REQUIRE(remove_update.range.end.column == 9);
}
