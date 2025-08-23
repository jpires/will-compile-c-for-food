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

#include "common.h"
#include "lexer.h"
#include "visitor.h"
#include <compare>
#include <expected>
#include <span>
#include <utility>
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

    [[nodiscard]] wccff::lexer::token peek(std::size_t pos = 0) const { return m_tokens.at(m_index + pos); }
    [[nodiscard]] wccff::lexer::token peek_after_next() const { return m_tokens.at(m_index + 1); }
    [[nodiscard]] wccff::lexer::token previous_token() const { return m_tokens.at(m_index - 1); }

  private:
    std::vector<wccff::lexer::token> m_tokens;
    std::size_t m_index{ 0 };
};

struct parser_error
{
    std::string message;
};

struct binary_node;
struct do_while_statement;
struct cast_expression;
struct compound_statement;
struct conditional_node;
struct for_statement;
struct function_declaration;
struct function_call;
struct if_node;
struct labelled_statement;
struct unary_node;
struct assignment_node;
struct while_statement;

struct param
{
    param(identifier name_, type p_type_)
      : name(std::move(name_))
      , p_type(std::move(p_type_))
    {
    }
    identifier name;
    type p_type;
};
enum class storage_class
{
    no_storage,
    extern_storage,
    static_storage,
};

struct specifier
{
    storage_class storage = storage_class::no_storage;
    type t;
};

struct var
{
    identifier name;
    std::optional<type> type;
};

using expression = std::variant<constant,
                                var,
                                std::unique_ptr<unary_node>,
                                std::unique_ptr<binary_node>,
                                std::unique_ptr<cast_expression>,
                                std::unique_ptr<assignment_node>,
                                std::unique_ptr<conditional_node>,
                                std::unique_ptr<function_call>>;

struct assignment_node
{
    assignment_node(expression lhs_, expression rhs_);
    assignment_node(expression lhs_, expression rhs_, std::optional<type> type_);
    expression lhs;
    expression rhs;
    std::optional<type> type;
};

struct cast_expression
{
    type target;
    expression exp;
    std::optional<type> type;
};

struct conditional_node
{
    expression condition;
    expression e1;
    expression e2;
    std::optional<type> type;
};

struct function_call
{
    identifier name;
    std::vector<expression> arguments;
    std::optional<type> type;
};

struct unary_node
{
    unary_node(unary_operator op_, expression expression_);
    unary_node(unary_operator op_, expression expression_, std::optional<type> type_);
    unary_operator op;
    expression exp;
    std::optional<type> type;
};

struct binary_node
{
    binary_node(binary_operator op_, expression left_, expression right_);
    binary_node(binary_operator op_, expression left_, expression right_, std::optional<type> type_);

    binary_operator op;
    expression left;
    expression right;
    std::optional<type> type;
};

struct break_statement
{
    identifier label;
};
struct continue_statement
{
    identifier label;
};
struct goto_statement
{
    identifier label;
};
struct return_node
{
    expression e;
};

using statement = std::variant<return_node,
                               expression,
                               std::unique_ptr<if_node>,
                               std::unique_ptr<compound_statement>,
                               break_statement,
                               continue_statement,
                               goto_statement,
                               std::unique_ptr<while_statement>,
                               std::unique_ptr<do_while_statement>,
                               std::unique_ptr<for_statement>,
                               std::unique_ptr<labelled_statement>,
                               std::monostate>;

struct do_while_statement
{
    statement body;
    expression condition;
    identifier label;
};

struct labelled_statement
{
    identifier label;
    statement body;
};

struct while_statement
{
    expression condition;
    statement body;
    identifier label;
};

struct if_node
{
    expression op;
    statement then_stmt;
    std::optional<statement> else_stmt;
};

struct variable_declaration
{
    identifier name;
    std::optional<expression> init;
    type var_type;
    storage_class storage_class;
};

struct init_declaration
{
    variable_declaration decl;
};
struct init_expression
{
    std::optional<expression> expression;
};

using for_init = std::variant<init_declaration, init_expression>;

