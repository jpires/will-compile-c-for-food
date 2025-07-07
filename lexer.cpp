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
#include <unordered_map>

namespace wccff::lexer {

constexpr char identifier_pattern[]{ R"((?<identifier>[a-zA-Z_]\w*\b))" };
constexpr char floating_point_constant_pattern[]{
    R"((?<float_const>([0-9]*\.[0-9]+|[0-9]+\.?)[Ee][+\-]?[0-9]+|[0-9]*\.[0-9]+|[0-9]+\.)[^\w.])"
};
constexpr char int_constant_pattern[]{ R"((?<int_const>[0-9]+)[^\w.])" };
constexpr char unsigned_int_constant_pattern[]{ R"((?<uint_const>[0-9]+[uU])[^\w.])" };
constexpr char long_constant_pattern[]{ R"((?<long_const>[0-9]+[lL])[^\w.])" };
constexpr char unsigned_long_constant_pattern[]{ R"((?<ulong_const>[0-9]+([lL][uU]|[uU][lL]))[^\w.])" };
constexpr char open_parenthesis_pattern[]{ R"((\())" };
constexpr char close_parenthesis_pattern[]{ R"((\)))" };
constexpr char open_brace_pattern[]{ R"((\{))" };
constexpr char close_brace_pattern[]{ R"((\}))" };
constexpr char semicolon_pattern[]{ "(;)" };
constexpr char decrement_operator_pattern[]{ "(--)" };
constexpr char negate_operator_pattern[]{ "(-)" };
constexpr char bitwise_complement_operator_pattern[]{ "(~)" };
constexpr char plus_operator_pattern[]{ R"((\+))" };
constexpr char multiplication_operator_pattern[]{ R"((\*))" };
constexpr char division_operator_pattern[]{ "(/)" };
constexpr char remainder_operator_pattern[]{ "(%)" };
constexpr char bitwise_and_operator_pattern[]{ "(&)" };
constexpr char bitwise_or_operator_pattern[]{ R"((\|))" };
constexpr char bitwise_xor_operator_pattern[]{ R"((\^))" };
constexpr char left_shift_operator_pattern[]{ R"((<<))" };
constexpr char right_shift_operator_pattern[]{ R"((>>))" };
constexpr char not_operator_pattern[]{ R"((!))" };
constexpr char and_operator_pattern[]{ R"((&&))" };
constexpr char or_operator_pattern[]{ R"((\|\|))" };
constexpr char equals_operator_pattern[]{ R"((==))" };
constexpr char not_equals_operator_pattern[]{ R"((!=))" };
constexpr char less_than_operator_pattern[]{ R"((<))" };
constexpr char less_than_or_equal_operator_pattern[]{ R"((<=))" };
constexpr char greater_than_operator_pattern[]{ R"((>))" };
constexpr char greater_than_or_equal_operator_pattern[]{ R"((>=))" };
constexpr char assignment_operator_pattern[]{ R"((=))" };
constexpr char compound_plus_pattern[]{ R"((\+=))" };
constexpr char compound_minus_pattern[]{ R"((-=))" };
constexpr char compound_multiplication_pattern[]{ R"((\*=))" };
constexpr char compound_division_pattern[]{ R"((/=))" };
constexpr char compound_remainder_pattern[]{ R"((%=))" };
constexpr char compound_bitwise_and_pattern[]{ R"((&=))" };
constexpr char compound_bitwise_or_pattern[]{ R"((\|=))" };
constexpr char compound_bitwise_xor_pattern[]{ R"((\^=))" };
constexpr char compound_left_shift_pattern[]{ R"((<<=))" };
constexpr char compound_right_shift_pattern[]{ R"((>>=))" };
constexpr char increment_operator_pattern[]{ R"((\+\+))" };
constexpr char question_mark_pattern[]{ R"((\?))" };
constexpr char colon_pattern[]{ R"((:))" };
constexpr char comma_pattern[]{ R"((,))" };

constexpr auto get_patters_constants_and_identifier()
{
    // clang-format off
    return std::array{
        floating_point_constant_pattern,
        unsigned_long_constant_pattern,
        long_constant_pattern,
        unsigned_int_constant_pattern,
        int_constant_pattern,
    };
    // clang-format on
}

constexpr auto get_patters()
{
    return std::array{
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

constexpr std::size_t str_length(const char *p)
{
    std::size_t j = 0;
    while (p[j] != '\0')
    {
        j++;
    }
    return j;
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

consteval auto create_regex_constant_and_identifier_pattern()
{
    constexpr auto patterns = get_patters_constants_and_identifier();
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

consteval std::ptrdiff_t get_pattern_constant_and_identifier_position(const char *p)
{
    auto patterns = get_patters_constants_and_identifier();
    auto f = std::find_if(patterns.begin(), patterns.end(), [p](const char *i) { return str_compare(i, p); });

    return std::distance(patterns.begin(), f) + 1;
}

consteval std::ptrdiff_t get_pattern_position(const char *p)
{
    auto patterns = get_patters();
    auto f = std::find_if(patterns.begin(), patterns.end(), [p](const char *i) { return str_compare(i, p); });

    return std::distance(patterns.begin(), f) + 1;
}

constexpr token_type get_token_type(std::string_view text)
{
    const static std::unordered_map<std::string_view, token_type> patterns = {
        { "break", token_type::break_keyword },
        { "continue", token_type::continue_keyword },
        { "do", token_type::do_keyword },
        { "double", token_type::double_keyword },
        { "else", token_type::else_keyword },
        { "extern", token_type::extern_keyword },
        { "for", token_type::for_keyword },
        { "goto", token_type::goto_keyword },
        { "if", token_type::if_keyword },
        { "int", token_type::int_keyword },
        { "long", token_type::long_keyword },
        { "return", token_type::return_keyword },
        { "signed", token_type::signed_keyword },
        { "static", token_type::static_keyword },
        { "unsigned", token_type::unsigned_keyword },
        { "void", token_type::void_keyword },
        { "while", token_type::while_keyword },
    };

    const auto &ret = patterns.find(text);
    if (ret == patterns.end())
    {
        return token_type::identifier;
    }
    return ret->second;
}

template<const char *pattern>
consteval auto build_fixed_string()
{
    return ctll::fixed_string<str_length(pattern)>{ ctll::construct_from_pointer, pattern };
}

std::optional<std::pair<token, std::size_t>> look_for_constant_and_identifier(std::string_view input,
                                                                              const file_location &location) noexcept
{
    if (auto m = ctre::starts_with<build_fixed_string<identifier_pattern>()>(input); m)
    {
        if (auto f = m.get<"identifier">(); f)
        {
            token t(get_token_type(f), f, location);
            return std::make_pair(t, f.size());
        }
    }

    if (auto m = ctre::starts_with<create_regex_constant_and_identifier_pattern()>(input); m)
    {
        if (auto f = m.get<"float_const">(); f)
        {
            std::string_view name{ f };
            token t(token_type::floating_porint_constant, name, location);
            return std::make_pair(t, name.size());
        }

        if (auto f = m.get<"int_const">(); f)
        {
            std::string_view name{ f };
            token t(token_type::int_constant, name, location);
            return std::make_pair(t, name.size());
        }

        if (auto f = m.get<"long_const">(); f)
        {
            token t(token_type::long_constant, f, location);
            return std::make_pair(t, f.size());
        }
        if (auto f = m.get<"uint_const">(); f)
        {
            token t(token_type::unsigned_int_constant, f, location);
            return std::make_pair(t, f.size());
        }
        if (auto f = m.get<"ulong_const">(); f)
        {
            token t(token_type::unsigned_long_constant, f, location);
            return std::make_pair(t, f.size());
        }
    }

    return std::nullopt;
}

std::optional<std::pair<token, std::size_t>> look_for_rest(std::string_view input,
                                                           const file_location &location) noexcept
{
    if (auto m = ctre::starts_with<create_regex_pattern()>(input))
    {
        if (auto f = ctre::get<get_pattern_position(open_parenthesis_pattern)>(m); f)
        {
            token t(token_type::open_parenthesis, f, location);
            return std::make_pair(t, f.size());
        }

        if (auto f = ctre::get<get_pattern_position(close_parenthesis_pattern)>(m); f)
        {
            token t(token_type::close_parenthesis, f, location);
            return std::make_pair(t, f.size());
        }
        if (auto f = ctre::get<get_pattern_position(open_brace_pattern)>(m); f)
        {
            token t(token_type::open_brace, f, location);
            return std::make_pair(t, f.size());
        }
        if (auto f = ctre::get<get_pattern_position(close_brace_pattern)>(m); f)
        {
            token t(token_type::close_brace, f, location);
            return std::make_pair(t, f.size());
        }
        if (auto f = ctre::get<get_pattern_position(semicolon_pattern)>(m); f)
        {
            token t(token_type::semicolon, f, location);
            return std::make_pair(t, f.size());
        }
        if (auto f = ctre::get<get_pattern_position(and_operator_pattern)>(m); f)
        {
            token t(token_type::and_operator, f, location);
            return std::make_pair(t, f.size());
        }
        if (auto f = ctre::get<get_pattern_position(or_operator_pattern)>(m); f)
        {
            token t(token_type::or_operator, f, location);
            return std::make_pair(t, f.size());
        }
        if (auto f = ctre::get<get_pattern_position(equals_operator_pattern)>(m); f)
        {
            token t(token_type::equals_operator, f, location);
            return std::make_pair(t, f.size());
        }
        if (auto f = ctre::get<get_pattern_position(not_equals_operator_pattern)>(m); f)
        {
            token t(token_type::not_equals_operator, f, location);
            return std::make_pair(t, f.size());
        }
        if (auto f = ctre::get<get_pattern_position(decrement_operator_pattern)>(m); f)
        {
            token t(token_type::decrement_operator, f, location);
            return std::make_pair(t, f.size());
        }
        if (auto f = ctre::get<get_pattern_position(negate_operator_pattern)>(m); f)
        {
            token t(token_type::negation_operator, f, location);
            return std::make_pair(t, f.size());
        }
        if (auto f = ctre::get<get_pattern_position(not_operator_pattern)>(m); f)
        {
            token t(token_type::not_operator, f, location);
            return std::make_pair(t, f.size());
        }
        if (auto f = ctre::get<get_pattern_position(bitwise_complement_operator_pattern)>(m); f)
        {
            token t(token_type::bitwise_complement_operator, f, location);
            return std::make_pair(t, f.size());
        }
        if (auto f = ctre::get<get_pattern_position(plus_operator_pattern)>(m); f)
        {
            token t(token_type::plus_operator, f, location);
            return std::make_pair(t, f.size());
        }
        if (auto f = ctre::get<get_pattern_position(multiplication_operator_pattern)>(m); f)
        {
            token t(token_type::multiplication_operator, f, location);
            return std::make_pair(t, f.size());
        }
        if (auto f = ctre::get<get_pattern_position(division_operator_pattern)>(m); f)
        {
            token t(token_type::division_operator, f, location);
            return std::make_pair(t, f.size());
        }
        if (auto f = ctre::get<get_pattern_position(remainder_operator_pattern)>(m); f)
        {
            token t(token_type::remainder_operator, f, location);
            return std::make_pair(t, f.size());
        }
        if (auto f = ctre::get<get_pattern_position(bitwise_and_operator_pattern)>(m); f)
        {
            token t(token_type::bitwise_and_operator, f, location);
            return std::make_pair(t, f.size());
        }
        if (auto f = ctre::get<get_pattern_position(bitwise_or_operator_pattern)>(m); f)
        {
            token t(token_type::bitwise_or_operator, f, location);
            return std::make_pair(t, f.size());
        }
        if (auto f = ctre::get<get_pattern_position(bitwise_xor_operator_pattern)>(m); f)
        {
            token t(token_type::bitwise_xor_operator, f, location);
            return std::make_pair(t, f.size());
        }
        if (auto f = ctre::get<get_pattern_position(left_shift_operator_pattern)>(m); f)
        {
            token t(token_type::left_shift_operator, f, location);
            return std::make_pair(t, f.size());
        }
        if (auto f = ctre::get<get_pattern_position(right_shift_operator_pattern)>(m); f)
        {
            token t(token_type::right_shift_operator, f, location);
            return std::make_pair(t, f.size());
        }
        if (auto f = ctre::get<get_pattern_position(less_than_operator_pattern)>(m); f)
        {
            token t(token_type::less_than_operator, f, location);
            return std::make_pair(t, f.size());
        }
        if (auto f = ctre::get<get_pattern_position(less_than_or_equal_operator_pattern)>(m); f)
        {
            token t(token_type::less_than_or_equal_operator, m, location);
            return std::make_pair(t, f.size());
        }
        if (auto f = ctre::get<get_pattern_position(greater_than_operator_pattern)>(m); f)
        {
            token t(token_type::greater_than_operator, f, location);
            return std::make_pair(t, f.size());
        }
        if (auto f = ctre::get<get_pattern_position(greater_than_or_equal_operator_pattern)>(m); f)
        {
            token t(token_type::greater_than_or_equal_operator, f, location);
            return std::make_pair(t, f.size());
        }
        if (auto f = ctre::get<get_pattern_position(assignment_operator_pattern)>(m); f)
        {
            token t(token_type::assignment_operator, f, location);
            return std::make_pair(t, f.size());
        }
        if (auto f = ctre::get<get_pattern_position(compound_plus_pattern)>(m); f)
        {
            token t(token_type::compound_plus, f, location);
            return std::make_pair(t, f.size());
        }
        if (auto f = ctre::get<get_pattern_position(compound_minus_pattern)>(m); f)
        {
            token t(token_type::compound_minus, f, location);
            return std::make_pair(t, f.size());
        }
        if (auto f = ctre::get<get_pattern_position(compound_multiplication_pattern)>(m); f)
        {
            token t(token_type::compound_multiplication, f, location);
            return std::make_pair(t, f.size());
        }
        if (auto f = ctre::get<get_pattern_position(compound_division_pattern)>(m); f)
        {
            token t(token_type::compound_division, f, location);
            return std::make_pair(t, f.size());
        }
        if (auto f = ctre::get<get_pattern_position(compound_remainder_pattern)>(m); f)
        {
            token t(token_type::compound_remainder, f, location);
            return std::make_pair(t, f.size());
        }
        if (auto f = ctre::get<get_pattern_position(compound_bitwise_and_pattern)>(m); f)
        {
            token t(token_type::compound_bitwise_and, f, location);
            return std::make_pair(t, f.size());
        }
        if (auto f = ctre::get<get_pattern_position(compound_bitwise_or_pattern)>(m); f)
        {
            token t(token_type::compound_bitwise_or, f, location);
            return std::make_pair(t, f.size());
        }
        if (auto f = ctre::get<get_pattern_position(compound_bitwise_xor_pattern)>(m); f)
        {
            token t(token_type::compound_bitwise_xor, f, location);
            return std::make_pair(t, f.size());
        }
        if (auto f = ctre::get<get_pattern_position(compound_left_shift_pattern)>(m); f)
        {
            token t(token_type::compound_left_shift, f, location);
            return std::make_pair(t, f.size());
        }
        if (auto f = ctre::get<get_pattern_position(compound_right_shift_pattern)>(m); f)
        {
            token t(token_type::compound_right_shift, f, location);
            return std::make_pair(t, f.size());
        }
        if (auto f = ctre::get<get_pattern_position(increment_operator_pattern)>(m); f)
        {
            token t(token_type::increment_operator, f, location);
            return std::make_pair(t, f.size());
        }
        if (auto f = ctre::get<get_pattern_position(question_mark_pattern)>(m); f)
        {
            token t(token_type::question_mark, f, location);
            return std::make_pair(t, f.size());
        }
        if (auto f = ctre::get<get_pattern_position(colon_pattern)>(m); f)
        {
            token t(token_type::colon, f, location);
            return std::make_pair(t, f.size());
        }
        if (auto f = ctre::get<get_pattern_position(comma_pattern)>(m); f)
        {
            token t(token_type::comma, f, location);
            return std::make_pair(t, f.size());
        }
    }
    return std::nullopt;
}

std::expected<std::vector<token>, lexer_error> lexer(std::string_view input) noexcept
{
    std::vector<token> result;
    file_location location;

    while (true)
    {
        // Remove trimming white spaces
        while ((input.empty() == false) && std::isspace(input[0]))
        {
            location.column++;
            if (input[0] == '\n')
            {
                location.line++;
                location.column = 1;
            }
            input = input.substr(1);
        }

        if (input.empty())
        {
            return result;
        }

        auto f = [&input, &location]() { return look_for_rest(input, location); };
        auto identifier = look_for_constant_and_identifier(input, location).or_else(f);
        if (identifier.has_value())
        {
            result.push_back(identifier.value().first);
            location.column += identifier.value().second;

            input = input.substr(identifier.value().second);
            continue;
        }

        return std::unexpected(lexer_error{ location, input, "Unhandled match" });
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
