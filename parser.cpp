#include "parser.h"
#include <charconv>
#include <fmt/core.h>

namespace wccff {

int32_t int32_t_from_string(std::string_view str)
{
    int32_t value = 0;
    std::from_chars(str.data(), str.data() + str.size(), value, 10);
    // ToDo: Handle errors
    return value;
}

std::expected<function, parser_error> parse_function(wccff::tokens &tokens)
{
    auto t1 = tokens.get_next_token();
    if (t1.has_value() == false)
    {
        auto msg = fmt::format("Parse failure at: Unexpected end of tokens");
        return std::unexpected{ parser_error{ msg } };
    }
    if (t1->t != token_type::int_keyword)
    {
        auto msg = fmt::format("Parse failure at: {}:{}. Expected int keyword found {}",
                               t1->loc.line,
                               t1->loc.column,
                               static_cast<int>(t1->t));
        return std::unexpected{ parser_error{ msg } };
    }
    auto function_name = parse_identifier(tokens);

    t1 = tokens.get_next_token();
    if (t1->t != token_type::open_parenthesis)
    {
        auto msg = fmt::format("Parse failure at: {}:{}. Expected '(' found {}",
                               t1->loc.line,
                               t1->loc.column,
                               static_cast<int>(t1->t));
        return std::unexpected{ parser_error{ msg } };
    }

    t1 = tokens.get_next_token();
    if (t1->t != token_type::void_keyword)
    {
        auto msg = fmt::format("Parse failure at: {}:{}. Expected void keyword found {}",
                               t1->loc.line,
                               t1->loc.column,
                               static_cast<int>(t1->t));
        return std::unexpected{ parser_error{ msg } };
    }

    t1 = tokens.get_next_token();
    if (t1->t != token_type::close_parenthesis)
    {
        auto msg = fmt::format("Parse failure at: {}:{}. Expected ')' found {}",
                               t1->loc.line,
                               t1->loc.column,
                               static_cast<int>(t1->t));
        return std::unexpected{ parser_error{ msg } };
    }

    t1 = tokens.get_next_token();
    if (t1->t != token_type::open_brace)
    {
        auto msg = fmt::format("Parse failure at: {}:{}. Expected '{{' found {}",
                               t1->loc.line,
                               t1->loc.column,
                               static_cast<int>(t1->t));
        return std::unexpected{ parser_error{ msg } };
    }

    auto statement = parse_statement(tokens);
    if (statement.has_value() == false)
    {
        return std::unexpected{ statement.error() };
    }

    t1 = tokens.get_next_token();
    if (t1->t != token_type::semicolon)
    {
        auto msg = fmt::format("Parse failure at: {}:{}. Expected ';' found {}",
                               t1->loc.line,
                               t1->loc.column,
                               static_cast<int>(t1->t));
        return std::unexpected{ parser_error{ msg } };
    }

    t1 = tokens.get_next_token();
    if (t1->t != token_type::close_brace)
    {
        auto msg = fmt::format("Parse failure at: {}:{}. Expected '}}' found {}",
                               t1->loc.line,
                               t1->loc.column,
                               static_cast<int>(t1->t));
        return std::unexpected{ parser_error{ msg } };
    }

    return function{ function_name.value(), statement.value() };
}

std::expected<program, parser_error> parse_program(wccff::tokens &tokens)
{
    program p;
    auto function = parse_function(tokens);
    if (function.has_value() == false)
    {
        return std::unexpected{ function.error() };
    }
    p.f = function.value();
    return p;
}

std::expected<return_node, parser_error> parse_return_node(wccff::tokens &tokens)
{
    auto t1 = tokens.get_next_token();
    if (t1.has_value() == false)
    {
        auto msg = fmt::format("Parse failure at: Unexpected end of tokens");
        return std::unexpected{ parser_error{ msg } };
    }

    if (t1->t != token_type::return_keyword)
    {
        auto msg = fmt::format("Parse failure at: {}:{}. Expected return keyword found {}",
                               t1->loc.line,
                               t1->loc.column,
                               static_cast<int>(t1->t));
        return std::unexpected{ parser_error{ msg } };
    }
    auto e = parse_expression(tokens);
    if (e.has_value() == false)
    {
        return std::unexpected{ e.error() };
    }

    return return_node{ e.value() };
}

std::expected<statement, parser_error> parse_statement(wccff::tokens &tokens)
{
    return parse_return_node(tokens);
}

std::expected<expression, parser_error> parse_expression(wccff::tokens &tokens)
{
    auto e = parse_constant(tokens);
    if (e.has_value() == false)
    {
        return std::unexpected{ e.error() };
    }

    return e.value();
}

std::expected<identifier, parser_error> parse_identifier(wccff::tokens &tokens)
{
    auto token = tokens.get_next_token();
    if (token.has_value() == false)
    {
        auto msg = fmt::format("Parse failure at: Unexpected end of tokens");
        return std::unexpected{ parser_error{ msg } };
    }

    if (token->t != token_type::identifier)
    {
        auto msg = fmt::format("Parse failure at: {}:{}. Expected Identifier found {}",
                               token->loc.line,
                               token->loc.column,
                               static_cast<int>(token->t));
        return std::unexpected{ parser_error{ msg } };
    }

    identifier c;
    c.name = token->c;
    return c;
}

std::expected<int_constant, parser_error> parse_constant(wccff::tokens &tokens)
{
    auto token = tokens.get_next_token();
    if (token.has_value() == false)
    {
        auto msg = fmt::format("Parse failure at: Unexpected end of tokens");
        return std::unexpected{ parser_error{ msg } };
    }

    if (token->t != token_type::constant)
    {
        auto msg = fmt::format("Parse failure at: {}:{}. Expected Constant found {}",
                               token->loc.line,
                               token->loc.column,
                               static_cast<int>(token->t));
        return std::unexpected{ parser_error{ msg } };
    }

    return int_constant{ int32_t_from_string(token->c) };
}

std::expected<program, parser_error> parse(wccff::tokens &tokens)
{
    auto p = parse_program(tokens);

    if (tokens.get_next_token().has_value())
    {
        // There are more tokens at the end of the program.
        // Which is invalid
        return std::unexpected{ parser_error{ "Unexpected tokens at the end of the input" } };
    }
    return p;
}

} // namespace wccff
