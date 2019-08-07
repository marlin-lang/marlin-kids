#include "prototypes.hpp"

#include "ast.hpp"

namespace marlin::control {

// TODO: extract common patterns in statement constructors
const std::array<statement_prototype, 4> statement_prototypes = {
    {{"let",
      [](size_t line, size_t indent) {
        static constexpr const char* source_template =
            "let @variable = @value;";
        static const size_t source_template_length = strlen(source_template);

        static const size_t variable_start = strlen("let ");
        static const size_t variable_end = strlen("let @variable");
        static const size_t value_start = strlen("let @variable = ");
        static const size_t value_end = strlen("let @variable = @value");

        const auto indent_length = indent * 2;

        auto node{ast::make<ast::variable_declaration>(
            ast::make<ast::variable_placeholder>("variable"),
            ast::make<ast::expression_placeholder>("value"))};
        node->source_code_range = {
            {line, indent_length},
            {line, indent_length + source_template_length}};
        node->as<ast::variable_declaration>().variable()->source_code_range = {
            {line, indent_length + variable_start},
            {line, indent_length + variable_end}};
        node->as<ast::variable_declaration>().value()->source_code_range = {
            {line, indent_length + value_start},
            {line, indent_length + value_end}};

        std::string source;
        source.reserve(indent_length + source_template_length + 1);
        for (size_t i = 0; i < indent; i++) {
          source.append("  ");
        }
        source.append(source_template);
        source.append("\n");

        std::vector<highlight_token> highlights{
            highlight_token{highlight_token_type::keyword, indent_length, 3},
            highlight_token{highlight_token_type::placeholder,
                            indent_length + variable_start,
                            variable_end - variable_start},
            highlight_token{highlight_token_type::placeholder,
                            indent_length + value_start,
                            value_end - value_start}};
        return std::pair{
            std::move(node),
            source_insertion{
                {line, 1}, std::move(source), std::move(highlights)}};
      }},
     {"print",
      [](size_t line, size_t indent) {
        static constexpr const char* source_template = "print(@value);";
        static const size_t source_template_length = strlen(source_template);

        static const size_t value_start = strlen("print(");
        static const size_t value_end = strlen("print(@value");

        const auto indent_length = indent * 2;

        auto node{ast::make<ast::print_statement>(
            ast::make<ast::expression_placeholder>("value"))};
        node->source_code_range = {
            {line, indent_length},
            {line, indent_length + source_template_length}};
        node->as<ast::print_statement>().value()->source_code_range = {
            {line, indent_length + value_start},
            {line, indent_length + value_end}};

        std::string source;
        source.reserve(indent_length + source_template_length + 1);
        for (size_t i = 0; i < indent; i++) {
          source.append("  ");
        }
        source.append(source_template);
        source.append("\n");

        std::vector<highlight_token> highlights{highlight_token{
            highlight_token_type::placeholder, indent_length + value_start,
            value_end - value_start}};
        return std::pair{
            std::move(node),
            source_insertion{
                {line, 1}, std::move(source), std::move(highlights)}};
      }},
     {"if",
      [](size_t line, size_t indent) {
        static constexpr const char* source_template1 = "if (@condition) {";
        static constexpr const char* source_template2 = "}";
        static const size_t source_template_length1 = strlen(source_template1);
        static const size_t source_template_length2 = strlen(source_template2);

        static const size_t condition_start = strlen("if (");
        static const size_t condition_end = strlen("if (@condition");

        const auto indent_length = indent * 2;

        auto node{ast::make<ast::if_statement>(
            ast::make<ast::expression_placeholder>("value"),
            std::vector<ast::node>{})};
        node->source_code_range = {{line, indent_length},
                                   {line + 1, indent_length + 1}};
        node->as<ast::if_statement>().condition()->source_code_range = {
            {line, indent_length + condition_start},
            {line, indent_length + condition_end}};

        std::string source;
        source.reserve(indent_length + source_template_length1 + 1 +
                       indent_length + source_template_length2 + 1);
        for (size_t i = 0; i < indent; i++) {
          source.append("  ");
        }
        source.append(source_template1);
        source.append("\n");
        for (size_t i = 0; i < indent; i++) {
          source.append("  ");
        }
        source.append(source_template2);
        source.append("\n");

        std::vector<highlight_token> highlights{
            highlight_token{highlight_token_type::keyword, indent_length, 2},
            highlight_token{highlight_token_type::placeholder,
                            indent_length + condition_start,
                            condition_end - condition_start}};
        return std::pair{
            std::move(node),
            source_insertion{
                {line, 1}, std::move(source), std::move(highlights)}};
      }},
     {"if-else", [](size_t line, size_t indent) {
        static constexpr const char* source_template1 = "if (@condition) {";
        static constexpr const char* source_template2 = "} else {";
        static constexpr const char* source_template3 = "}";
        static const size_t source_template_length1 = strlen(source_template1);
        static const size_t source_template_length2 = strlen(source_template2);
        static const size_t source_template_length3 = strlen(source_template3);

        static const size_t condition_start = strlen("if (");
        static const size_t condition_end = strlen("if (@condition");

        const auto indent_length = indent * 2;

        auto node{ast::make<ast::if_else_statement>(
            ast::make<ast::expression_placeholder>("value"),
            std::vector<ast::node>{}, std::vector<ast::node>{})};
        node->source_code_range = {{line, indent_length},
                                   {line + 2, indent_length + 1}};
        node->as<ast::if_else_statement>().condition()->source_code_range = {
            {line, indent_length + condition_start},
            {line, indent_length + condition_end}};

        std::string source;
        source.reserve(indent_length + source_template_length1 + 1 +
                       indent_length + source_template_length2 + 1 +
                       indent_length + source_template_length3 + 1);
        for (size_t i = 0; i < indent; i++) {
          source.append("  ");
        }
        source.append(source_template1);
        source.append("\n");
        for (size_t i = 0; i < indent; i++) {
          source.append("  ");
        }
        source.append(source_template2);
        source.append("\n");
        for (size_t i = 0; i < indent; i++) {
          source.append("  ");
        }
        source.append(source_template3);
        source.append("\n");

        std::vector<highlight_token> highlights{
            highlight_token{highlight_token_type::keyword, indent_length, 2},
            highlight_token{highlight_token_type::placeholder,
                            indent_length + condition_start,
                            condition_end - condition_start},
            highlight_token{
                highlight_token_type::keyword,
                indent_length + source_template_length1 + 1 + indent_length + 2,
                4}};
        return std::pair{
            std::move(node),
            source_insertion{
                {line, 1}, std::move(source), std::move(highlights)}};
      }}}};

}  // namespace marlin::control
