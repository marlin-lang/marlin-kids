#include "prototypes.hpp"

#include "ast.hpp"

namespace marlin::control {

const std::array<statement_prototype, 1> statement_prototypes = {
    {{"let", [](size_t line, size_t indent) {
        static constexpr const char* source_template =
            "let @variable = @value;";
        static const size_t source_template_length = strlen(source_template);

        static const size_t variable_start = strlen("let ");
        static const size_t variable_end = strlen("let @variable");
        static const size_t value_start = strlen("let @variable = ");
        static const size_t value_end = strlen("let @variable = @value");

        auto node{ast::make<ast::variable_declaration>(
            ast::make<ast::variable_placeholder>("variable"),
            ast::make<ast::expression_placeholder>("value"))};
        node->source_code_range = {{line, indent},
                                   {line, indent + source_template_length}};
        node->as<ast::variable_declaration>().variable()->source_code_range = {
            {line, indent + variable_start}, {line, indent + variable_end}};
        node->as<ast::variable_declaration>().value()->source_code_range = {
            {line, indent + value_start}, {line, indent + value_end}};

        std::string source;
        source.reserve(indent + source_template_length + 1);
        for (size_t i = 0; i < indent; i++) {
          source.append("  ");
        }
        source.append(source_template);
        source.append("\n");

        std::vector<highlight_token> highlights{
            highlight_token{highlight_token_type::placeholder,
                            indent + variable_start,
                            variable_end - variable_start},
            highlight_token{highlight_token_type::placeholder,
                            indent + value_start, value_end - value_start}};
        return std::pair{
            std::move(node),
            source_insertion{
                {line, 1}, std::move(source), std::move(highlights)}};
      }}}};

}  // namespace marlin::control
