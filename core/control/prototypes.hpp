#ifndef marlin_control_prototypes_hpp
#define marlin_control_prototypes_hpp

#include "prototype_definition.hpp"

#include "prototypes.hpp"

#include "ast.hpp"

namespace marlin::control {

const std::string indent_str(indent_space_count, ' ');

template <size_t line_count>
static std::string generate_source(
    const char* const (&source_template)[line_count], size_t indent) {
  const auto indent_length = indent * indent_space_count;

  std::string source;
  size_t source_length{0};
  for (size_t i{0}; i < line_count; i++) {
    source_length += indent_length + strlen(source_template[i]) + 1;
  }
  source.reserve(source_length);

  std::string indent_full;
  for (size_t i = 0; i < indent; i++) {
    indent_full.append(indent_str);
  }
  for (size_t i = 0; i < line_count; i++) {
    source.append(indent_full);
    source.append(source_template[i]);
    source.append("\n");
  }
  return source;
}

struct assignment_prototype : statement_prototype::impl<assignment_prototype> {
  [[nodiscard]] std::string name() const override { return "assign"; }
  [[nodiscard]] std::pair<ast::node, source_insertion> construct(
      size_t line, size_t indent) const override {
    static constexpr const char* source_template[] = {"@variable = @value;"};

    static const size_t variable_start = strlen("");
    static const size_t variable_end = strlen("@variable");
    static const size_t value_start = strlen("@variable = ");
    static const size_t value_end = strlen("@variable = @value");
    static const size_t final_line_end = strlen("@variable = @value;");

    const auto indent_length = indent * indent_space_count;

    auto node{ast::make<ast::assignment>(
        ast::make<ast::variable_placeholder>("variable"),
        ast::make<ast::expression_placeholder>("value"))};
    node->source_code_range = {{line, 1 + indent_length},
                               {line, 1 + indent_length + final_line_end}};
    node->as<ast::assignment>().variable()->source_code_range = {
        {line, 1 + indent_length + variable_start},
        {line, 1 + indent_length + variable_end}};
    node->as<ast::assignment>().value()->source_code_range = {
        {line, 1 + indent_length + value_start},
        {line, 1 + indent_length + value_end}};

    std::vector<highlight_token> highlights{
        highlight_token{highlight_token_type::keyword, indent_length, 3},
        highlight_token{highlight_token_type::placeholder,
                        indent_length + variable_start,
                        variable_end - variable_start},
        highlight_token{highlight_token_type::placeholder,
                        indent_length + value_start, value_end - value_start}};
    return std::pair{std::move(node),
                     source_insertion{{line, 1},
                                      generate_source(source_template, indent),
                                      std::move(highlights)}};
  }
};

struct print_prototype : statement_prototype::impl<print_prototype> {
  [[nodiscard]] std::string name() const override { return "print"; }
  [[nodiscard]] std::pair<ast::node, source_insertion> construct(
      size_t line, size_t indent) const override {
    static constexpr const char* source_template[] = {"print(@value);"};

    static const size_t value_start = strlen("print(");
    static const size_t value_end = strlen("print(@value");
    static const size_t final_line_end = strlen("print(@value);");

    const auto indent_length = indent * indent_space_count;

    auto node{ast::make<ast::print_statement>(
        ast::make<ast::expression_placeholder>("value"))};
    node->source_code_range = {{line, 1 + indent_length},
                               {line, 1 + indent_length + final_line_end}};
    node->as<ast::print_statement>().value()->source_code_range = {
        {line, 1 + indent_length + value_start},
        {line, 1 + indent_length + value_end}};

    std::vector<highlight_token> highlights{
        highlight_token{highlight_token_type::placeholder,
                        indent_length + value_start, value_end - value_start}};
    return std::pair{std::move(node),
                     source_insertion{{line, 1},
                                      generate_source(source_template, indent),
                                      std::move(highlights)}};
  }
};

struct if_prototype : statement_prototype::impl<if_prototype> {
  [[nodiscard]] std::string name() const override { return "if"; }
  [[nodiscard]] std::pair<ast::node, source_insertion> construct(
      size_t line, size_t indent) const override {
    static constexpr const char* source_template[] = {"if (@condition) {", "}"};

    static const size_t condition_start = strlen("if (");
    static const size_t condition_end = strlen("if (@condition");

    const auto indent_length = indent * indent_space_count;

    auto node{ast::make<ast::if_statement>(
        ast::make<ast::expression_placeholder>("value"),
        std::vector<ast::node>{})};
    node->source_code_range = {{line, 1 + indent_length},
                               {line + 1, 1 + indent_length + 1}};
    node->as<ast::if_statement>().condition()->source_code_range = {
        {line, 1 + indent_length + condition_start},
        {line, 1 + indent_length + condition_end}};

    std::vector<highlight_token> highlights{
        highlight_token{highlight_token_type::keyword, indent_length, 2},
        highlight_token{highlight_token_type::placeholder,
                        indent_length + condition_start,
                        condition_end - condition_start}};
    return std::pair{std::move(node),
                     source_insertion{{line, 1},
                                      generate_source(source_template, indent),
                                      std::move(highlights)}};
  }
};

struct if_else_prototype : statement_prototype::impl<if_else_prototype> {
  [[nodiscard]] std::string name() const override { return "if-else"; }
  [[nodiscard]] std::pair<ast::node, source_insertion> construct(
      size_t line, size_t indent) const override {
    static constexpr const char* source_template[] = {"if (@condition) {",
                                                      "} else {", "}"};

    static const size_t condition_start = strlen("if (");
    static const size_t condition_end = strlen("if (@condition");
    static const size_t else_start = strlen("if (@condition) {\n} ");

    const auto indent_length = indent * indent_space_count;

    auto node{ast::make<ast::if_else_statement>(
        ast::make<ast::expression_placeholder>("value"),
        std::vector<ast::node>{}, std::vector<ast::node>{})};
    node->source_code_range = {{line, 1 + indent_length},
                               {line + 2, 1 + indent_length + 1}};
    node->as<ast::if_else_statement>().condition()->source_code_range = {
        {line, 1 + indent_length + condition_start},
        {line, 1 + indent_length + condition_end}};

    std::vector<highlight_token> highlights{
        highlight_token{highlight_token_type::keyword, indent_length, 2},
        highlight_token{highlight_token_type::placeholder,
                        indent_length + condition_start,
                        condition_end - condition_start},
        highlight_token{highlight_token_type::keyword,
                        indent_length * 2 + else_start, 4}};
    return std::pair{std::move(node),
                     source_insertion{{line, 1},
                                      generate_source(source_template, indent),
                                      std::move(highlights)}};
  }
};

}  // namespace marlin::control

#endif  // marlin_control_prototypes_hpp