struct for_statement
{
    for_init init;
    std::optional<expression> condition;
    std::optional<expression> post;
    statement body;
    identifier label;
};

using declaration = std::variant<function_declaration, variable_declaration>;

using block_item = std::variant<declaration, statement, std::monostate>;

struct block
{
    std::vector<block_item> items;
};

struct function_declaration
{
    identifier name;
    std::vector<identifier> arguments;
    std::optional<block> body;
    type f_type;
    storage_class storage_class;
};

struct compound_statement
{
    block block;
};
struct function
{
    identifier function_name;
    block body;
};

struct program
{
    std::vector<declaration> f;
};

expression convert_to(const expression &e, const type &t);

/**
 * This function consumes the tokens on list from the input. It consumes them in the same order.
 * It will return an error, if the input doesn't have the expected tokens.
 * @param tokens The input tokens
 * @param list The expected tokens to be consumed
 * @return A parse_error if the input doesn't contain all the tokens on list
 */
std::optional<parser_error> consume_tokens(tokens &tokens, const std::vector<lexer::token_type> &list);

std::unique_ptr<assignment_node> copy_assignment_node(const std::unique_ptr<assignment_node> &node);
std::unique_ptr<binary_node> copy_binary_node(const std::unique_ptr<binary_node> &node);
std::unique_ptr<cast_expression> copy_cast_expresion(const std::unique_ptr<cast_expression> &node);
std::unique_ptr<conditional_node> copy_conditional_node(const std::unique_ptr<conditional_node> &node);
variable_declaration copy_declaration(const variable_declaration &node);
expression copy_expression(const expression &expression);
std::unique_ptr<function_call> copy_function_call(const std::unique_ptr<function_call> &n);
std::unique_ptr<unary_node> copy_unary_node(const std::unique_ptr<unary_node> &node);

type get_type(const constant &n);
type get_type(const expression &n);
type get_type(const std::unique_ptr<assignment_node> &n);
type get_type(const std::unique_ptr<binary_node> &n);
type get_type(const std::unique_ptr<cast_expression> &n);
type get_type(const std::unique_ptr<conditional_node> &n);
type get_type(const std::unique_ptr<function_call> &n);
type get_type(const std::unique_ptr<unary_node> &n);
type get_type(const var &n);

/// \brief Returns true if the token is a type specifier as declared in 6.7.2 Type specifiers
bool is_type_specifier(const lexer::token &t);

/// \brief Returns true if the token is a storage specifier as declared in 6.7.1 Storage-class specifiers
bool is_storage_specifier(const lexer::token &t);

std::expected<std::vector<expression>, parser_error> parse_argument_list(tokens &tokens);
std::expected<block_item, parser_error> parse_block_item(tokens &tokens);
std::expected<block, parser_error> parse_block(tokens &tokens);
std::expected<std::unique_ptr<cast_expression>, parser_error> parse_cast_expression(tokens &tokens);
std::expected<std::unique_ptr<compound_statement>, parser_error> parse_compound_statement(tokens &tokens);
std::expected<expression, parser_error> parse_conditional(tokens &tokens);
std::expected<constant, parser_error> parse_constant(tokens &tokens);
std::expected<std::unique_ptr<do_while_statement>, parser_error> parse_do_while(tokens &tokens);
std::expected<declaration, parser_error> parse_declaration(tokens &tokens);
std::expected<for_init, parser_error> parse_for_init(tokens &tokens);
std::expected<std::unique_ptr<for_statement>, parser_error> parse_for_statement(tokens &tokens);
std::expected<std::unique_ptr<function_call>, parser_error> parse_function_call(tokens &tokens);
std::expected<function_declaration, parser_error> parse_function_declaration(tokens &tokens, specifier specifieres);
std::expected<identifier, parser_error> parse_identifier(tokens &tokens);
std::expected<std::unique_ptr<if_node>, parser_error> parse_if_node(tokens &tokens);
std::expected<expression, parser_error> parse_expression(tokens &tokens, int32_t min_precedence = 0);
std::expected<expression, parser_error> parse_factor(tokens &tokens);
std::expected<std::vector<param>, parser_error> parse_params_list(tokens &tokens);
std::optional<parser_error> parse_semicolon(tokens &tokens);
std::expected<specifier, parser_error> parse_specifier(tokens &tokens);
std::expected<statement, parser_error> parse_statement(tokens &tokens);
std::expected<type, parser_error> parse_type(const std::vector<lexer::token> &tokens);
std::expected<type, parser_error> parse_type_specifier(tokens &token, lexer::token_type stop_token);
std::expected<std::unique_ptr<unary_node>, parser_error> parse_unary_node(tokens &tokens);
std::expected<variable_declaration, parser_error> parse_variable_declaration(tokens &tokens, specifier specifieres);
std::expected<std::unique_ptr<while_statement>, parser_error> parse_while_statement(tokens &tokens);

