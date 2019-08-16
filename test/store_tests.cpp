#include <catch2/catch.hpp>

#include "store.hpp"

TEST_CASE("store::Read unrecognized data", "[store]") {
  REQUIRE_THROWS(marlin::store::read(""));
}

TEST_CASE("store::Write and read data", "[store]") {
  auto node{marlin::ast::make<marlin::ast::binary_expression>(
      marlin::ast::make<marlin::ast::expression_placeholder>("left"),
      marlin::ast::binary_op::multiply,
      marlin::ast::make<marlin::ast::expression_placeholder>("right"))};
  std::string data{marlin::store::write({node.get()})};

  auto result{marlin::store::read(std::move(data))};
}