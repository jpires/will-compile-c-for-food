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

#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include <span>
#include <variant>
#include <vector>

namespace wccff::parser {

/**
 * Abstract a list of tokens.
 * Makes it easier for the parser to navigate said list.
 */
class tokens
{
  public:
    explicit tokens(std::vector<wccff::lexer::token> tokens_)
      : m_tokens(std::move(tokens_))
    {
    }
    void discard_token() { m_index++; }

    [[nodiscard]] std::size_t remaining_tokens() const { return m_tokens.size() - m_index; }
    wccff::lexer::token get_next_token_safe() { return m_tokens.at(m_index++); }
    std::optional<wccff::lexer::token> get_next_token()
    {
        if (m_index >= m_tokens.size())
        {
            return std::nullopt;
        }
        return m_tokens[m_index++];
    }

    [[nodiscard]] wccff::lexer::token peek() const { return m_tokens.at(m_index); }
    [[nodiscard]] wccff::lexer::token previous_token() const { return m_tokens.at(m_index - 1); }

  private:
    std::vector<wccff::lexer::token> m_tokens;
    std::size_t m_index{ 0 };
};

struct parser_error
{
    std::string message;
};

struct identifier
{
    std::string name;
};

struct bitwise_complement_operator
{
};
struct bitwise_and_operator
{
};
struct bitwise_or_operator
{
};
struct bitwise_xor_operator
{
};
struct equals_operator
{
};
struct greater_than_operator
{
};
struct greater_than_or_equal_operator
{
};
struct less_than_operator
{
};
struct less_than_or_equal_operator
{
};
struct logical_and_operator
{
};
struct logical_not_operator
{
};
struct logical_or_operator
{
};
struct negate_operator
{
};
struct not_equals_operator
{
};
struct plus_operator
{
};
struct subtract_operator
{
};
struct multiply_operator
{
};
struct divide_operator
{
};
struct remainder_operator
{
};
struct left_shift_operator
{
};
struct right_shift_operator
{
};
struct assignment_operator
{
};
struct compound_plus_operator
{
};
struct compound_subtract_operator
{
};
struct compound_multiply_operator
{
};
struct compound_divide_operator
{
};
struct compound_remainder_operator
{
};
struct compound_bitwise_and_operator
{
};
struct compound_bitwise_or_operator
{
};
struct compound_bitwise_xor_operator
{
};
struct compound_left_shift_operator
{
};
struct compound_right_shift_operator
{
};
struct prefix_decrement_operator
{
};
struct prefix_increment_operator
{
};
struct postfix_decrement_operator
{
};
struct postfix_increment_operator
{
};

using unary_operator = std::variant<bitwise_complement_operator,
                                    negate_operator,
                                    logical_not_operator,
                                    postfix_decrement_operator,
                                    postfix_increment_operator,
                                    prefix_decrement_operator,
                                    prefix_increment_operator>;

using binary_operator = std::variant<plus_operator,
                                     subtract_operator,
                                     multiply_operator,
                                     divide_operator,
                                     remainder_operator,
                                     bitwise_and_operator,
                                     bitwise_or_operator,
                                     bitwise_xor_operator,
                                     left_shift_operator,
                                     right_shift_operator,
                                     logical_and_operator,
                                     logical_or_operator,
                                     equals_operator,
                                     not_equals_operator,
                                     less_than_operator,
                                     less_than_or_equal_operator,
                                     greater_than_operator,
                                     greater_than_or_equal_operator,
                                     assignment_operator,
                                     compound_plus_operator,
                                     compound_subtract_operator,
                                     compound_multiply_operator,
                                     compound_divide_operator,
                                     compound_remainder_operator,
                                     compound_bitwise_and_operator,
                                     compound_bitwise_or_operator,
                                     compound_bitwise_xor_operator,
                                     compound_left_shift_operator,
                                     compound_right_shift_operator>;
struct binary_node;
struct conditional_node;
struct if_node;
struct unary_node;
struct assignment_node;

struct int_constant
{
    int32_t value;
};

struct var
{
    identifier name;
};

using expression = std::variant<int_constant,
                                var,
                                std::unique_ptr<unary_node>,
                                std::unique_ptr<binary_node>,
                                std::unique_ptr<assignment_node>,
                                std::unique_ptr<conditional_node>>;

struct assignment_node
{
    assignment_node(binary_operator op_, expression lhs_, expression rhs_)
      : op(op_)
      , lhs(std::move(lhs_))
      , rhs(std::move(rhs_))
    {
    }
    binary_operator op;
    expression lhs;
    expression rhs;
};

struct conditional_node
{
    expression condition;
    expression e1;
    expression e2;
};

struct unary_node
{
    unary_node(unary_operator op_, expression expression_)
      : op(op_)
      , exp(std::move(expression_))
    {
    }
    unary_operator op;
    expression exp;
};

struct binary_node
{
    binary_node(binary_operator op_, expression left_, expression right_)
      : op(op_)
      , left(std::move(left_))
      , right(std::move(right_))
    {
    }

