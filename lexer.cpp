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

constexpr auto get_patters()
{
    return std::array{ identifier_pattern,          constant_pattern,
                       open_parenthesis_pattern,    close_parenthesis_pattern,
                       open_brace_pattern,          close_brace_pattern,
                       semicolon_pattern,           decrement_operator_pattern,
                       negate_operator_pattern,     bitwise_complement_operator_pattern,
                       plus_operator_pattern,       multiplication_operator_pattern,
                       division_operator_pattern,   remainder_operator_pattern,
                       bitwise_and_operator_pattern };
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

consteval int get_identifier_position()
{
    auto patterns = get_patters();
    auto f = std::find_if(patterns.begin(), patterns.end(), [](const char *i) {
        return str_compare(i, identifier_pattern);
    });
    return std::distance(patterns.begin(), f) + 1;
}
consteval int get_constant_position()
{
    auto patterns = get_patters();
    auto f = std::find_if(patterns.begin(), patterns.end(), [](const char *i) {
        return str_compare(i, constant_pattern);
    });

    return std::distance(patterns.begin(), f) + 1;
}
consteval int get_open_parenthesis_position()
{
    auto patterns = get_patters();
    auto f = std::find_if(patterns.begin(), patterns.end(), [](const char *i) {
        return str_compare(i, open_parenthesis_pattern);
    });

    return std::distance(patterns.begin(), f) + 1;
}
consteval int get_close_parenthesis_position()
{
    auto patterns = get_patters();
    auto f = std::find_if(patterns.begin(), patterns.end(), [](const char *i) {
        return str_compare(i, close_parenthesis_pattern);
    });

    return std::distance(patterns.begin(), f) + 1;
}
consteval int get_open_braces_position()
{
    auto patterns = get_patters();
    auto f = std::find_if(patterns.begin(), patterns.end(), [](const char *i) {
        return str_compare(i, open_brace_pattern);
    });

    return std::distance(patterns.begin(), f) + 1;
}
consteval int get_close_braces_position()
{
    auto patterns = get_patters();
    auto f = std::find_if(patterns.begin(), patterns.end(), [](const char *i) {
        return str_compare(i, close_brace_pattern);
    });

    return std::distance(patterns.begin(), f) + 1;
}
consteval int get_semicolon_position()
{
    auto patterns = get_patters();
    auto f = std::find_if(patterns.begin(), patterns.end(), [](const char *i) {
        return str_compare(i, semicolon_pattern);
    });

    return std::distance(patterns.begin(), f) + 1;
}
consteval int get_decrement_operator_position()
{
    auto patterns = get_patters();
    auto f = std::find_if(patterns.begin(), patterns.end(), [](const char *i) {
        return str_compare(i, decrement_operator_pattern);
    });

    return std::distance(patterns.begin(), f) + 1;
}
consteval int get_negate_operator_position()
{
    auto patterns = get_patters();
    auto f = std::find_if(patterns.begin(), patterns.end(), [](const char *i) {
        return str_compare(i, negate_operator_pattern);
    });

    return std::distance(patterns.begin(), f) + 1;
}
consteval int get_bitwise_complement_operator_position()
{
    auto patterns = get_patters();
    auto f = std::find_if(patterns.begin(), patterns.end(), [](const char *i) {
        return str_compare(i, bitwise_complement_operator_pattern);
    });

    return std::distance(patterns.begin(), f) + 1;
}
consteval int get_plus_operator_position()
{
    auto patterns = get_patters();
    auto f = std::find_if(patterns.begin(), patterns.end(), [](const char *i) {
        return str_compare(i, plus_operator_pattern);
    });

    return std::distance(patterns.begin(), f) + 1;
}
consteval int get_multiplication_operator_position()
{
    auto patterns = get_patters();
    auto f = std::find_if(patterns.begin(), patterns.end(), [](const char *i) {
        return str_compare(i, multiplication_operator_pattern);
    });

    return std::distance(patterns.begin(), f) + 1;
}
consteval int get_division_operator_position()
{
    auto patterns = get_patters();
    auto f = std::find_if(patterns.begin(), patterns.end(), [](const char *i) {
        return str_compare(i, division_operator_pattern);
    });

    return std::distance(patterns.begin(), f) + 1;
}
consteval int get_remainder_operator_position()
{
    auto patterns = get_patters();
    auto f = std::find_if(patterns.begin(), patterns.end(), [](const char *i) {
        return str_compare(i, remainder_operator_pattern);
    });

    return std::distance(patterns.begin(), f) + 1;
}
consteval int get_bitwise_and_operator_position()
{
    auto patterns = get_patters();
    auto f = std::find_if(patterns.begin(), patterns.end(), [](const char *i) {
        return str_compare(i, bitwise_and_operator_pattern);
    });

    return std::distance(patterns.begin(), f) + 1;
}

std::expected<std::vector<token>, lexer_error> lexer(std::string_view input, file_location location) noexcept
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
        return {};
    }

    std::cout << "Input: " << input << '\n';
    auto m = ctre::starts_with<create_regex_pattern()>(input);
    if (m)
    {
        std::vector<token> result;
        if (ctre::get<get_identifier_position()>(m))
        {
            if (m == "int")
            {
                std::cout << "int keyword" << '\n';
                result.emplace_back(token_type::int_keyword, m, location);
            }
            else if (m == "void")
            {
                std::cout << "void keyword" << '\n';
                result.emplace_back(token_type::void_keyword, m, location);
            }
            else if (m == "return")
            {
                std::cout << "return keyword" << '\n';
                result.emplace_back(token_type::return_keyword, m, location);
            }
            else
            {
                std::cout << "Found identifier" << '\n';
                result.emplace_back(token_type::identifier, m, location);
            }
        }

        if (ctre::get<get_constant_position()>(m))
        {
            std::cout << "Found Constan" << '\n';
            result.emplace_back(token_type::constant, m, location);
        }

        if (ctre::get<get_open_parenthesis_position()>(m))
        {
            std::cout << "Found Open Parenthesis" << '\n';
            result.emplace_back(token_type::open_parenthesis, m, location);
        }

        if (ctre::get<get_close_parenthesis_position()>(m))
        {
            std::cout << "Found Close Parenthesis" << '\n';
            result.emplace_back(token_type::close_parenthesis, m, location);
        }
        if (ctre::get<get_open_braces_position()>(m))
        {
            std::cout << "Found Open Brace" << '\n';
            result.emplace_back(token_type::open_brace, m, location);
        }
        if (ctre::get<get_close_braces_position()>(m))
        {
            std::cout << "Found Close Brace" << '\n';
            result.emplace_back(token_type::close_brace, m, location);
        }
        if (ctre::get<get_semicolon_position()>(m))
        {
            std::cout << "Found Semicolon" << '\n';
            result.emplace_back(token_type::semicolon, m, location);
        }
        if (ctre::get<get_decrement_operator_position()>(m))
        {
            std::cout << "Found decrement Operator" << '\n';
            result.emplace_back(token_type::decrement_operator, m, location);
        }
        if (ctre::get<get_negate_operator_position()>(m))
        {
            std::cout << "Found Negation Operator" << '\n';
            result.emplace_back(token_type::negation_operator, m, location);
        }
        if (ctre::get<get_bitwise_complement_operator_position()>(m))
        {
            std::cout << "Found Bitwise Complement Operator" << '\n';
            result.emplace_back(token_type::bitwise_complement_operator, m, location);
        }
        if (ctre::get<get_plus_operator_position()>(m))
        {
            std::cout << "Found Plus Operator" << '\n';
            result.emplace_back(token_type::plus_operator, m, location);
        }
        if (ctre::get<get_multiplication_operator_position()>(m))
        {
            std::cout << "Found Multiplication Operator" << '\n';
            result.emplace_back(token_type::multiplication_operator, m, location);
        }
        if (ctre::get<get_division_operator_position()>(m))
        {
            std::cout << "Found Division Operator" << '\n';
            result.emplace_back(token_type::division_operator, m, location);
        }
        if (ctre::get<get_remainder_operator_position()>(m))
        {
            std::cout << "Found Remainder Operator" << '\n';
            result.emplace_back(token_type::remainder_operator, m, location);
        }
        if (ctre::get<get_bitwise_and_operator_position()>(m))
        {
            std::cout << "Found Bitwise And Operator" << '\n';
            result.emplace_back(token_type::bitwise_and_operator, m, location);
        }

        if (result.empty())
        {
            return std::unexpected(lexer_error{ location, m, "Unhandled match" });
        }

        location.column += m.size();

        auto tail = lexer({ input.substr(m.size()) }, location);
        if (tail.has_value() == false)
        {
            return std::unexpected(tail.error());
        }

        result.append_range(tail.value());
        return result;
    }

    return std::unexpected(lexer_error{ location, input, "Failed to find a match" });
}

std::expected<std::string, std::error_code> read_file(const std::filesystem::path &file_name)
{
    std::cout << file_name << '\n';

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
