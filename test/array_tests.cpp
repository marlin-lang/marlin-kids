#include <catch2/catch.hpp>

#include "expr_inserter.hpp"
#include "line_inserter.hpp"
#include "source_selection.hpp"
#include "store.hpp"

static auto make_test_document() {
  std::vector<marlin::ast::node> array_elements;
  array_elements.emplace_back(
      marlin::ast::make<marlin::ast::number_literal>("1"));
  array_elements.emplace_back(
      marlin::ast::make<marlin::ast::expression_placeholder>("elem1"));
  array_elements.emplace_back(
      marlin::ast::make<marlin::ast::expression_placeholder>("elem2"));
  array_elements.emplace_back(
      marlin::ast::make<marlin::ast::string_literal>("x"));
  array_elements.emplace_back(marlin::ast::make<marlin::ast::identifier>("m"));
  auto new_array =
      marlin::ast::make<marlin::ast::new_array>(std::move(array_elements));
  auto assignment = marlin::ast::make<marlin::ast::assignment>(
      marlin::ast::make<marlin::ast::variable_placeholder>("variable"),
      std::move(new_array));
  std::vector<marlin::ast::node> on_start_statements;
  on_start_statements.emplace_back(std::move(assignment));
  auto on_start{
      marlin::ast::make<marlin::ast::on_start>(std::move(on_start_statements))};
  std::vector<marlin::ast::node> program_blocks;
  program_blocks.emplace_back(std::move(on_start));
  auto program{
      marlin::ast::make<marlin::ast::program>(std::move(program_blocks))};
  auto data{marlin::store::write({program.get()})};

  return marlin::control::document::make_document(std::move(data));
}

TEST_CASE("control::Get new_array element count", "[control]") {
  auto result = make_test_document();
  REQUIRE(result.has_value());
  auto [document, init_data] = *std::move(result);
  marlin::control::source_selection selection{document, {2, 17}};
  REQUIRE(selection.is_new_array());
  CHECK(selection.get_new_array_element_count() == 5);
  CHECK(selection.get_new_array_minimum_count() == 3);
}

TEST_CASE("control::Increase new_array element count", "[control]") {
  auto result = make_test_document();
  REQUIRE(result.has_value());
  auto [document, init_data] = *std::move(result);
  marlin::control::source_selection selection{document, {2, 17}};
  REQUIRE(selection.is_new_array());

  auto update{selection.set_new_array_elements_count(7)};
  REQUIRE(update.size() == 1);
  REQUIRE(update[0].display.source ==
          "array(1, @elem1, @elem2, \"x\", m, @elem5, @elem6)");
  REQUIRE(update[0].range.begin.line == 2);
  REQUIRE(update[0].range.begin.column == 15);
  REQUIRE(update[0].range.end.line == 2);
  REQUIRE(update[0].range.end.column == 47);
}

TEST_CASE("control::Decrease new_array element count", "[control]") {
  auto result = make_test_document();
  REQUIRE(result.has_value());
  auto [document, init_data] = *std::move(result);
  marlin::control::source_selection selection{document, {2, 17}};
  REQUIRE(selection.is_new_array());

  auto update{selection.set_new_array_elements_count(4)};
  REQUIRE(update.size() == 1);
  REQUIRE(update[0].display.source == "array(1, @elem1, \"x\", m)");
  REQUIRE(update[0].range.begin.line == 2);
  REQUIRE(update[0].range.begin.column == 15);
  REQUIRE(update[0].range.end.line == 2);
  REQUIRE(update[0].range.end.column == 47);
}
