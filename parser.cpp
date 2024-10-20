/*
 * Will Compile C for Food, a toy C compiler
 * Copyright (C) 2024  João Pires
 * https://github.com/jpires/will-compile-c-for-food
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

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

std::optional<parser_error> consume_tokens(tokens &tokens, const std::vector<lexer::token_type> &list)
{
    if (list.size() > tokens.remaining_tokens())
    {
        return generate_unexpected_end_of_tokens(tokens);
    }

    for (const auto &t : list)
    {
        auto next_token = tokens.get_next_token_safe();
        if (next_token.type != t)
        {
            auto msg = fmt::format("Parse failure at: {}. Expected '{}' found {}", next_token.loc, t, next_token.text);
            return parser_error{ msg };
        }
    }

    return std::nullopt;
}

std::unique_ptr<assignment_node> copy_assignment_node(const std::unique_ptr<assignment_node> &node)
{
    return std::make_unique<assignment_node>(node->op, copy_expression(node->lhs), copy_expression(node->rhs));
}
std::unique_ptr<binary_node> copy_binary_node(const std::unique_ptr<binary_node> &node)
{
    return std::make_unique<binary_node>(node->op, copy_expression(node->left), copy_expression(node->right));
}
std::unique_ptr<conditional_node> copy_conditional_node(const std::unique_ptr<conditional_node> &node)
{
    return std::make_unique<conditional_node>(copy_expression(node->condition),
                                              copy_expression(node->e1),
                                              copy_expression(node->e2));
}
declaration copy_declaration(const declaration &node)
{
    std::optional<expression> init;
    if (node.init.has_value())
    {
        init = copy_expression(node.init.value());
    }
    return declaration{ node.name, std::move(init) };
}
expression copy_expression(const expression &exp)
{
    return std::visit(
      visitor{
        [](const int_constant &n) -> expression { return n; },
        [](const var &n) -> expression { return n; },
        [](const std::unique_ptr<unary_node> &n) -> expression { return copy_unary_node(n); },
        [](const std::unique_ptr<binary_node> &n) -> expression { return copy_binary_node(n); },
        [](const std::unique_ptr<assignment_node> &n) -> expression { return copy_assignment_node(n); },
        [](const std::unique_ptr<conditional_node> &n) -> expression { return copy_conditional_node(n); },
      },
      exp);
}
std::unique_ptr<unary_node> copy_unary_node(const std::unique_ptr<unary_node> &node)
{
    return std::make_unique<unary_node>(node->op, copy_expression(node->exp));
}

std::optional<parser_error> parse_semicolon(tokens &tokens)
{
    auto next_token = tokens.get_next_token();
    if (next_token.has_value() == false)
    {
        return generate_unexpected_end_of_tokens(tokens);
    }
    if (next_token->type != lexer::token_type::semicolon)
    {
        auto msg = fmt::format("Parse failure at: {}. Expected ';' found {}", next_token->loc, next_token->type);
        return parser_error{ msg };
    }

    return std::nullopt;
}

std::expected<block_item, parser_error> parse_block_item(tokens &tokens)
{
    auto next_token = tokens.peek();
    if (next_token.type == lexer::token_type::int_keyword)
    {
        return parse_declaration(tokens);
    }
    return parse_statement(tokens);
}
std::expected<block, parser_error> parse_block(tokens &tokens)
{
    // Discard the '{'
    tokens.discard_token();

    block b;
    while (tokens.peek().type != lexer::token_type::close_brace)
    {
        auto item = parse_block_item(tokens);
        if (item.has_value() == false)
        {
            return std::unexpected{ item.error() };
        }
        b.items.push_back(std::move(item.value()));
    }

    auto token = tokens.get_next_token();
    if (token.has_value() == false)
    {
        return std::unexpected{ generate_unexpected_end_of_tokens(tokens) };
    }
    if (token->type != lexer::token_type::close_brace)
    {
        auto msg = fmt::format("Parse failure at: {}. Expected '}}' found {}", token->loc, token->type);
        return std::unexpected{ parser_error{ msg } };
    }

    return b;
}
std::expected<std::unique_ptr<compound_statement>, parser_error> parse_compound_statement(tokens &tokens)
{
    auto block = parse_block(tokens);
    if (block.has_value() == false)
    {
        return std::unexpected{ block.error() };
    }
    return std::make_unique<compound_statement>(std::move(block.value()));
}
std::expected<expression, parser_error> parse_conditional(tokens &tokens)
{
    tokens.discard_token();
    auto exp = parse_expression(tokens, 0);
    if (exp.has_value() == false)
    {
        return std::unexpected{ exp.error() };
    }
    auto next_token = tokens.get_next_token();
    if (next_token.has_value() == false)
    {
        return std::unexpected{ generate_unexpected_end_of_tokens(tokens) };
    }

    if (next_token.value().type != lexer::token_type::colon)
    {
        auto msg = fmt::format("Parse failure at: {}. Expected ':' found {}", next_token->loc, next_token->type);
        return std::unexpected{ parser_error{ msg } };
    }

    return exp;
}

std::expected<std::unique_ptr<do_while_statement>, parser_error> parse_do_while(tokens &tokens)
{
    using enum lexer::token_type;
    if (auto p = consume_tokens(tokens, { do_keyword }); p.has_value())
    {
        return std::unexpected{ p.value() };
    }

    auto body = parse_statement(tokens);
    if (body.has_value() == false)
    {
        return std::unexpected{ body.error() };
    }

    if (auto p = consume_tokens(tokens, { while_keyword, open_parenthesis }); p.has_value())
    {
        return std::unexpected{ p.value() };
    }

    auto conditional = parse_expression(tokens);
    if (conditional.has_value() == false)
    {
        return std::unexpected{ conditional.error() };
    }

    if (auto p = consume_tokens(tokens, { close_parenthesis, semicolon }); p.has_value())
    {
        return std::unexpected{ p.value() };
    }

    return std::make_unique<do_while_statement>(std::move(body.value()), std::move(conditional.value()));
}

std::expected<declaration, parser_error> parse_declaration(tokens &tokens)
{
    // Discard the int keyword.
    tokens.discard_token();

    auto id = parse_identifier(tokens);
    if (id.has_value() == false)
    {
        return std::unexpected{ id.error() };
    }

    auto next_token = tokens.get_next_token();
    if (next_token.has_value() == false)
    {
        return std::unexpected{ generate_unexpected_end_of_tokens(tokens) };
    }

    if (next_token->type == lexer::token_type::semicolon)
    {
        return declaration{ id.value(), std::nullopt };
    }

    if (next_token->type == lexer::token_type::assignment_operator)
    {
        auto init = parse_expression(tokens);
        if (init.has_value() == false)
        {
            return std::unexpected{ init.error() };
        }

        auto s = parse_semicolon(tokens);
        if (s.has_value())
        {
            return std::unexpected{ s.value() };
        }

        return declaration{ id.value(), std::optional{ std::move(init.value()) } };
    }

    auto msg = fmt::format("Parse failure at: {}. Expected '=' or ';' but found {}", next_token->loc, next_token->type);
    return std::unexpected{ parser_error{ msg } };
}

std::expected<std::optional<expression>, parser_error> parse_optional_expression(tokens &tokens,
                                                                                 lexer::token_type end_token)
{
    if (tokens.peek().type == end_token)
    {
        tokens.discard_token();
        return std::nullopt;
    }

    auto expr = parse_expression(tokens);
    if (expr.has_value() == false)
    {
        return std::unexpected{ expr.error() };
    }

    if (auto p = consume_tokens(tokens, { end_token }); p.has_value())
    {
        return std::unexpected{ p.value() };
    }

    return std::move(expr.value());
}
std::expected<for_init, parser_error> parse_for_init(tokens &tokens)
{
    using enum lexer::token_type;
    if (tokens.peek().type == int_keyword)
    {
        auto decl = parse_declaration(tokens);
        if (decl.has_value() == false)
        {
            return std::unexpected{ decl.error() };
        }
        return init_declaration{ std::move(decl.value()) };
    }
    auto expr = parse_optional_expression(tokens, semicolon);
    if (expr.has_value() == false)
    {
        return std::unexpected{ expr.error() };
    }

    return init_expression{ std::move(expr.value()) };
}
std::expected<std::unique_ptr<for_statement>, parser_error> parse_for_statement(tokens &tokens)
{
    using enum lexer::token_type;

    if (auto p = consume_tokens(tokens, { for_keyword, open_parenthesis }); p.has_value())
    {
        return std::unexpected{ p.value() };
    }

    auto init = parse_for_init(tokens);
    if (init.has_value() == false)
    {
        return std::unexpected{ init.error() };
    }

    auto conditional = parse_optional_expression(tokens, semicolon);
    if (conditional.has_value() == false)
    {
        return std::unexpected{ conditional.error() };
    }

    auto post = parse_optional_expression(tokens, close_parenthesis);
    if (post.has_value() == false)
    {
        return std::unexpected{ post.error() };
    }

    auto stmt = parse_statement(tokens);
    if (stmt.has_value() == false)
    {
        return std::unexpected{ stmt.error() };
    }

    return std::make_unique<for_statement>(std::move(init.value()),
                                           std::move(conditional.value()),
                                           std::move(post.value()),
                                           std::move(stmt.value()));
}

std::expected<std::unique_ptr<if_node>, parser_error> parse_if_node(tokens &tokens)
{
    // Discard the if keyword
    tokens.discard_token();

    auto open_parens = tokens.get_next_token();
    if (open_parens.has_value() == false)
    {
        return std::unexpected{ generate_unexpected_end_of_tokens(tokens) };
    }

    auto condition = parse_expression(tokens);

    auto close_parens = tokens.get_next_token();
    if (close_parens.has_value() == false)
    {
        return std::unexpected{ generate_unexpected_end_of_tokens(tokens) };
    }

    auto then_stmt = parse_statement(tokens);
    if (then_stmt.has_value() == false)
    {
        return std::unexpected{ then_stmt.error() };
    }

    std::optional<statement> else_stmt;
    // Since the else is optional, don't consume the token yet.
    auto else_token = tokens.peek();
    if (else_token.type == lexer::token_type::else_keyword)
    {
        tokens.discard_token();
        auto tmp = parse_statement(tokens);
        if (tmp.has_value() == false)
        {
            return std::unexpected{ tmp.error() };
        }
        else_stmt = std::move(tmp.value());
    }

    return std::make_unique<if_node>(std::move(condition.value()), std::move(then_stmt.value()), std::move(else_stmt));
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

    if (tokens.peek().type != lexer::token_type::open_brace)
    {
        auto msg = fmt::format("Parse failure at: {}. Expected '{{' found {}", t1->loc, t1->type);
        return std::unexpected{ parser_error{ msg } };
    }

    auto items = parse_block(tokens);
    if (items.has_value() == false)
    {
        return std::unexpected{ items.error() };
    }

    return function{ function_name.value(), std::move(items.value()) };
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

    auto s = parse_semicolon(tokens);
    if (s.has_value())
    {
        return std::unexpected{ s.value() };
    }

    return return_node{ std::move(e.value()) };
}

std::expected<statement, parser_error> parse_statement(tokens &tokens)
{
    using enum lexer::token_type;
    auto next_token = tokens.peek();
    if (next_token.type == lexer::token_type::return_keyword)
    {
        return parse_return_node(tokens);
    }
    if (next_token.type == lexer::token_type::semicolon)
    {
        tokens.discard_token();
        return std::monostate{};
    }
    if (next_token.type == lexer::token_type::if_keyword)
    {
        return parse_if_node(tokens);
    }
    if (next_token.type == lexer::token_type::open_brace)
    {
        return parse_compound_statement(tokens);
    }
    if (next_token.type == lexer::token_type::break_keyword)
    {
        if (auto p = consume_tokens(tokens, { break_keyword, semicolon }); p.has_value())
        {
            return std::unexpected{ p.value() };
        }
        return break_statement{};
    }
    if (next_token.type == lexer::token_type::continue_keyword)
    {
        if (auto p = consume_tokens(tokens, { continue_keyword, semicolon }); p.has_value())
        {
            return std::unexpected{ p.value() };
        }
        return continue_statement{};
    }
    if (next_token.type == lexer::token_type::while_keyword)
    {
        return parse_while_statement(tokens);
    }
    if (next_token.type == lexer::token_type::do_keyword)
    {
        return parse_do_while(tokens);
    }
    if (next_token.type == lexer::token_type::for_keyword)
    {
        return parse_for_statement(tokens);
    }
    if (next_token.type == identifier && tokens.peek_after_next().type == colon)
    {
        auto label = parse_identifier(tokens);
        if (label.has_value() == false)
        {
            return std::unexpected{ label.error() };
        }
        consume_tokens(tokens, { colon });
        auto next_stmt = parse_statement(tokens);
        if (next_stmt.has_value() == false)
        {
            return std::unexpected{ next_stmt.error() };
        }

        return std::make_unique<labelled_statement>(label.value(), std::move(next_stmt.value()));
    }
    if (next_token.type == goto_keyword)
    {
        consume_tokens(tokens, { goto_keyword });
        auto label = parse_identifier(tokens);
        if (label.has_value() == false)
        {
            return std::unexpected{ label.error() };
        }
        consume_tokens(tokens, { semicolon });
        return goto_statement{ label.value() };
    }

    auto e = parse_expression(tokens);
    if (e.has_value() == false)
    {
        return std::unexpected{ e.error() };
    }

    if (auto p = consume_tokens(tokens, { semicolon }); p.has_value())
    {
        return std::unexpected{ p.value() };
    }
    return statement{ std::move(e.value()) };
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
        case lexer::token_type::and_operator:
            return logical_and_operator{};
        case lexer::token_type::or_operator:
            return logical_or_operator{};
        case lexer::token_type::equals_operator:
            return equals_operator{};
        case lexer::token_type::not_equals_operator:
            return not_equals_operator{};
        case lexer::token_type::less_than_operator:
            return less_than_operator{};
        case lexer::token_type::less_than_or_equal_operator:
            return less_than_or_equal_operator{};
        case lexer::token_type::greater_than_operator:
            return greater_than_operator{};
        case lexer::token_type::greater_than_or_equal_operator:
            return greater_than_or_equal_operator{};
        case lexer::token_type::assignment_operator:
            return assignment_operator{};
        case lexer::token_type::compound_plus:
            return compound_plus_operator{};
        case lexer::token_type::compound_minus:
            return compound_subtract_operator{};
        case lexer::token_type::compound_multiplication:
            return compound_multiply_operator{};
        case lexer::token_type::compound_division:
            return compound_divide_operator{};
        case lexer::token_type::compound_remainder:
            return compound_remainder_operator{};
        case lexer::token_type::compound_bitwise_and:
            return compound_bitwise_and_operator{};
        case lexer::token_type::compound_bitwise_or:
            return compound_bitwise_or_operator{};
        case lexer::token_type::compound_bitwise_xor:
            return compound_bitwise_xor_operator{};
        case lexer::token_type::compound_left_shift:
            return compound_left_shift_operator{};
        case lexer::token_type::compound_right_shift:
            return compound_right_shift_operator{};

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
        case lexer::token_type::not_operator:
            op = logical_not_operator{};
            break;
        case lexer::token_type::decrement_operator:
            op = prefix_decrement_operator{};
            break;
        case lexer::token_type::increment_operator:
            op = prefix_increment_operator{};
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
        case lexer::token_type::identifier:
        {
            auto i = parse_identifier(tokens);
            if (i.has_value() == false)
            {
                return std::unexpected{ i.error() };
            }

            return var{ i.value() };
        }
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
        case lexer::token_type::decrement_operator:
        case lexer::token_type::increment_operator:
        case lexer::token_type::negation_operator:
        case lexer::token_type::not_operator:
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

    auto is_right_associative = [](lexer::token_type type) {
        using enum lexer::token_type;
        return type == assignment_operator || type == compound_plus || type == compound_minus ||
               type == compound_multiplication || type == compound_division || type == compound_remainder ||
               type == compound_bitwise_and || type == compound_bitwise_or || type == compound_bitwise_xor ||
               type == compound_left_shift || type == compound_right_shift;
    };

    auto is_binary_operator = [](lexer::token_type type) {
        using enum lexer::token_type;
        return type == plus_operator || type == negation_operator || type == multiplication_operator ||
               type == division_operator || type == remainder_operator || type == bitwise_and_operator ||
               type == bitwise_or_operator || type == bitwise_xor_operator || type == left_shift_operator ||
               type == right_shift_operator || type == and_operator || type == or_operator || type == equals_operator ||
               type == not_equals_operator || type == less_than_operator || type == less_than_or_equal_operator ||
               type == greater_than_operator || type == greater_than_or_equal_operator || type == assignment_operator ||
               type == compound_plus || type == compound_minus || type == compound_multiplication ||
               type == compound_division || type == compound_remainder || type == compound_bitwise_and ||
               type == compound_bitwise_or || type == compound_bitwise_xor || type == compound_left_shift ||
               type == compound_right_shift || type == question_mark;
    };

    auto get_precedende = [](lexer::token_type type) {
        using enum lexer::token_type;
        switch (type)
        {
            case assignment_operator:
            case compound_plus:
            case compound_minus:
            case compound_multiplication:
            case compound_division:
            case compound_remainder:
            case compound_bitwise_and:
            case compound_bitwise_or:
            case compound_bitwise_xor:
            case compound_left_shift:
            case compound_right_shift:
                return 1;
            case question_mark:
                return 3;
            case or_operator:
                return 5;
            case and_operator:
                return 10;
            case bitwise_or_operator:
                return 15;
            case bitwise_xor_operator:
                return 20;
            case bitwise_and_operator:
                return 25;
            case equals_operator:
            case not_equals_operator:
                return 30;
            case less_than_operator:
            case less_than_or_equal_operator:
            case greater_than_operator:
            case greater_than_or_equal_operator:
                return 35;
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
    while (is_binary_operator(next_token.type) && min_precedence <= get_precedende(next_token.type))
    {
        if (is_right_associative(next_token.type))
        {
            auto op = parse_binary_operator(tokens);
            if (op.has_value() == false)
            {
                return std::unexpected{ op.error() };
            }
            auto right = parse_expression(tokens, get_precedende(next_token.type));
            if (right.has_value() == false)
            {
                return std::unexpected{ right.error() };
            }

            left = std::make_unique<assignment_node>(op.value(), std::move(left.value()), std::move(right.value()));
        }
        else if (next_token.type == lexer::token_type::question_mark)
        {
            auto middle = parse_conditional(tokens);
            if (middle.has_value() == false)
            {
                return std::unexpected{ middle.error() };
            }
            auto right = parse_expression(tokens, get_precedende(next_token.type));
            if (right.has_value() == false)
            {
                return std::unexpected{ right.error() };
            }
            left = std::make_unique<conditional_node>(std::move(left.value()),
                                                      std::move(middle.value()),
                                                      std::move(right.value()));
        }
        else
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
        }

        next_token = tokens.peek();
    }

    if (next_token.type == lexer::token_type::decrement_operator ||
        next_token.type == lexer::token_type::increment_operator)
    {
        auto op = next_token.type == lexer::token_type::increment_operator
                    ? unary_operator{ postfix_increment_operator{} }
                    : unary_operator{ postfix_decrement_operator{} };

        tokens.discard_token();
        return std::make_unique<unary_node>(op, std::move(left.value()));
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
std::expected<std::unique_ptr<while_statement>, parser_error> parse_while_statement(tokens &tokens)
{
    using enum lexer::token_type;
    if (auto p = consume_tokens(tokens, { while_keyword, open_parenthesis }); p.has_value())
    {
        return std::unexpected{ p.value() };
    }

    auto condition = parse_expression(tokens);
    if (condition.has_value() == false)
    {
        return std::unexpected{ condition.error() };
    }

    if (auto p = consume_tokens(tokens, { close_parenthesis }); p.has_value())
    {
        return std::unexpected{ p.value() };
    }

    auto stmt = parse_statement(tokens);
    if (stmt.has_value() == false)
    {
        return std::unexpected{ stmt.error() };
    }

    return std::make_unique<while_statement>(std::move(condition.value()), std::move(stmt.value()));
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

std::string pretty_print(const binary_operator &node, int32_t ident)
{
    return std::visit(
      wccff::visitor{
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
        [ident](const logical_and_operator &) { return wccff::format_indented(ident, "Logic And"); },
        [ident](const logical_or_operator &) { return wccff::format_indented(ident, "Logic Or"); },
        [ident](const equals_operator &) { return wccff::format_indented(ident, "Equals"); },
        [ident](const not_equals_operator &) { return wccff::format_indented(ident, "Not Equals"); },
        [ident](const less_than_operator &) { return wccff::format_indented(ident, "Less Than"); },
        [ident](const less_than_or_equal_operator &) { return wccff::format_indented(ident, "Less Than or Equals"); },
        [ident](const greater_than_operator &) { return wccff::format_indented(ident, "Greater Than"); },
        [ident](const greater_than_or_equal_operator &) {
            return wccff::format_indented(ident, "Greater Than or Equals");
        },
        [ident](const assignment_operator &) { return wccff::format_indented(ident, "Assignment"); },
        [ident](const parser::compound_plus_operator &) -> std::string {
            return wccff::format_indented(ident, "Compound Plus");
        },
        [ident](const parser::compound_subtract_operator &) -> std::string {
            return wccff::format_indented(ident, "Compound Minus");
        },
        [ident](const parser::compound_multiply_operator &) -> std::string {
            return wccff::format_indented(ident, "Compound Multiplication");
        },
        [ident](const parser::compound_divide_operator &) -> std::string {
            return wccff::format_indented(ident, "Compound Division");
        },
        [ident](const parser::compound_remainder_operator &) -> std::string {
            return wccff::format_indented(ident, "Compound Remainder");
        },
        [ident](const parser::compound_bitwise_and_operator &) -> std::string {
            return wccff::format_indented(ident, "Compound Bitwise And");
        },
        [ident](const parser::compound_bitwise_or_operator &) -> std::string {
            return wccff::format_indented(ident, "Compound Bitwise Or");
        },
        [ident](const parser::compound_bitwise_xor_operator &) -> std::string {
            return wccff::format_indented(ident, "Compound Bitwise Xor");
        },
        [ident](const parser::compound_left_shift_operator &) -> std::string {
            return wccff::format_indented(ident, "Compound Left Shift");
        },
        [ident](const parser::compound_right_shift_operator &) -> std::string {
            return wccff::format_indented(ident, "Compound Right Shift");
        },
      },
      node);
}

std::string pretty_print(const block &node, int32_t ident)
{
    std::string output;
    for (const auto &item : node.items)
    {
        output += pretty_print(item, ident);
        output += "\n";
    }
    if (node.items.empty() == false)
    {
        output.erase(output.size() - 1);
    }

    return output;
}
std::string pretty_print(const block_item &node, int32_t ident)
{
    return std::visit(visitor{
                        [ident](const declaration &n) { return pretty_print(n, ident); },
                        [ident](const statement &n) { return pretty_print(n, ident); },
                        [ident](const std::monostate &) { return wccff::format_indented(ident, "EMPTY Block item"); },
                      },
                      node);
}
std::string pretty_print(const break_statement &node, int32_t ident)
{
    return wccff::format_indented(ident, "Break({})", pretty_print(node.label));
}
std::string pretty_print(const continue_statement &node, int32_t ident)
{
    return wccff::format_indented(ident, "Continue({})", pretty_print(node.label));
}
std::string pretty_print(const declaration &node, int32_t ident)
{
    auto name = wccff::format_indented(0, "{}", pretty_print(node.name));
    std::string init;
    if (node.init.has_value())
    {
        init = pretty_print(node.init.value(), ident + 8);
    }
    else
    {
        init = format_indented(ident + 8, "NO_INIT");
    }
    auto sufix = wccff::format_indented(ident, ")");

    return format_indented(ident, "DeclVar({}\n{}\n{}", name, init, sufix);
}

std::string pretty_print(const expression &node, int32_t ident)
{
    return std::visit(wccff::visitor{
                        [ident](const var &n) { return pretty_print(n, ident); },
                        [ident](const int_constant &n) { return pretty_print(n, ident); },
                        [ident](const std::unique_ptr<unary_node> &n) { return pretty_print(n, ident); },
                        [ident](const std::unique_ptr<binary_node> &n) { return pretty_print(n, ident); },
                        [ident](const std::unique_ptr<assignment_node> &n) { return pretty_print(n, ident); },
                        [ident](const std::unique_ptr<conditional_node> &n) { return pretty_print(n, ident); },
                      },
                      node);
}
std::string pretty_print(const for_init &node, int32_t ident)
{
    return std::visit(wccff::visitor{
                        [ident](const init_declaration &n) { return pretty_print(n.decl, ident); },
                        [ident](const init_expression &n) { return pretty_print(n.expression, ident); },
                      },
                      node);
}

std::string pretty_print(const function &node, int32_t ident)
{
    auto prefix = wccff::format_indented(ident, "Function({}", pretty_print(node.function_name));
    auto left = wccff::format_indented(0, "{}", pretty_print(node.body, ident + 9));
    auto sufix = wccff::format_indented(ident, ")");

    return fmt::format("{}\n{}\n{}", prefix, left, sufix);
}
std::string pretty_print(const goto_statement &node, int32_t ident)
{
    return wccff::format_indented(ident, "GoTo({})", pretty_print(node.label));
}
std::string pretty_print(const identifier &node, int32_t ident)
{
    return wccff::format_indented(ident, "{}", node.name);
}
std::string pretty_print(const int_constant &node, int32_t ident)
{
    return wccff::format_indented(ident, "IntConstant({})", node.value);
}
std::string pretty_print(const program &node, int32_t ident)
{
    return pretty_print(node.f, ident);
}

std::string pretty_print(const statement &node, int32_t ident)
{
    return std::visit(
      visitor{
        [ident](const return_node &n) { return pretty_print(n, ident); },
        [ident](const expression &n) { return pretty_print(n, ident); },
        [ident](const std::unique_ptr<if_node> &n) { return pretty_print(n, ident); },
        [ident](const std::unique_ptr<compound_statement> &n) { return pretty_print(n, ident); },
        [ident](const break_statement &n) { return pretty_print(n, ident); },
        [ident](const continue_statement &n) { return pretty_print(n, ident); },
        [ident](const goto_statement &n) { return pretty_print(n, ident); },
        [ident](const std::unique_ptr<while_statement> &n) { return pretty_print(n, ident); },
        [ident](const std::unique_ptr<do_while_statement> &n) { return pretty_print(n, ident); },
        [ident](const std::unique_ptr<for_statement> &n) { return pretty_print(n, ident); },
        [ident](const std::monostate &) { return wccff::format_indented(ident, "EMPTY STATEMENT\n"); },
        [ident](const std::unique_ptr<parser::labelled_statement> &n) { return pretty_print(n, ident); },
      },
      node);
}

std::string pretty_print(const return_node &node, int32_t ident)
{
    auto prefix = wccff::format_indented(ident, "Return(");
    auto a = pretty_print(node.e, ident + 7);
    auto sufix = wccff::format_indented(ident, ")");
    return fmt::format("{}\n{}\n{}", prefix, a, sufix);
}

std::string pretty_print(const std::optional<expression> &node, int32_t ident)
{
    if (node.has_value())
    {
        return pretty_print(node.value(), ident);
    }
    return wccff::format_indented(ident, "EMPTY EXPRESSION");
}
std::string pretty_print(const std::unique_ptr<assignment_node> &node, int32_t ident)
{
    auto prefix = wccff::format_indented(ident, "Assign({}", pretty_print(node->op, 0));
    auto left = wccff::format_indented(0, "{}", pretty_print(node->lhs, ident + 7));
    auto right = wccff::format_indented(0, "{}", pretty_print(node->rhs, ident + 7));
    auto sufix = wccff::format_indented(ident, ")");

    return fmt::format("{}\n{}\n{}\n{}", prefix, left, right, sufix);
}

std::string pretty_print(const std::unique_ptr<binary_node> &node, int32_t ident)
{
    auto prefix = wccff::format_indented(ident, "Binary({}", pretty_print(node->op, 0));
    auto left = wccff::format_indented(0, "{}", pretty_print(node->left, ident + 7));
    auto right = wccff::format_indented(0, "{}", pretty_print(node->right, ident + 7));
    auto sufix = wccff::format_indented(ident, ")");

    return fmt::format("{}\n{}\n{}\n{}", prefix, left, right, sufix);
}

std::string pretty_print(const std::unique_ptr<compound_statement> &node, int32_t ident)
{
    auto prefix = format_indented(ident, "Compound(");
    auto middle = format_indented(0, "{}", pretty_print(node->block, ident + 9));
    auto sufix = format_indented(ident, ")");

    return fmt::format("{}\n{}\n{}", prefix, middle, sufix);
}
std::string pretty_print(const std::unique_ptr<conditional_node> &node, int32_t ident)
{
    auto prefix = wccff::format_indented(ident, "Conditional({}", pretty_print(node->condition, 0));
    auto middle = wccff::format_indented(0, "{}", pretty_print(node->e1, ident + 7));
    auto right = wccff::format_indented(0, "{}", pretty_print(node->e2, ident + 7));
    auto sufix = wccff::format_indented(ident, ")");

    return fmt::format("{}\n{}\n{}\n{}", prefix, middle, right, sufix);
}
std::string pretty_print(const std::unique_ptr<do_while_statement> &node, int32_t ident)
{
    auto prefix = wccff::format_indented(ident, "Do While({}", pretty_print(node->label, 0));
    auto body = wccff::format_indented(0, "{}", pretty_print(node->body, ident + 9));
    auto condition = wccff::format_indented(0, "{}", pretty_print(node->condition, ident + 9));

    auto sufix = wccff::format_indented(ident, ")");

    return fmt::format("{}\n{}\n{}\n{}", prefix, body, condition, sufix);
}
std::string pretty_print(const std::unique_ptr<for_statement> &node, int32_t ident)
{
    auto prefix = wccff::format_indented(ident, "For Loop({}", pretty_print(node->label, 0));
    auto init = wccff::format_indented(0, "{}", pretty_print(node->init, ident + 9));
    auto condition = wccff::format_indented(0, "{}", pretty_print(node->condition, ident + 9));
    auto post = wccff::format_indented(0, "{}", pretty_print(node->post, ident + 9));
    auto body = wccff::format_indented(0, "{}", pretty_print(node->body, ident + 9));
    auto sufix = wccff::format_indented(ident, ")");

    return fmt::format("{}\n{}\n{}\n{}\n{}\n{}", prefix, init, condition, post, body, sufix);
}

std::string pretty_print(const std::unique_ptr<if_node> &node, int32_t ident)
{
    auto prefix = wccff::format_indented(ident, "If({}", pretty_print(node->op, 0));
    auto then_stmt = wccff::format_indented(0, "{}", pretty_print(node->then_stmt, ident + 7));

    std::string else_stmt;
    if (node->else_stmt.has_value())
    {
        else_stmt = wccff::format_indented(ident, "Else\n");
        else_stmt += wccff::format_indented(0, "{}", pretty_print(node->else_stmt.value(), ident + 7));
    }
    auto sufix = wccff::format_indented(ident, ")");

    return fmt::format("{}\n{}\n{}\n{}", prefix, then_stmt, else_stmt, sufix);
}
std::string pretty_print(const std::unique_ptr<labelled_statement> &node, int32_t ident)
{
    auto prefix = wccff::format_indented(ident, "LabeledStatement({}", pretty_print(node->label, 0));
    auto stmt = pretty_print(node->body, ident + 17);
    auto sufix = wccff::format_indented(ident, ")");

    return fmt::format("{}\n{}\n{}", prefix, stmt, sufix);
}

std::string pretty_print(const std::unique_ptr<unary_node> &node, int32_t ident)
{
    auto prefix = wccff::format_indented(ident, "Unary({}", pretty_print(node->op, 0));
    auto inner = wccff::format_indented(0, "{}", pretty_print(node->exp, ident + 6));
    auto sufix = wccff::format_indented(ident, ")");

    return fmt::format("{}\n{}\n{}", prefix, inner, sufix);
}

std::string pretty_print(const std::unique_ptr<while_statement> &node, int32_t ident)
{
    auto prefix = wccff::format_indented(ident, "While({}", pretty_print(node->label, 0));
    auto condition = wccff::format_indented(0, "{}", pretty_print(node->condition, ident + 6));
    auto body = wccff::format_indented(0, "{}", pretty_print(node->body, ident + 6));
    auto sufix = wccff::format_indented(ident, ")");

    return fmt::format("{}\n{}\n{}\n{}", prefix, condition, body, sufix);
}

std::string pretty_print(const unary_operator &node, int32_t ident)
{
    return std::visit(
      wccff::visitor{
        [ident](const bitwise_complement_operator &) { return wccff::format_indented(ident, "Complement"); },
        [ident](const negate_operator &) { return wccff::format_indented(ident, "Negate"); },
        [ident](const logical_not_operator &) { return wccff::format_indented(ident, "Not"); },
        [ident](const postfix_decrement_operator &) { return wccff::format_indented(ident, "Postfix Decrement"); },
        [ident](const postfix_increment_operator &) { return wccff::format_indented(ident, "Postfix Increment"); },
        [ident](const prefix_decrement_operator &) { return wccff::format_indented(ident, "Prefix Decrement"); },
        [ident](const prefix_increment_operator &) { return wccff::format_indented(ident, "Prefix Increment"); },
      },
      node);
}

std::string pretty_print(const var &node, int32_t ident)
{
    return wccff::format_indented(ident, "Var({})", pretty_print(node.name));
}
} // namespace wccff::parser
