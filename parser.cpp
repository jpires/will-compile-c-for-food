#include "parser.h"
#include "utils.h"
#include "visitor.h"
#include <charconv>
#include <fmt/core.h>

namespace wccff::parser {

static parser_error generate_unexpected_end_of_tokens(const tokens &tokens)
{
    auto previous = tokens.previous_token();
    auto msg = fmt::format("{}: Error: Unexpected end of tokens after '{}'", previous.loc, previous.text);
    return { msg };
}

int32_t int32_t_from_string(std::string_view str)
{
    int32_t value = 0;
    std::from_chars(str.data(), str.data() + str.size(), value, 10);
    // ToDo: Handle errors
    return value;
}

std::expected<function, parser_error> parse_function(tokens &tokens)
{
    auto t1 = tokens.get_next_token();
    if (t1.has_value() == false)
    {
        return std::unexpected{ generate_unexpected_end_of_tokens(tokens) };
    }
    if (t1->type != lexer::token_type::int_keyword)
    {
        auto msg = fmt::format("Parse failure at: {}. Expected int keyword found {}", t1->loc, t1->type);
        return std::unexpected{ parser_error{ msg } };
    }
    auto function_name = parse_identifier(tokens);
    if (function_name.has_value() == false)
    {
        return std::unexpected{ function_name.error() };
    }

    // At this point, we need at least 4 tokens until we get to the statement.
    if (tokens.remaining_tokens() < 4)
    {
        return std::unexpected{ generate_unexpected_end_of_tokens(tokens) };
    }

    t1 = tokens.get_next_token_safe();
    if (t1->type != lexer::token_type::open_parenthesis)
    {
        auto msg = fmt::format("Parse failure at: {}. Expected '(' found {}", t1->loc, t1->type);
        return std::unexpected{ parser_error{ msg } };
    }

    t1 = tokens.get_next_token_safe();
    if (t1->type != lexer::token_type::void_keyword)
    {
        auto msg = fmt::format("Parse failure at: {}. Expected void keyword found {}", t1->loc, t1->type);
        return std::unexpected{ parser_error{ msg } };
    }

    t1 = tokens.get_next_token_safe();
    if (t1->type != lexer::token_type::close_parenthesis)
    {
        auto msg = fmt::format("Parse failure at: {}. Expected ')' found {}", t1->loc, t1->type);
        return std::unexpected{ parser_error{ msg } };
    }

    t1 = tokens.get_next_token_safe();
    if (t1->type != lexer::token_type::open_brace)
    {
        auto msg = fmt::format("Parse failure at: {}. Expected '{{' found {}", t1->loc, t1->type);
        return std::unexpected{ parser_error{ msg } };
    }

    auto statement = parse_statement(tokens);
    if (statement.has_value() == false)
    {
        return std::unexpected{ statement.error() };
    }

    // At this point, we need two more tokens
    if (tokens.remaining_tokens() < 2)
    {
        return std::unexpected{ generate_unexpected_end_of_tokens(tokens) };
    }
    t1 = tokens.get_next_token_safe();
    if (t1->type != lexer::token_type::semicolon)
    {
        auto msg = fmt::format("Parse failure at: {}. Expected ';' found {}", t1->loc, t1->type);
        return std::unexpected{ parser_error{ msg } };
    }

    t1 = tokens.get_next_token_safe();
    if (t1->type != lexer::token_type::close_brace)
    {
        auto msg = fmt::format("Parse failure at: {}. Expected '}}' found {}", t1->loc, t1->type);
        return std::unexpected{ parser_error{ msg } };
    }

    return function{ function_name.value(), std::move(statement.value()) };
}

std::expected<program, parser_error> parse_program(tokens &tokens)
{
    program p;
    auto function = parse_function(tokens);
    if (function.has_value() == false)
    {
        return std::unexpected{ function.error() };
    }
    p.f = std::move(function.value());
    return p;
}

std::expected<return_node, parser_error> parse_return_node(tokens &tokens)
{
    auto t1 = tokens.get_next_token();
    if (t1.has_value() == false)
    {
        return std::unexpected{ generate_unexpected_end_of_tokens(tokens) };
    }

    if (t1->type != lexer::token_type::return_keyword)
    {
        auto msg = fmt::format("Parse failure at: {}. Expected return keyword found {}", t1->loc, t1->type);
        return std::unexpected{ parser_error{ msg } };
    }
    auto e = parse_expression(tokens);
    if (e.has_value() == false)
    {
        return std::unexpected{ e.error() };
    }

    return return_node{ std::move(e.value()) };
}

std::expected<statement, parser_error> parse_statement(tokens &tokens)
{
    return parse_return_node(tokens);
}

std::expected<binary_operator, parser_error> parse_binary_operator(tokens &tokens)
{
    auto t = tokens.get_next_token();
    if (t.has_value() == false)
    {
        return std::unexpected{ generate_unexpected_end_of_tokens(tokens) };
    }

    switch (t->type)
    {
        case lexer::token_type::bitwise_and_operator:
            return bitwise_and_operator{};
        case lexer::token_type::bitwise_or_operator:
            return bitwise_or_operator{};
        case lexer::token_type::bitwise_xor_operator:
            return bitwise_xor_operator{};
        case lexer::token_type::plus_operator:
            return plus_operator{};
        case lexer::token_type::negation_operator:
            return subtract_operator{};
        case lexer::token_type::multiplication_operator:
            return multiply_operator{};
        case lexer::token_type::division_operator:
            return divide_operator{};
        case lexer::token_type::remainder_operator:
            return remainder_operator{};
        case lexer::token_type::left_shift_operator:
            return left_shift_operator{};
        case lexer::token_type::right_shift_operator:
            return right_shift_operator{};
        default:
            auto msg = fmt::format("Expected Binary Operator but found '{}'", t->text);
            return std::unexpected{ parser_error{ msg } };
    }
}
std::expected<std::unique_ptr<unary_node>, parser_error> parse_unary_node(tokens &tokens)
{
    auto t = tokens.get_next_token();
    if (t.has_value() == false)
    {
        return std::unexpected{ generate_unexpected_end_of_tokens(tokens) };
    }
    unary_operator op;
    switch (t->type)
    {
        case lexer::token_type::bitwise_complement_operator:
            op = bitwise_complement_operator{};
            break;
        case lexer::token_type::negation_operator:
            op = negate_operator{};
            break;

        default:
            auto msg = fmt::format("Parse failure at: {}. Expected Unary Operator '~' or '-' but found {}",
                                   t->loc,
                                   t->type);
            return std::unexpected{ parser_error{ msg } };
    }

    auto exp = parse_factor(tokens);
    if (exp.has_value() == false)
    {
        return std::unexpected{ exp.error() };
    }

    return std::make_unique<unary_node>(op, std::move(exp.value()));
}

std::expected<expression, parser_error> parse_factor(tokens &tokens)
{
    auto next_toke = tokens.peek();
    switch (next_toke.type)
    {
        case lexer::token_type::constant:
        {
            auto e = parse_constant(tokens);
            if (e.has_value() == false)
            {
                return std::unexpected{ e.error() };
            }

            return e.value();
        }
        case lexer::token_type::bitwise_complement_operator:
        case lexer::token_type::negation_operator:
        {
            auto u = parse_unary_node(tokens);
            if (u.has_value() == false)
            {
                return std::unexpected{ u.error() };
            }
            return std::move(u.value());
        }
        case lexer::token_type::open_parenthesis:
        {
            tokens.get_next_token_safe();
            auto inner_expr = parse_expression(tokens);
            auto n_t = tokens.get_next_token();
            if (n_t.has_value() == false)
            {
                return std::unexpected{ generate_unexpected_end_of_tokens(tokens) };
            }
            if (n_t->type != lexer::token_type::close_parenthesis)
            {
                auto msg = fmt::format("Parse failure at: {}. Expected return keyword found {}", n_t->loc, n_t->type);
                return std::unexpected{ parser_error{ msg } };
            }
            return inner_expr;
        }
        default:
        {
            auto msg = fmt::format("Parse failure at: Unexpected token '{}', expected an Expression", next_toke.text);
            return std::unexpected{ parser_error{ msg } };
        }
    }
}

std::expected<expression, parser_error> parse_expression(tokens &tokens, int32_t min_precedence)
{
    auto is_binary_operator = [](lexer::token_type type) {
        using enum lexer::token_type;
        return type == plus_operator || type == negation_operator || type == multiplication_operator ||
               type == division_operator || type == remainder_operator || type == bitwise_and_operator ||
               type == bitwise_or_operator || type == bitwise_xor_operator || type == left_shift_operator ||
               type == right_shift_operator;
        ;
    };

    auto get_precedende = [](lexer::token_type type) {
        using enum lexer::token_type;
        switch (type)
        {
            case bitwise_or_operator:
                return 15;
            case bitwise_xor_operator:
                return 20;
            case bitwise_and_operator:
                return 25;
            case left_shift_operator:
            case right_shift_operator:
                return 40;
            case plus_operator:
            case negation_operator:
                return 45;
            case multiplication_operator:
            case division_operator:
            case remainder_operator:
                return 50;
        }
        return 0;
    };

    auto left = parse_factor(tokens);
    if (left.has_value() == false)
    {
        return std::unexpected{ left.error() };
    }

    auto next_token = tokens.peek();
    while (is_binary_operator(next_token.type) && min_precedence < get_precedende(next_token.type))
    {
        auto op = parse_binary_operator(tokens);
        if (op.has_value() == false)
        {
            return std::unexpected{ op.error() };
        }

        auto right = parse_expression(tokens, get_precedende(next_token.type) + 1);
        if (right.has_value() == false)
        {
            return std::unexpected{ right.error() };
        }

        left = std::make_unique<binary_node>(op.value(), std::move(left.value()), std::move(right.value()));
        next_token = tokens.peek();
    }
    return left;
}

std::expected<identifier, parser_error> parse_identifier(tokens &tokens)
{
    auto token = tokens.get_next_token();
    if (token.has_value() == false)
    {
        return std::unexpected{ generate_unexpected_end_of_tokens(tokens) };
    }

    if (token->type != lexer::token_type::identifier)
    {
        auto msg = fmt::format("Parse failure at: {}. Expected Identifier found {}", token->loc, token->type);
        return std::unexpected{ parser_error{ msg } };
    }

    identifier c;
    c.name = token->text;
    return c;
}

std::expected<int_constant, parser_error> parse_constant(tokens &tokens)
{
    auto token = tokens.get_next_token();
    if (token.has_value() == false)
    {
        return std::unexpected{ generate_unexpected_end_of_tokens(tokens) };
    }

    if (token->type != lexer::token_type::constant)
    {
        auto msg = fmt::format("Parse failure at: {}. Expected Constant found {}", token->loc, token->type);
        return std::unexpected{ parser_error{ msg } };
    }

    return int_constant{ int32_t_from_string(token->text) };
}

std::expected<program, parser_error> parse(tokens &tokens)
{
    auto p = parse_program(tokens);
    if (p.has_value() == false)
    {
        return std::unexpected{ p.error() };
    }

    if (tokens.remaining_tokens() != 0)
    {
        // There are more tokens at the end of the program.
        // Which is invalid
        return std::unexpected{ parser_error{ "Unexpected tokens at the end of the input" } };
    }
    return p;
}

std::string pretty_print(const unary_operator &node, int32_t ident)
{
    return std::visit(
      wccff::visitor{
        [ident](const bitwise_complement_operator &) { return wccff::format_indented(ident, "Complement"); },
        [ident](const negate_operator &) { return wccff::format_indented(ident, "Negate"); } },
      node);
}

std::string pretty_print(const binary_operator &node, int32_t ident)
{
    return std::visit(wccff::visitor{
                        [ident](const plus_operator &) { return wccff::format_indented(ident, "Plus"); },
                        [ident](const subtract_operator &) { return wccff::format_indented(ident, "Subtract"); },
                        [ident](const multiply_operator &) { return wccff::format_indented(ident, "Multiply"); },
                        [ident](const divide_operator &) { return wccff::format_indented(ident, "Divide"); },
                        [ident](const remainder_operator &) { return wccff::format_indented(ident, "Remainder"); },
                        [ident](const bitwise_and_operator &) { return wccff::format_indented(ident, "Bitwise And"); },
                        [ident](const bitwise_or_operator &) { return wccff::format_indented(ident, "Bitwise Or"); },
                        [ident](const bitwise_xor_operator &) { return wccff::format_indented(ident, "Bitwise Xor"); },
                        [ident](const left_shift_operator &) { return wccff::format_indented(ident, "Left Shift"); },
                        [ident](const right_shift_operator &) { return wccff::format_indented(ident, "Right Shift"); },
                      },
                      node);
}

std::string pretty_print(const std::unique_ptr<unary_node> &node, int32_t ident)
{
    auto prefix = wccff::format_indented(ident, "Unary({}", pretty_print(node->op, 0));
    auto inner = wccff::format_indented(0, "{}", pretty_print(node->exp, ident + 6));
    auto sufix = wccff::format_indented(ident, ")");

    return fmt::format("{}\n{}\n{}", prefix, inner, sufix);
}

std::string pretty_print(const std::unique_ptr<binary_node> &node, int32_t ident)
{
    auto prefix = wccff::format_indented(ident, "Binary({}", pretty_print(node->op, 0));
    auto left = wccff::format_indented(0, "{}", pretty_print(node->left, ident + 7));
    auto right = wccff::format_indented(0, "{}", pretty_print(node->right, ident + 7));
    auto sufix = wccff::format_indented(ident, ")");

    return fmt::format("{}\n{}\n{}\n{}", prefix, left, right, sufix);
}
std::string pretty_print(const expression &node, int32_t ident)
{
    return std::visit(
      wccff::visitor{ [ident](const int_constant &n) { return wccff::format_indented(ident, "Constant({})", n.value); },
                      [ident](const std::unique_ptr<unary_node> &n) { return pretty_print(n, ident); },
                      [ident](const std::unique_ptr<binary_node> &n) { return pretty_print(n, ident); } },
      node);
}
std::string pretty_print(const statement &node, int32_t ident)
{
    return std::visit(
      [ident](const return_node &n) {
          return wccff::format_indented(ident, "Return\n{}", pretty_print(n.e, ident + 4));
      },
      node);
}
std::string pretty_print(const function &node, int32_t ident)
{
    return wccff::format_indented(ident,
                                  "Function({})\n{}",
                                  node.function_name.name,
                                  pretty_print(node.body, ident + 4));
}
std::string pretty_print(const program &node, int32_t ident)
{
    return pretty_print(node.f, ident);
}
} // namespace wccff::parser
