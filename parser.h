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

std::expected<int_constant, parser_error> parse_constant(tokens &tokens);
std::expected<identifier, parser_error> parse_identifier(tokens &tokens);
std::expected<expression, parser_error> parse_expression(tokens &tokens);
std::expected<statement, parser_error> parse_statement(tokens &tokens);

std::expected<program, parser_error> parse(tokens &tokens);

} // namespace wccff::parser

#endif // PARSER_H
