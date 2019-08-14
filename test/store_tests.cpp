#include <catch2/catch.hpp>

#include "store.hpp"

// Testing
#include <iostream>

TEST_CASE("store::Read unrecognized data", "[store]") {
  REQUIRE_THROWS(marlin::store::read(""));
}

TEST_CASE("store::Write data", "[store]") {
  auto node{marlin::ast::make<marlin::ast::binary_expression>(
      marlin::ast::make<marlin::ast::expression_placeholder>("left"),
      marlin::ast::binary_op::multiply,
      marlin::ast::make<marlin::ast::expression_placeholder>("right"))};
  std::string data{marlin::store::write({node.get()})};

  std::cout << data << "\n";
}