#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

#include <span>
#include <variant>
#include <vector>

namespace wccff {

struct parser_error
{
    std::string message;
};

struct identifier
{
    std::string name;
};

struct int_constant
{
    int32_t value;
};
using expression = std::variant<int_constant>;

struct return_node
{
    expression e;
};

using statement = std::variant<return_node>;

struct function
{
    identifier function_name;
    statement body;
};

struct program
{
    function f;
};

std::expected<int_constant, parser_error> parse_constant(wccff::tokens &tokens);
std::expected<identifier, parser_error> parse_identifier(wccff::tokens &tokens);
std::expected<expression, parser_error> parse_expression(wccff::tokens &tokens);
std::expected<statement, parser_error> parse_statement(wccff::tokens &tokens);

std::expected<program, parser_error> parse(wccff::tokens &tokens);

} // namespace wccff

#endif // PARSER_H
