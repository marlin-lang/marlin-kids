#include <catch2/catch.hpp>

#include "ast.hpp"
#include "store.hpp"

TEST_CASE("store::Read unrecognized data", "[store]") {
  marlin::store::data_vector data;
  REQUIRE_THROWS(marlin::store::read(data));
}

TEST_CASE("store::Write and read expressions", "[store]") {
  auto node{marlin::ast::make<marlin::ast::binary_expression>(
      marlin::ast::make<marlin::ast::expression_placeholder>("left"),
      marlin::ast::binary_op::multiply,
      marlin::ast::make<marlin::ast::expression_placeholder>("right"))};
  auto data{marlin::store::write({node.get()})};

  auto result{marlin::store::read(data)};
  REQUIRE(result.nodes.size() == 1);
  REQUIRE(result.source == "@left * @right");

  auto& left = *result.nodes[0]->as<marlin::ast::binary_expression>().left();

  auto inner_node{marlin::ast::make<marlin::ast::binary_expression>(
      marlin::ast::make<marlin::ast::expression_placeholder>("left"),
      marlin::ast::binary_op::add,
      marlin::ast::make<marlin::ast::expression_placeholder>("right"))};
  auto inner_data{marlin::store::write({inner_node.get()})};

  auto inner_result{marlin::store::read(inner_data, left)};
  REQUIRE(inner_result.nodes.size() == 1);
  REQUIRE(inner_result.source == "(@left + @right)");
}

TEST_CASE("store::Write and read statements", "[store]") {
  auto outer_if{marlin::ast::make<marlin::ast::if_statement>(
      marlin::ast::make<marlin::ast::expression_placeholder>("condition"),
      std::vector<marlin::ast::node>{})};
  auto& outer_if_ref{*outer_if};
  std::vector<marlin::ast::node> on_start_statements;
  on_start_statements.emplace_back(std::move(outer_if));
  auto on_start{
      marlin::ast::make<marlin::ast::on_start>(std::move(on_start_statements))};
  std::vector<marlin::ast::node> program_blocks;
  program_blocks.emplace_back(std::move(on_start));
  auto program{
      marlin::ast::make<marlin::ast::program>(std::move(program_blocks))};

  auto assignment{marlin::ast::make<marlin::ast::assignment>(
      marlin::ast::make<marlin::ast::variable_name>("needs_print"),
      marlin::ast::make<marlin::ast::number_literal>("0"))};
  auto print{marlin::ast::make<marlin::ast::print_statement>(
      marlin::ast::make<marlin::ast::string_literal>("\"Hello, world!\""))};
  std::vector<marlin::ast::node> inner_if_statements;
  inner_if_statements.emplace_back(std::move(print));
  auto inner_if{marlin::ast::make<marlin::ast::if_else_statement>(
      marlin::ast::make<marlin::ast::identifier>("needs_print"),
      std::move(inner_if_statements), std::vector<marlin::ast::node>{})};

  auto data{marlin::store::write({assignment.get(), inner_if.get()})};
  auto result{marlin::store::read(data, outer_if_ref, 3)};
  REQUIRE(result.nodes.size() == 2);
  REQUIRE(result.source ==
          "    needs_print = 0;\n"
          "    if (needs_print) {\n"
          "      print(\"\\\"Hello, world!\\\"\");\n"
          "    } else {\n"
          "    }\n");
}

TEST_CASE("store::Report type error", "[store]") {
  auto node{marlin::ast::make<marlin::ast::binary_expression>(
      marlin::ast::make<marlin::ast::expression_placeholder>("left"),
      marlin::ast::binary_op::multiply,
      marlin::ast::make<marlin::ast::expression_placeholder>("right"))};
  auto data{marlin::store::write({node.get()})};

  auto result{marlin::store::read(data)};
  REQUIRE(result.nodes.size() == 1);
  REQUIRE(result.source == "@left * @right");

  auto& left = *result.nodes[0]->as<marlin::ast::binary_expression>().left();

  auto inner_node{marlin::ast::make<marlin::ast::if_statement>(
      marlin::ast::make<marlin::ast::expression_placeholder>("con"),
      std::vector<marlin::ast::node>{})};
  auto inner_data{marlin::store::write({inner_node.get()})};

  REQUIRE_THROWS(marlin::store::read(inner_data, left));
}
