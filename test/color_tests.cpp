#include <catch2/catch.hpp>

#include "expr_inserter.hpp"
#include "line_inserter.hpp"
#include "source_selection.hpp"
#include "store.hpp"

static auto make_test_document() {
  std::vector<marlin::ast::node> new_color_arguments;
  new_color_arguments.emplace_back(
      marlin::ast::make<marlin::ast::number_literal>("0"));
  new_color_arguments.emplace_back(
      marlin::ast::make<marlin::ast::expression_placeholder>("green"));
  new_color_arguments.emplace_back(
      marlin::ast::make<marlin::ast::number_literal>("255"));
  new_color_arguments.emplace_back(
      marlin::ast::make<marlin::ast::number_literal>("0.8"));
  auto new_color = marlin::ast::make<marlin::ast::new_color>(
      marlin::ast::color_mode::rgba, std::move(new_color_arguments));
  auto assignment = marlin::ast::make<marlin::ast::assignment>(
      marlin::ast::make<marlin::ast::variable_placeholder>("variable"),
      std::move(new_color));
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

TEST_CASE("control::Change color literal", "[control]") {
  auto result = make_test_document();
  REQUIRE(result.has_value());
  auto [document, init_data] = *std::move(result);
  marlin::control::source_selection selection{document, {2, 17}};
  REQUIRE(selection.is_color_literal());

  auto literal{selection.get_color_literal()};
  CHECK(literal.mode == marlin::ast::color_mode::rgba);

  marlin::control::color_literal new_literal{marlin::ast::color_mode::rgb};
  new_literal.set(255.0, 255.0, 0.0);

  auto update{selection.set_color_literal(new_literal)};
  REQUIRE(update.size() == 1);
  REQUIRE(update[0].display.source == "rgb(255, 255, 0)");
  REQUIRE(update[0].range.begin.line == 2);
  REQUIRE(update[0].range.begin.column == 15);
  REQUIRE(update[0].range.end.line == 2);
  REQUIRE(update[0].range.end.column == 40);
}
