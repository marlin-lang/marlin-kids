#include "node.hpp"

#include "base.hpp"

namespace marlin::ast {

void base_deleter::operator()(base *b) {
  b->apply<void>([](auto &node) { delete &node; });
}

}  // namespace marlin::ast