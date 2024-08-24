#include "../compiler.h"
#include "../lexer.h"

#include <catch2/catch_test_macros.hpp>
#include <string_view>

TEST_CASE("Identifiers", "[lexer]")
{
    SECTION("Only Chars")
    {
        SECTION("One char long")
        {
            std::string_view input{ "a" };
            auto result = wccff::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().size() == 1);
            REQUIRE(result.value().at(0).t == wccff::token_type::identifier);
            REQUIRE(result.value().at(0).c == "a");
        }

        SECTION("Five chars long")
        {
            std::string_view input{ "abcde" };
            auto result = wccff::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().size() == 1);
            REQUIRE(result.value().at(0).t == wccff::token_type::identifier);
            REQUIRE(result.value().at(0).c == "abcde");
        }
    }

    SECTION("Mix Chars and Digits")
    {
        SECTION("One char and One digit")
        {
            std::string_view input{ "a1" };
            auto result = wccff::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().size() == 1);
            REQUIRE(result.value().at(0).t == wccff::token_type::identifier);
            REQUIRE(result.value().at(0).c == "a1");
        }

        SECTION("Five chars long and two digits")
        {
            std::string_view input{ "abcde45" };
            auto result = wccff::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().size() == 1);
            REQUIRE(result.value().at(0).t == wccff::token_type::identifier);
            REQUIRE(result.value().at(0).c == "abcde45");
        }
    }
}

TEST_CASE("Keywords", "[lexer]")
{
    SECTION("void")
    {
        std::string_view input{ "void" };
        auto result = wccff::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).t == wccff::token_type::void_keyword);
        REQUIRE(result.value().at(0).c == "void");
    }

    SECTION("int")
    {
        std::string_view input{ "int" };
        auto result = wccff::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).t == wccff::token_type::int_keyword);
        REQUIRE(result.value().at(0).c == "int");
    }

    SECTION("return")
    {
        std::string_view input{ "return" };
        auto result = wccff::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).t == wccff::token_type::return_keyword);
        REQUIRE(result.value().at(0).c == "return");
    }
}
TEST_CASE("Constants", "[lexer]")
{
    SECTION("Zero length")
    {
        std::string_view input;
        auto result = wccff::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().empty());
    }

    SECTION("One digit")
    {
        std::string_view input{ "1" };
        auto result = wccff::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).t == wccff::token_type::constant);
        REQUIRE(result.value().at(0).c == "1");
    }

    SECTION("four digit")
    {
        std::string_view input{ "1234" };
        auto result = wccff::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).t == wccff::token_type::constant);
        REQUIRE(result.value().at(0).c == "1234");
    }

    SECTION("four digit Followed by semicolon")
    {
        std::string_view input{ "1234;" };
        auto result = wccff::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 2);
        REQUIRE(result.value().at(0).t == wccff::token_type::constant);
        REQUIRE(result.value().at(0).c == "1234");
        REQUIRE(result.value().at(1).t == wccff::token_type::semicolon);
        REQUIRE(result.value().at(1).c == ";");
    }

    SECTION("four digit Followed by close parenthesis")
    {
        std::string_view input{ "1234)" };
        auto result = wccff::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 2);
        REQUIRE(result.value().at(0).t == wccff::token_type::constant);
        REQUIRE(result.value().at(0).c == "1234");
        REQUIRE(result.value().at(1).t == wccff::token_type::close_parenthesis);
        REQUIRE(result.value().at(1).c == ")");
    }
}
