#ifndef LEXER_H
#define LEXER_H

#include <expected>
#include <filesystem>
#include <ostream>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

namespace wccff {
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

struct token
{
    token(token_type t_, std::string_view v_)
      : t(t_)
      , c(v_)
    {
    }
    token_type t;
    std::string_view c;
};

std::expected<std::vector<token>, std::error_code> lexer(std::string_view input) noexcept;

std::expected<std::string, std::error_code> read_file(const std::filesystem::path &file_name);

} // namespace wccff
#endif // LEXER_H
