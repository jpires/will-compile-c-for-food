#ifndef LEXER_H
#define LEXER_H

#include <cstdint>
#include <expected>
#include <filesystem>
#include <fmt/format.h>
#include <optional>
#include <ostream>
#include <string>
#include <string_view>
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

} // namespace wccff

template<>
struct fmt::formatter<wccff::token_type> : formatter<string_view>
{
    template<typename FormatContext>
    auto format(wccff::token_type c, FormatContext &ctx) const
    {
        using wccff::token_type;
        string_view str = "wccff::token_type::invalid";
        switch (c)
        {
            case token_type::identifier:
                str = "Identifier";
                break;
            case token_type::constant:
                str = "Constant";
                break;
            case token_type::int_keyword:
                str = "Int Keyword";
                break;
            case token_type::void_keyword:
                str = "Void Keyword";
                break;
            case token_type::return_keyword:
                str = "Return Keyword";
                break;
            case token_type::open_parenthesis:
                str = "Open Parenthesis";
                break;
            case token_type::close_parenthesis:
                str = "Close Parenthesis";
                break;
            case token_type::open_brace:
                str = "Open Brace";
                break;
            case token_type::close_brace:
                str = "Close Brace";
                break;
            case token_type::semicolon:
                str = "Semicolon";
                break;
        }
        return formatter<string_view>::format(str, ctx);
    }
};

template<>
struct fmt::formatter<wccff::file_location> : formatter<string_view>
{
    template<typename FormatContext>
    auto format(wccff::file_location loc, FormatContext &ctx) const
    {
        auto str = fmt::format("{}:{}", loc.line, loc.column);
        return formatter<string_view>::format(str, ctx);
    }
};
#endif // LEXER_H
