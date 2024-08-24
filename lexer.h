#ifndef LEXER_H
#define LEXER_H

#include <cstdint>
#include <expected>
#include <filesystem>
#include <optional>
#include <ostream>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

namespace wccff {

struct file_location
{
    int32_t line{ 0 };
    int32_t column{ 0 };
};

enum class token_type
{
    identifier,
    constant,
    int_keyword,
    void_keyword,
    return_keyword,
    open_parenthesis,
    close_parenthesis,
    open_brace,
    close_brace,
    semicolon,
};

inline std::ostream &operator<<(std::ostream &os, const token_type &t)
{
    switch (t)
    {
        case token_type::identifier:
            os << "Identifier";
            return os;
        case token_type::constant:
            os << "Constant";
            return os;
        case token_type::int_keyword:
            os << "Int Keyword";
            return os;
        case token_type::void_keyword:
            os << "Void Keyword";
            return os;
        case token_type::return_keyword:
            os << "Return Keyword";
            return os;
        case token_type::open_parenthesis:
            os << "Open Parenthesis";
            return os;
        case token_type::close_parenthesis:
            os << "Close Parenthesis";
            return os;
        case token_type::open_brace:
            os << "Open Brace";
            return os;
        case token_type::close_brace:
            os << "Close Brace";
            return os;
        case token_type::semicolon:
            os << "Semicolon";
            return os;
    }
}

struct lexer_error
{
    lexer_error(file_location location_, std::string_view input_, std::string message_ = "")
      : location(location_)
      , input(input_)
      , message(std::move(message_))
    {
    }

    file_location location;
    std::string_view input;
    std::string message;
};

struct token
{
    token(token_type t_, std::string_view v_, file_location loc_)
      : t(t_)
      , c(v_)
      , loc(loc_)
    {
    }
    token_type t;
    std::string_view c;
    file_location loc;
};

std::expected<std::vector<token>, lexer_error> lexer(std::string_view input, file_location location = {}) noexcept;

std::expected<std::string, std::error_code> read_file(const std::filesystem::path &file_name);

class tokens
{
  public:
    explicit tokens(std::vector<token> tokens_)
      : m_tokens(std::move(tokens_))
    {
    }

    std::optional<token> get_next_token()
    {
        if (m_index >= m_tokens.size())
        {
            return std::nullopt;
        }
        return m_tokens[m_index++];
    }

  private:
    std::vector<token> m_tokens;
    std::size_t m_index{ 0 };
};

} // namespace wccff
#endif // LEXER_H
