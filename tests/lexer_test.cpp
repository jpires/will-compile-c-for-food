#include "../compiler.h"
#include "../lexer.h"

#include <catch2/catch_test_macros.hpp>
#include <string_view>

TEST_CASE("Lexer", "[lexer]")
{
    using wccff::lexer::file_location;
    using wccff::lexer::token_type;

    SECTION("No tokens")
    {
        SECTION("nullptr input")
        {
            // This string_view doesn't have any memory associated.
            // Any attempt to read data will crash
            std::string_view input;
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().empty());
        }
        SECTION("Empty input")
        {
            // This string_view has a string that only has a NULL char
            // Even though the length is zero, we can still read input[0] without
            // an out-of-bounds read.
            std::string_view input{ "" };
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().empty());
        }

        SECTION("Only contains spaces")
        {
            std::string_view input{ "      " };
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().empty());
        }
        SECTION("Only contains newlines")
        {
            std::string_view input{ "\n\n\n" };
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().empty());
        }
        SECTION("Only contains newlines and spaces")
        {
            std::string_view input{ "   \n\n  \n   " };
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().empty());
        }
    }

    SECTION("Location")
    {
        SECTION("Token at the begin of the input")
        {
            file_location loc{ 0, 0 };
            std::string_view input{ "a" };
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().size() == 1);
            REQUIRE(result.value().at(0).loc == loc);
        }
        SECTION("With spaces at before the token")
        {
            file_location loc{ 0, 3 };
            std::string_view input{ "   a" };
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().size() == 1);
            REQUIRE(result.value().at(0).loc == loc);
        }
        SECTION("With empty lines before the token")
        {
            file_location loc{ 3, 0 };
            std::string_view input{ "\n\n\na" };
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().size() == 1);
            REQUIRE(result.value().at(0).loc == loc);
        }
        SECTION("With empty lines and spaces before the token")
        {
            file_location loc{ 3, 4 };
            std::string_view input{ "\n\n\n    a" };
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().size() == 1);
            REQUIRE(result.value().at(0).loc == loc);
        }
        SECTION("Multiple tokens")
        {
            file_location loc1{ 3, 4 };
            file_location loc2{ 5, 2 };
            std::string_view input{ "\n\n\n    a\n\n  fo" };
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().size() == 2);
            REQUIRE(result.value().at(0).loc == loc1);
            REQUIRE(result.value().at(1).loc == loc2);
        }
    }

    SECTION("Multiple tokens")
    {
        SECTION("Separated with spaces")
        {
            std::string_view input{ "a 11" };
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().size() == 2);
            REQUIRE(result.value().at(0).type == token_type::identifier);
            REQUIRE(result.value().at(0).text == "a");
            REQUIRE(result.value().at(1).type == token_type::constant);
            REQUIRE(result.value().at(1).text == "11");
        }

        SECTION("Separated by newlines")
        {
            std::string_view input{ "a\n\n11" };
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().size() == 2);
            REQUIRE(result.value().at(0).type == token_type::identifier);
            REQUIRE(result.value().at(0).text == "a");
            REQUIRE(result.value().at(1).type == token_type::constant);
            REQUIRE(result.value().at(1).text == "11");
        }
    }
}

TEST_CASE("Identifiers", "[lexer]")
{
    SECTION("Only Chars")
    {
        SECTION("One char long")
        {
            std::string_view input{ "a" };
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().size() == 1);
            REQUIRE(result.value().at(0).type == wccff::lexer::token_type::identifier);
            REQUIRE(result.value().at(0).text == "a");
        }

        SECTION("Five chars long")
        {
            std::string_view input{ "abcde" };
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().size() == 1);
            REQUIRE(result.value().at(0).type == wccff::lexer::token_type::identifier);
            REQUIRE(result.value().at(0).text == "abcde");
        }
    }

    SECTION("Mix Chars and Digits")
    {
        SECTION("One char and One digit")
        {
            std::string_view input{ "a1" };
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().size() == 1);
            REQUIRE(result.value().at(0).type == wccff::lexer::token_type::identifier);
            REQUIRE(result.value().at(0).text == "a1");
        }

        SECTION("Five chars long and two digits")
        {
            std::string_view input{ "abcde45" };
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().size() == 1);
            REQUIRE(result.value().at(0).type == wccff::lexer::token_type::identifier);
            REQUIRE(result.value().at(0).text == "abcde45");
        }
    }
}

TEST_CASE("Keywords", "[lexer]")
{
    SECTION("void")
    {
        std::string_view input{ "void" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::void_keyword);
        REQUIRE(result.value().at(0).text == "void");
    }

    SECTION("int")
    {
        std::string_view input{ "int" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::int_keyword);
        REQUIRE(result.value().at(0).text == "int");
    }

    SECTION("return")
    {
        std::string_view input{ "return" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::return_keyword);
        REQUIRE(result.value().at(0).text == "return");
    }
}

