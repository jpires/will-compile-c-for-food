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

#ifndef LEXER_H
#define LEXER_H

#include <compare>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <fmt/format.h>
#include <optional>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

namespace wccff::lexer {

struct file_location
{
    int32_t line{ 0 };
    int32_t column{ 0 };
    bool operator==(const file_location &) const = default;
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
    negation_operator,
    decrement_operator,
    bitwise_complement_operator,
    bitwise_and_operator,
    bitwise_or_operator,
    bitwise_xor_operator,
    plus_operator,
    multiplication_operator,
    division_operator,
    remainder_operator,
    left_shift_operator,
    right_shift_operator,
    not_operator,
    and_operator,
    or_operator,
    equals_operator,
    not_equals_operator,
    less_than_operator,
    less_than_or_equal_operator,
    greater_than_operator,
    greater_than_or_equal_operator,
    assignment_operator,
};

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
    token(token_type type_, std::string_view text_, file_location loc_)
      : type(type_)
      , text(text_)
      , loc(loc_)
    {
    }
    token_type type;
    std::string_view text;
    file_location loc;
};

std::expected<std::vector<token>, lexer_error> lexer(std::string_view input, file_location location = {}) noexcept;

std::expected<std::string, std::error_code> read_file(const std::filesystem::path &file_name);

} // namespace wccff::lexer

template<>
struct fmt::formatter<wccff::lexer::token_type> : formatter<string_view>
{
    template<typename FormatContext>
    auto format(wccff::lexer::token_type c, FormatContext &ctx) const
    {
        using wccff::lexer::token_type;
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
            case token_type::negation_operator:
                str = "Negation Operator";
                break;
            case token_type::decrement_operator:
                str = "Decrment Operator";
                break;
            case token_type::bitwise_complement_operator:
                str = "Bitwise Complement Operator";
                break;
            case token_type::bitwise_and_operator:
                str = "Bitwise And Operator";
                break;
            case token_type::bitwise_or_operator:
                str = "Bitwise Or Operator";
                break;
            case token_type::bitwise_xor_operator:
                str = "Bitwise Xor Operator";
                break;
            case token_type::plus_operator:
                str = "Plus Operator";
                break;
            case token_type::multiplication_operator:
                str = "Multiplication Operator";
                break;
            case token_type::division_operator:
                str = "Division Operator";
                break;
            case token_type::remainder_operator:
                str = "Remainder Operator";
                break;
            case token_type::left_shift_operator:
                str = "Left Shift Operator";
                break;
            case token_type::right_shift_operator:
                str = "Right Shift Operator";
                break;
            case token_type::not_operator:
                str = "Not Operator";
                break;
            case token_type::and_operator:
                str = "And Operator";
                break;
            case token_type::or_operator:
                str = "Or Operator";
                break;
            case token_type::equals_operator:
                str = "Equals Operator";
                break;
            case token_type::not_equals_operator:
                str = "Not Equals Operator";
                break;
            case token_type::less_than_operator:
                str = "Less Than Operator";
                break;
            case token_type::less_than_or_equal_operator:
                str = "Lestt Than or Equal To Operator";
                break;
            case token_type::greater_than_operator:
                str = "Greater Than Operator";
                break;
            case token_type::greater_than_or_equal_operator:
                str = "Greater Than or Equal To Operator";
                break;
            case token_type::assignment_operator:
                str = "Assignment Operator";
                break;
        }
        return formatter<string_view>::format(str, ctx);
    }
};

template<>
struct fmt::formatter<wccff::lexer::file_location> : formatter<string_view>
{
    template<typename FormatContext>
    auto format(wccff::lexer::file_location loc, FormatContext &ctx) const
    {
        auto str = fmt::format("{}:{}", loc.line, loc.column);
        return formatter<string_view>::format(str, ctx);
    }
};

namespace wccff::lexer {
inline std::ostream &operator<<(std::ostream &os, const token_type &t)
{
    os << fmt::format("{}", t);
    return os;
}
} // namespace wccff::lexer
#endif // LEXER_H
