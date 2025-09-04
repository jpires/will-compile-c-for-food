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

#ifndef PARSER_HELPERS_H
#define PARSER_HELPERS_H

#include "parser.h"
namespace wccff::testing {

template<typename Type>
constexpr bool is_constant_expression_of(const wccff::parser::expression &exp, int64_t value)
{
    if (std::holds_alternative<wccff::constant>(exp) == false)
    {
        return false;
    }

    auto &inner = std::get<wccff::constant>(exp);
    if (std::holds_alternative<Type>(inner) == false)
    {
        return false;
    }
    return std::get<Type>(inner).value == value;
}

inline std::string_view get_token_text(wccff::lexer::token_type token)
{
    switch (token)
    {
        case wccff::lexer::token_type::identifier:
            return { "identifier" };
        case wccff::lexer::token_type::floating_porint_constant:
            return { "42.0" };
        case wccff::lexer::token_type::int_constant:
            return { "42" };
        case wccff::lexer::token_type::long_constant:
            return { "42l" };
        case wccff::lexer::token_type::unsigned_int_constant:
            return { "42u" };
        case wccff::lexer::token_type::unsigned_long_constant:
            return { "42lu" };
        case wccff::lexer::token_type::break_keyword:
            return { "break" };
        case wccff::lexer::token_type::continue_keyword:
            return { "continue" };
        case wccff::lexer::token_type::do_keyword:
            return { "do" };
        case wccff::lexer::token_type::double_keyword:
            return { "double" };
        case wccff::lexer::token_type::else_keyword:
            return { "else" };
        case wccff::lexer::token_type::extern_keyword:
            return { "extern" };
        case wccff::lexer::token_type::for_keyword:
            return { "for" };
        case wccff::lexer::token_type::goto_keyword:
            return { "goto" };
        case wccff::lexer::token_type::if_keyword:
            return { "if" };
        case wccff::lexer::token_type::int_keyword:
            return { "int" };
        case wccff::lexer::token_type::long_keyword:
            return { "long" };
        case wccff::lexer::token_type::void_keyword:
            return { "void" };
        case wccff::lexer::token_type::static_keyword:
            return { "static" };
        case wccff::lexer::token_type::return_keyword:
            return { "return" };
        case wccff::lexer::token_type::signed_keyword:
            return { "signed" };
        case wccff::lexer::token_type::unsigned_keyword:
            return { "unsigned" };
        case wccff::lexer::token_type::while_keyword:
            return { "while" };
        case wccff::lexer::token_type::open_parenthesis:
            return { "(" };
        case wccff::lexer::token_type::close_parenthesis:
            return { ")" };
        case wccff::lexer::token_type::open_brace:
            return { "{" };
        case wccff::lexer::token_type::close_brace:
            return { "}" };
        case wccff::lexer::token_type::semicolon:
            return { "," };
        case wccff::lexer::token_type::negation_operator:
            return { "-" };
        case wccff::lexer::token_type::decrement_operator:
            return { "--" };
        case wccff::lexer::token_type::bitwise_and_operator:
            return { "&" };
        case wccff::lexer::token_type::bitwise_or_operator:
            return { "|" };
        case wccff::lexer::token_type::bitwise_xor_operator:
            return { "^" };
        case wccff::lexer::token_type::plus_operator:
            return { "+" };
        case wccff::lexer::token_type::multiplication_operator:
            return { "*" };
        case wccff::lexer::token_type::remainder_operator:
            return { "%" };
        case wccff::lexer::token_type::left_shift_operator:
            return { "<<" };
        case wccff::lexer::token_type::right_shift_operator:
            return { ">>" };
        case wccff::lexer::token_type::not_operator:
            return { "!" };
        case wccff::lexer::token_type::and_operator:
            return { "&&" };
        case wccff::lexer::token_type::or_operator:
            return { "||" };
        case wccff::lexer::token_type::equals_operator:
            return { "==" };
        case wccff::lexer::token_type::not_equals_operator:
            return { "!=" };
        case wccff::lexer::token_type::less_than_operator:
            return { "<" };
        case wccff::lexer::token_type::less_than_or_equal_operator:
            return { "<=" };
        case wccff::lexer::token_type::greater_than_operator:
            return { ">" };
        case wccff::lexer::token_type::greater_than_or_equal_operator:
            return { ">=" };
        case wccff::lexer::token_type::assignment_operator:
            return { "=" };
        case wccff::lexer::token_type::compound_plus:
            return { "+=" };
        case wccff::lexer::token_type::compound_minus:
            return { "-=" };
        case wccff::lexer::token_type::compound_multiplication:
            return { "*=" };
        case wccff::lexer::token_type::compound_division:
            return { "/=" };
        case wccff::lexer::token_type::compound_remainder:
            return { "%=" };
        case wccff::lexer::token_type::compound_bitwise_and:
            return { "&=" };
        case wccff::lexer::token_type::compound_bitwise_or:
            return { "|=" };
        case wccff::lexer::token_type::compound_bitwise_xor:
            return { "^=" };
        case wccff::lexer::token_type::compound_left_shift:
            return { "<<=" };
        case wccff::lexer::token_type::compound_right_shift:
            return { ">>=" };
        case wccff::lexer::token_type::increment_operator:
            return { "++" };
        case wccff::lexer::token_type::question_mark:
            return { "?" };
        case wccff::lexer::token_type::colon:
            return { ":" };
        case wccff::lexer::token_type::comma:
            return { "," };
    }
    return { "UNKNOWN" };
}

inline wccff::parser::tokens build_tokens(const std::vector<wccff::lexer::token_type> &ts)
{
    wccff::lexer::file_location location{ 0, 0 };
    std::vector<wccff::lexer::token> tokens;
    tokens.reserve(ts.size());

    for (const auto &t : ts)
    {
        tokens.emplace_back(t, get_token_text(t), location);
    }

    return wccff::parser::tokens{ tokens };
}

} // namespace wccff::testing

#endif // PARSER_HELPERS_H
