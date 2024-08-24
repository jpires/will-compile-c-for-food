#include "lexer.h"
#include <ctre.hpp>
#include <fstream>
#include <iostream>

namespace wccff {
/*constexpr auto get_pattern()
{
    static constexpr auto identifier_pattern = R"(([a-zA-Z_]\w*\b))";
    static constexpr auto constant_pattern =  R"(([0-9]+\b)";
    static constexpr auto int_pattern = R"(int)";
    static constexpr auto void_pattern = R"(void)" ;
    static constexpr auto return_pattern = R"(return)" ;
    static constexpr auto open_parenthesis_pattern =  R"(\()";
    static constexpr auto close_parenthesis_pattern =  R"(\))";
    static constexpr auto open_brace_pattern =  R"({)" ;
    static constexpr auto close_brace_pattern = R"(})" ;
    static constexpr auto semicolon_pattern =  R"(;)" ;

    //static constexpr std::string c;
    //c += identifier_pattern;

    static constexpr std::array b{identifier_pattern};
    static constexpr auto pattern = ctll::fixed_string{ b };


    return ctll::fixed_string("12");
}*/

std::expected<std::vector<token>, std::error_code> lexer(std::string_view input) noexcept
{
    static constexpr auto pattern = ctll::fixed_string{ R"(([a-zA-Z_]\w*\b)|([0-9]+\b)|(\()|(\))|(\{)|(\})|(;))" };

    if (input.empty())
    {
        return {};
    }
    // Remove trimming white spaces
    while (std::isspace(input[0]))
    {
        input = input.substr(1);
    }

    std::cout << "Input: " << input << '\n';
    auto m = ctre::starts_with<pattern>(input);
    if (m)
    {
        std::vector<token> result;
        if (ctre::get<1>(m))
        {
            if (m == "int")
            {
                std::cout << "int keyword" << '\n';
                result.emplace_back(token_type::int_keyword, m);
            }
            else if (m == "void")
            {
                std::cout << "void keyword" << '\n';
                result.emplace_back(token_type::void_keyword, m);
            }
            else if (m == "return")
            {
                std::cout << "return keyword" << '\n';
                result.emplace_back(token_type::return_keyword, m);
            }
            else
            {
                std::cout << "Found identifier" << '\n';
                result.emplace_back(token_type::identifier, m);
            }
        }

        if (ctre::get<2>(m))
        {
            std::cout << "Found Constan" << '\n';
            result.emplace_back(token_type::constant, m);
        }

        if (ctre::get<3>(m))
        {
            std::cout << "Found Open Parenthesis" << '\n';
            result.emplace_back(token_type::open_parenthesis, m);
        }

        if (ctre::get<4>(m))
        {
            std::cout << "Found Close Parenthesis" << '\n';
            result.emplace_back(token_type::close_parenthesis, m);
        }
        if (ctre::get<5>(m))
        {
            std::cout << "Found Open Brace" << '\n';
            result.emplace_back(token_type::open_brace, m);
        }
        if (ctre::get<6>(m))
        {
            std::cout << "Found Close Brace" << '\n';
            result.emplace_back(token_type::close_brace, m);
        }
        if (ctre::get<7>(m))
        {
            std::cout << "Found Semicolon" << '\n';
            result.emplace_back(token_type::semicolon, m);
        }

        if (result.empty())
        {
            std::cout << "No match for: " << m << '\n';
            return std::unexpected(std::make_error_code(std::errc::address_family_not_supported));
        }

        auto tail = lexer({ input.substr(m.size()) });
        if (tail.has_value() == false)
        {
            return std::unexpected(tail.error());
        }

        result.append_range(tail.value());
        return result;
    }

    if (input.empty() == false)
    {
        return std::unexpected(std::make_error_code(std::errc::address_family_not_supported));
    }

    return {};
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
} // namespace wccff