    binary_operator op;
    expression left;
    expression right;
};

struct return_node
{
    expression e;
};

using statement = std::variant<return_node, expression, std::unique_ptr<if_node>, std::monostate>;

struct if_node
{
    expression op;
    statement then_stmt;
    std::optional<statement> else_stmt;
};

struct declaration
{
    identifier name;
    std::optional<expression> init;
};

using block_item = std::variant<declaration, statement, std::monostate>;

struct function
{
    identifier function_name;
    std::vector<block_item> body;
};

struct program
{
    function f;
};

std::expected<block_item, parser_error> parse_block_item(tokens &tokens);
std::expected<expression, parser_error> parse_conditional(tokens &tokens);
std::expected<int_constant, parser_error> parse_constant(tokens &tokens);
std::expected<declaration, parser_error> parse_declaration(tokens &tokens);
std::expected<identifier, parser_error> parse_identifier(tokens &tokens);
std::expected<std::unique_ptr<if_node>, parser_error> parse_if_node(tokens &tokens);
std::expected<expression, parser_error> parse_expression(tokens &tokens, int32_t min_precedence = 0);
std::expected<expression, parser_error> parse_factor(tokens &tokens);
std::optional<parser_error> parse_semicolon(tokens &tokens);
std::expected<statement, parser_error> parse_statement(tokens &tokens);
std::expected<std::unique_ptr<unary_node>, parser_error> parse_unary_node(tokens &tokens);

std::expected<program, parser_error> parse(tokens &tokens);

std::string pretty_print(const binary_operator &node, int32_t ident = 0);
std::string pretty_print(const declaration &node, int32_t ident = 0);
std::string pretty_print(const expression &node, int32_t ident = 0);
std::string pretty_print(const function &node, int32_t ident = 0);
std::string pretty_print(const identifier &node, int32_t ident = 0);
std::string pretty_print(const int_constant &node, int32_t ident = 0);
std::string pretty_print(const program &node, int32_t ident = 0);
std::string pretty_print(const statement &node, int32_t ident = 0);
std::string pretty_print(const return_node &node, int32_t ident = 0);
std::string pretty_print(const std::unique_ptr<assignment_node> &node, int32_t ident = 0);
std::string pretty_print(const std::unique_ptr<binary_node> &node, int32_t ident = 0);
std::string pretty_print(const std::unique_ptr<conditional_node> &node, int32_t ident = 0);
std::string pretty_print(const std::unique_ptr<if_node> &node, int32_t ident = 0);
std::string pretty_print(const std::unique_ptr<unary_node> &node, int32_t ident = 0);
std::string pretty_print(const std::vector<block_item> &node, int32_t ident = 0);
std::string pretty_print(const unary_operator &node, int32_t ident = 0);
std::string pretty_print(const var &node, int32_t ident = 0);

} // namespace wccff::parser

#endif // PARSER_H
