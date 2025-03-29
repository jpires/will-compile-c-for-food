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

#include "lexer.h"
#include <algorithm>
#include <array>
#include <ctre.hpp>
#include <fstream>
#include <iostream>
#include <iterator>

namespace wccff::lexer {

constexpr auto identifier_pattern{ R"(([a-zA-Z_]\w*\b))" };
constexpr auto constant_pattern{ R"(([0-9]+\b))" };
constexpr auto open_parenthesis_pattern{ R"((\())" };
constexpr auto close_parenthesis_pattern{ R"((\)))" };
constexpr auto open_brace_pattern{ R"((\{))" };
constexpr auto close_brace_pattern{ R"((\}))" };
constexpr auto semicolon_pattern{ "(;)" };
constexpr auto decrement_operator_pattern{ "(--)" };
constexpr auto negate_operator_pattern{ "(-)" };
constexpr auto bitwise_complement_operator_pattern{ "(~)" };
constexpr auto plus_operator_pattern{ R"((\+))" };
constexpr auto multiplication_operator_pattern{ R"((\*))" };
constexpr auto division_operator_pattern{ "(/)" };
constexpr auto remainder_operator_pattern{ "(%)" };
constexpr auto bitwise_and_operator_pattern{ "(&)" };
constexpr auto bitwise_or_operator_pattern{ R"((\|))" };
constexpr auto bitwise_xor_operator_pattern{ R"((\^))" };
constexpr auto left_shift_operator_pattern{ R"((<<))" };
constexpr auto right_shift_operator_pattern{ R"((>>))" };
constexpr auto not_operator_pattern{ R"((!))" };
constexpr auto and_operator_pattern{ R"((&&))" };
constexpr auto or_operator_pattern{ R"((\|\|))" };
constexpr auto equals_operator_pattern{ R"((==))" };
constexpr auto not_equals_operator_pattern{ R"((!=))" };
constexpr auto less_than_operator_pattern{ R"((<))" };
constexpr auto less_than_or_equal_operator_pattern{ R"((<=))" };
constexpr auto greater_than_operator_pattern{ R"((>))" };
constexpr auto greater_than_or_equal_operator_pattern{ R"((>=))" };
constexpr auto assignment_operator_pattern{ R"((=))" };
constexpr auto compound_plus_pattern{ R"((\+=))" };
constexpr auto compound_minus_pattern{ R"((-=))" };
constexpr auto compound_multiplication_pattern{ R"((\*=))" };
constexpr auto compound_division_pattern{ R"((/=))" };
constexpr auto compound_remainder_pattern{ R"((%=))" };
constexpr auto compound_bitwise_and_pattern{ R"((&=))" };
constexpr auto compound_bitwise_or_pattern{ R"((\|=))" };
constexpr auto compound_bitwise_xor_pattern{ R"((\^=))" };
constexpr auto compound_left_shift_pattern{ R"((<<=))" };
constexpr auto compound_right_shift_pattern{ R"((>>=))" };
constexpr auto increment_operator_pattern{ R"((\+\+))" };
constexpr auto question_mark_pattern{ R"((\?))" };
constexpr auto colon_pattern{ R"((:))" };
constexpr auto comma_pattern{ R"((,))" };

constexpr auto get_patters()
{
    return std::array{
        identifier_pattern,
        constant_pattern,
        open_parenthesis_pattern,
        close_parenthesis_pattern,
        open_brace_pattern,
        close_brace_pattern,
        semicolon_pattern,
        // Three chars operators
        compound_left_shift_pattern,
        compound_right_shift_pattern,
        // Two chars operators
        and_operator_pattern,
        or_operator_pattern,
        equals_operator_pattern,
        not_equals_operator_pattern,
        decrement_operator_pattern,
        less_than_or_equal_operator_pattern,
        greater_than_or_equal_operator_pattern,
        left_shift_operator_pattern,
        right_shift_operator_pattern,
        compound_plus_pattern,
        compound_minus_pattern,
        compound_multiplication_pattern,
        compound_division_pattern,
        compound_remainder_pattern,
        compound_bitwise_and_pattern,
        compound_bitwise_and_pattern,
        compound_bitwise_or_pattern,
        compound_bitwise_xor_pattern,
        increment_operator_pattern,
        //  One char Operator
        negate_operator_pattern,
        not_operator_pattern,
        bitwise_complement_operator_pattern,
        plus_operator_pattern,
        multiplication_operator_pattern,
        division_operator_pattern,
        remainder_operator_pattern,
        bitwise_and_operator_pattern,
        bitwise_or_operator_pattern,
        bitwise_xor_operator_pattern,
        less_than_operator_pattern,
        greater_than_operator_pattern,
        assignment_operator_pattern,
        question_mark_pattern,
        colon_pattern,
        comma_pattern,
    };
}

constexpr bool str_compare(const char *p1, const char *p2)
{
    while (*p1 || *p2)
    {
        if (*p1 != *p2)
        {
            return false;
        }
        p1++;
        p2++;
    }
    return true;
}
constexpr std::size_t calculate_final_pattern_size(auto patters)
{
    std::size_t length = 0;
    for (const auto &p : patters)
    {
        std::size_t j = 0;
        while (p[j] != '\0')
        {
            length++;
            j++;
        }
    }
    // The final pattern contains all the patters plus a divider in between the patters
    // So the size if the sum of the length for all patterns, plus the number of patterns minus one, plus the '\0' char.
    // Note: the number of dividers is the number of patters - 1, there's no divider at the end.
    return length + patters.size() - 1 + 1;
}

consteval auto create_regex_pattern()
{
    constexpr auto patterns = get_patters();
    std::array<char, calculate_final_pattern_size(patterns)> final_pattern;

    std::size_t index = 0;
    for (const auto &b1 : patterns)
    {
        std::size_t j = 0;
        while (b1[j] != '\0')
        {
            final_pattern[index] = b1[j];
            index++;
            j++;
        }
        final_pattern[index++] = '|';
    }
    final_pattern[index - 1] = '\0';

    return ctll::fixed_string{ final_pattern };
}

consteval std::ptrdiff_t get_pattern_position(const char *p)
{
    auto patterns = get_patters();
    auto f = std::find_if(patterns.begin(), patterns.end(), [p](const char *i) { return str_compare(i, p); });

    return std::distance(patterns.begin(), f) + 1;
}

std::expected<std::vector<token>, lexer_error> lexer(std::string_view input, file_location location) noexcept
{
    std::vector<token> result;

    while (true)
    {
        // Remove trimming white spaces
        while ((input.empty() == false) && std::isspace(input[0]))
        {
            location.column++;
            if (input[0] == '\n')
            {
                location.line++;
                location.column = 0;
            }
            input = input.substr(1);
        }

        if (input.empty())
        {
            return result;
        }

        auto m = ctre::starts_with<create_regex_pattern()>(input);
        if (m)
        {
            if (ctre::get<get_pattern_position(identifier_pattern)>(m))
            {
                if (m == "do")
                {
                    result.emplace_back(token_type::do_keyword, m, location);
                }
                else if (m == "break")
                {
                    result.emplace_back(token_type::break_keyword, m, location);
                }
                else if (m == "continue")
                {
                    result.emplace_back(token_type::continue_keyword, m, location);
                }
                else if (m == "else")
                {
                    result.emplace_back(token_type::else_keyword, m, location);
                }
                else if (m == "extern")
                {
                    result.emplace_back(token_type::extern_keyword, m, location);
                }
                else if (m == "for")
                {
                    result.emplace_back(token_type::for_keyword, m, location);
                }
                else if (m == "goto")
                {
                    result.emplace_back(token_type::goto_keyword, m, location);
                }
                else if (m == "if")
                {
                    result.emplace_back(token_type::if_keyword, m, location);
                }
                else if (m == "int")
                {
                    result.emplace_back(token_type::int_keyword, m, location);
                }
                else if (m == "void")
                {
                    result.emplace_back(token_type::void_keyword, m, location);
                }
                else if (m == "return")
                {
                    result.emplace_back(token_type::return_keyword, m, location);
                }
                else if (m == "static")
                {
                    result.emplace_back(token_type::static_keyword, m, location);
                }
                else if (m == "while")
                {
                    result.emplace_back(token_type::while_keyword, m, location);
                }
                else
                {
                    result.emplace_back(token_type::identifier, m, location);
                }
            }

            if (ctre::get<get_pattern_position(constant_pattern)>(m))
            {
                result.emplace_back(token_type::constant, m, location);
            }

            if (ctre::get<get_pattern_position(open_parenthesis_pattern)>(m))
            {
                result.emplace_back(token_type::open_parenthesis, m, location);
            }

            if (ctre::get<get_pattern_position(close_parenthesis_pattern)>(m))
            {
                result.emplace_back(token_type::close_parenthesis, m, location);
            }
            if (ctre::get<get_pattern_position(open_brace_pattern)>(m))
            {
                result.emplace_back(token_type::open_brace, m, location);
            }
            if (ctre::get<get_pattern_position(close_brace_pattern)>(m))
            {
                result.emplace_back(token_type::close_brace, m, location);
            }
            if (ctre::get<get_pattern_position(semicolon_pattern)>(m))
            {
                result.emplace_back(token_type::semicolon, m, location);
            }
            if (ctre::get<get_pattern_position(and_operator_pattern)>(m))
            {
                result.emplace_back(token_type::and_operator, m, location);
            }
            if (ctre::get<get_pattern_position(or_operator_pattern)>(m))
            {
                result.emplace_back(token_type::or_operator, m, location);
            }
            if (ctre::get<get_pattern_position(equals_operator_pattern)>(m))
            {
                result.emplace_back(token_type::equals_operator, m, location);
            }
            if (ctre::get<get_pattern_position(not_equals_operator_pattern)>(m))
            {
                result.emplace_back(token_type::not_equals_operator, m, location);
            }
            if (ctre::get<get_pattern_position(decrement_operator_pattern)>(m))
            {
                result.emplace_back(token_type::decrement_operator, m, location);
            }
            if (ctre::get<get_pattern_position(negate_operator_pattern)>(m))
            {
                result.emplace_back(token_type::negation_operator, m, location);
            }
            if (ctre::get<get_pattern_position(not_operator_pattern)>(m))
            {
                result.emplace_back(token_type::not_operator, m, location);
            }
            if (ctre::get<get_pattern_position(bitwise_complement_operator_pattern)>(m))
            {
                result.emplace_back(token_type::bitwise_complement_operator, m, location);
            }
            if (ctre::get<get_pattern_position(plus_operator_pattern)>(m))
            {
                result.emplace_back(token_type::plus_operator, m, location);
            }
            if (ctre::get<get_pattern_position(multiplication_operator_pattern)>(m))
            {
                result.emplace_back(token_type::multiplication_operator, m, location);
            }
            if (ctre::get<get_pattern_position(division_operator_pattern)>(m))
            {
                result.emplace_back(token_type::division_operator, m, location);
            }
            if (ctre::get<get_pattern_position(remainder_operator_pattern)>(m))
            {
                result.emplace_back(token_type::remainder_operator, m, location);
            }
            if (ctre::get<get_pattern_position(bitwise_and_operator_pattern)>(m))
            {
                result.emplace_back(token_type::bitwise_and_operator, m, location);
            }
            if (ctre::get<get_pattern_position(bitwise_or_operator_pattern)>(m))
            {
                result.emplace_back(token_type::bitwise_or_operator, m, location);
            }
            if (ctre::get<get_pattern_position(bitwise_xor_operator_pattern)>(m))
            {
                result.emplace_back(token_type::bitwise_xor_operator, m, location);
            }
            if (ctre::get<get_pattern_position(left_shift_operator_pattern)>(m))
            {
                result.emplace_back(token_type::left_shift_operator, m, location);
            }
            if (ctre::get<get_pattern_position(right_shift_operator_pattern)>(m))
            {
                result.emplace_back(token_type::right_shift_operator, m, location);
            }
            if (ctre::get<get_pattern_position(less_than_operator_pattern)>(m))
            {
                result.emplace_back(token_type::less_than_operator, m, location);
            }
            if (ctre::get<get_pattern_position(less_than_or_equal_operator_pattern)>(m))
            {
                result.emplace_back(token_type::less_than_or_equal_operator, m, location);
            }
            if (ctre::get<get_pattern_position(greater_than_operator_pattern)>(m))
            {
                result.emplace_back(token_type::greater_than_operator, m, location);
            }
            if (ctre::get<get_pattern_position(greater_than_or_equal_operator_pattern)>(m))
            {
                result.emplace_back(token_type::greater_than_or_equal_operator, m, location);
            }
            if (ctre::get<get_pattern_position(assignment_operator_pattern)>(m))
            {
                result.emplace_back(token_type::assignment_operator, m, location);
            }
            if (ctre::get<get_pattern_position(compound_plus_pattern)>(m))
            {
                result.emplace_back(token_type::compound_plus, m, location);
            }
            if (ctre::get<get_pattern_position(compound_minus_pattern)>(m))
            {
                result.emplace_back(token_type::compound_minus, m, location);
            }
            if (ctre::get<get_pattern_position(compound_multiplication_pattern)>(m))
            {
                result.emplace_back(token_type::compound_multiplication, m, location);
            }
            if (ctre::get<get_pattern_position(compound_division_pattern)>(m))
            {
                result.emplace_back(token_type::compound_division, m, location);
            }
            if (ctre::get<get_pattern_position(compound_remainder_pattern)>(m))
            {
                result.emplace_back(token_type::compound_remainder, m, location);
            }
            if (ctre::get<get_pattern_position(compound_bitwise_and_pattern)>(m))
            {
                result.emplace_back(token_type::compound_bitwise_and, m, location);
            }
            if (ctre::get<get_pattern_position(compound_bitwise_or_pattern)>(m))
            {
                result.emplace_back(token_type::compound_bitwise_or, m, location);
            }
            if (ctre::get<get_pattern_position(compound_bitwise_xor_pattern)>(m))
            {
                result.emplace_back(token_type::compound_bitwise_xor, m, location);
            }
            if (ctre::get<get_pattern_position(compound_left_shift_pattern)>(m))
            {
                result.emplace_back(token_type::compound_left_shift, m, location);
            }
            if (ctre::get<get_pattern_position(compound_right_shift_pattern)>(m))
            {
                result.emplace_back(token_type::compound_right_shift, m, location);
            }
            if (ctre::get<get_pattern_position(increment_operator_pattern)>(m))
            {
                result.emplace_back(token_type::increment_operator, m, location);
            }
            if (ctre::get<get_pattern_position(question_mark_pattern)>(m))
            {
                result.emplace_back(token_type::question_mark, m, location);
            }
            if (ctre::get<get_pattern_position(colon_pattern)>(m))
            {
                result.emplace_back(token_type::colon, m, location);
            }
            if (ctre::get<get_pattern_position(comma_pattern)>(m))
            {
                result.emplace_back(token_type::comma, m, location);
            }

            if (result.empty())
            {
                return std::unexpected(lexer_error{ location, m, "Unhandled match" });
            }

            location.column += m.size();

            input = input.substr(m.size());
            continue;
        }

        return std::unexpected(lexer_error{ location, input, "Failed to find a match" });
    }
}

std::expected<std::string, std::error_code> read_file(const std::filesystem::path &file_name)
{
    std::ifstream file{ file_name, std::ios::ate };
    if (file.is_open() == false)
    {
        return std::unexpected(std::error_code(errno, std::generic_category()));
    }

    auto file_size = file.tellg();

    std::string content(file_size, '\0');

    file.seekg(0);
    file.read(content.data(), file_size);

    return content;
}
} // namespace wccff::lexer
