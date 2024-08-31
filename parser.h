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
struct negate_operator
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
using unary_operator = std::variant<bitwise_complement_operator, negate_operator>;

using binary_operator = std::variant<plus_operator,
                                     subtract_operator,
                                     multiply_operator,
                                     divide_operator,
                                     remainder_operator,
                                     bitwise_and_operator>;
struct binary_node;
struct unary_node;

struct int_constant
{
    int32_t value;
};
using expression = std::variant<int_constant, std::unique_ptr<unary_node>, std::unique_ptr<binary_node>>;

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

std::expected<int_constant, parser_error> parse_constant(tokens &tokens);
std::expected<identifier, parser_error> parse_identifier(tokens &tokens);
std::expected<expression, parser_error> parse_expression(tokens &tokens, int32_t min_precedence = 0);
std::expected<expression, parser_error> parse_factor(tokens &tokens);
std::expected<statement, parser_error> parse_statement(tokens &tokens);
std::expected<std::unique_ptr<unary_node>, parser_error> parse_unary_node(tokens &tokens);

std::expected<program, parser_error> parse(tokens &tokens);

std::string pretty_print(const expression &node, int32_t ident);
std::string pretty_print(const function &node, int32_t ident);
std::string pretty_print(const program &node, int32_t ident = 0);
} // namespace wccff::parser

#endif // PARSER_H