TEST_CASE("Constants", "[lexer]")
{
    SECTION("One digit")
    {
        std::string_view input{ "1" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::constant);
        REQUIRE(result.value().at(0).text == "1");
    }

    SECTION("Four digits")
    {
        std::string_view input{ "1234" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::constant);
        REQUIRE(result.value().at(0).text == "1234");
    }
}

TEST_CASE("Operators", "[lexer]")
{
    SECTION("Negation Operator")
    {
        std::string_view input{ "-" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::negation_operator);
        REQUIRE(result.value().at(0).text == "-");
    }

    SECTION("Decrement Operator")
    {
        std::string_view input{ "--" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::decrement_operator);
        REQUIRE(result.value().at(0).text == "--");
    }

    SECTION("Bitwise Complement Operator")
    {
        std::string_view input{ "~" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::bitwise_complement_operator);
        REQUIRE(result.value().at(0).text == "~");
    }

    SECTION("Plus Operator")
    {
        std::string_view input{ "+" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::plus_operator);
        REQUIRE(result.value().at(0).text == "+");
    }

    SECTION("Multiplication Operator")
    {
        std::string_view input{ "*" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::multiplication_operator);
        REQUIRE(result.value().at(0).text == "*");
    }

    SECTION("Division Operator")
    {
        std::string_view input{ "/" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::division_operator);
        REQUIRE(result.value().at(0).text == "/");
    }

    SECTION("Remainder Operator")
    {
        std::string_view input{ "%" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::remainder_operator);
        REQUIRE(result.value().at(0).text == "%");
    }
    SECTION("Bitwise And Operator")
    {
        std::string_view input{ "&" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::bitwise_and_operator);
        REQUIRE(result.value().at(0).text == "&");
    }
    SECTION("Bitwise Or Operator")
    {
        std::string_view input{ "|" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::bitwise_or_operator);
        REQUIRE(result.value().at(0).text == "|");
    }
    SECTION("Bitwise Xor Operator")
    {
        std::string_view input{ "^" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::bitwise_xor_operator);
        REQUIRE(result.value().at(0).text == "^");
    }
    SECTION("Left Shift Operator")
    {
        std::string_view input{ "<<" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::left_shift_operator);
        REQUIRE(result.value().at(0).text == "<<");
    }
    SECTION("Right Shift Operator")
    {
        std::string_view input{ ">>" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::right_shift_operator);
        REQUIRE(result.value().at(0).text == ">>");
    }

    SECTION("Not Operator")
    {
        std::string_view input{ "!" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::not_operator);
        REQUIRE(result.value().at(0).text == "!");
    }
    SECTION("And Operator")
    {
        std::string_view input{ "&&" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::and_operator);
        REQUIRE(result.value().at(0).text == "&&");
    }
    SECTION("Or Operator")
    {
        std::string_view input{ "||" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::or_operator);
        REQUIRE(result.value().at(0).text == "||");
    }
    SECTION("Equals Operator")
    {
        std::string_view input{ "==" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::equals_operator);
        REQUIRE(result.value().at(0).text == "==");
    }
    SECTION("Not Equals Operator")
    {
        std::string_view input{ "!=" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::not_equals_operator);
        REQUIRE(result.value().at(0).text == "!=");
    }
    SECTION("Less Than Operator")
    {
        std::string_view input{ "<" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::less_than_operator);
        REQUIRE(result.value().at(0).text == "<");
    }
    SECTION("Less Than or Equal Operator")
    {
        std::string_view input{ "<=" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::less_than_or_equal_operator);
        REQUIRE(result.value().at(0).text == "<=");
    }
    SECTION("Greater Than Operator")
    {
        std::string_view input{ ">" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::greater_than_operator);
        REQUIRE(result.value().at(0).text == ">");
    }
    SECTION("Greater Than or Equal Operator")
    {
        std::string_view input{ ">=" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::greater_than_or_equal_operator);
        REQUIRE(result.value().at(0).text == ">=");
    }
}

TEST_CASE("Other tokens", "[lexer]")
{
    SECTION("Semicolon")
    {
        std::string_view input{ ";" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::semicolon);
        REQUIRE(result.value().at(0).text == ";");
    }

    SECTION("Open Brace")
    {
        std::string_view input{ "{" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::open_brace);
        REQUIRE(result.value().at(0).text == "{");
    }

    SECTION("Close Brace")
    {
        std::string_view input{ "}" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::close_brace);
        REQUIRE(result.value().at(0).text == "}");
    }

    SECTION("Open Parenthesis")
    {
        std::string_view input{ "(" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::open_parenthesis);
        REQUIRE(result.value().at(0).text == "(");
    }

    SECTION("Close Parenthesis")
    {
        std::string_view input{ ")" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::close_parenthesis);
        REQUIRE(result.value().at(0).text == ")");
    }
}