std::expected<program, parser_error> parse(tokens &tokens);

std::string pretty_print(const block &node, int32_t ident = 0);
std::string pretty_print(const block_item &node, int32_t ident = 0);
std::string pretty_print(const break_statement &node, int32_t ident = 0);
std::string pretty_print(const constant &node, int32_t ident = 0);
std::string pretty_print(const continue_statement &node, int32_t ident = 0);
std::string pretty_print(const declaration &node, int32_t ident = 0);
std::string pretty_print(const double_constant &node, int32_t ident = 0);
std::string pretty_print(const expression &node, int32_t ident = 0);
std::string pretty_print(const for_init &node, int32_t ident = 0);
std::string pretty_print(const function &node, int32_t ident = 0);
std::string pretty_print(const function_declaration &node, int32_t ident = 0);
std::string pretty_print(const goto_statement &node, int32_t ident = 0);
std::string pretty_print(const identifier &node, int32_t ident = 0);
std::string pretty_print(const int_constant &node, int32_t ident = 0);
std::string pretty_print(const long_constant &node, int32_t ident = 0);
std::string pretty_print(const program &node, int32_t ident = 0);
std::string pretty_print(const statement &node, int32_t ident = 0);
std::string pretty_print(const storage_class &node, int32_t ident = 0);
std::string pretty_print(const return_node &node, int32_t ident = 0);
std::string pretty_print(const std::optional<expression> &node, int32_t ident = 0);
std::string pretty_print(const std::optional<type> &node, int32_t ident = 0);
std::string pretty_print(const std::unique_ptr<assignment_node> &node, int32_t ident = 0);
std::string pretty_print(const std::unique_ptr<binary_node> &node, int32_t ident = 0);
std::string pretty_print(const std::unique_ptr<cast_expression> &node, int32_t ident = 0);
std::string pretty_print(const std::unique_ptr<compound_statement> &node, int32_t ident = 0);
std::string pretty_print(const std::unique_ptr<conditional_node> &node, int32_t ident = 0);
std::string pretty_print(const std::unique_ptr<do_while_statement> &node, int32_t ident = 0);
std::string pretty_print(const std::unique_ptr<for_statement> &node, int32_t ident = 0);
std::string pretty_print(const std::unique_ptr<function_call> &node, int32_t ident = 0);
std::string pretty_print(const std::unique_ptr<fun_type> &node, int32_t ident = 0);
std::string pretty_print(const std::unique_ptr<if_node> &node, int32_t ident = 0);
std::string pretty_print(const std::unique_ptr<labelled_statement> &node, int32_t ident = 0);
std::string pretty_print(const std::unique_ptr<unary_node> &node, int32_t ident = 0);
std::string pretty_print(const std::unique_ptr<while_statement> &node, int32_t ident = 0);
std::string pretty_print(const type &node, int32_t ident = 0);
std::string pretty_print(const unsigned_int_constant &node, int32_t ident = 0);
std::string pretty_print(const unsigned_long_constant &node, int32_t ident = 0);
std::string pretty_print(const var &node, int32_t ident = 0);
std::string pretty_print(const variable_declaration &node, int32_t ident = 0);

} // namespace wccff::parser

#endif // PARSER_H
