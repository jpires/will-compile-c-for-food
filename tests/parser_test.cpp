#include "../parser.h"

#include <__ranges/to.h>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Identifiers", "[parser]")
{
    SECTION("Parse Valid identifier")
    {
        wccff::lexer::file_location location{ 1, 2 };
        std::vector<wccff::lexer::token> tokens_vector;
        tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "main", location);

        wccff::parser::tokens tokens{ tokens_vector };

        auto r = wccff::parser::parse_identifier(tokens);

        REQUIRE(r.has_value());
        REQUIRE(r.value().name == "main");
    }

    SECTION("Parse Valid identifier")
    {
        wccff::lexer::file_location location{ 1, 3 };
        std::vector<wccff::lexer::token> tokens_vector;
        tokens_vector.emplace_back(wccff::lexer::token_type::negation_operator, "-", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::constant, "2", location);

        wccff::parser::tokens tokens{ tokens_vector };

        auto r = wccff::parser::parse_unary_node(tokens);

        REQUIRE(r.has_value());
        REQUIRE(std::holds_alternative<wccff::parser::negate_operator>(r.value()->op) == true);
    }

    SECTION("Parse Valid identifier")
    {
        wccff::lexer::file_location location{ 1, 3 };
        std::vector<wccff::lexer::token> tokens_vector;
        tokens_vector.emplace_back(wccff::lexer::token_type::bitwise_complement_operator, "~", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::constant, "2", location);

        wccff::parser::tokens tokens{ tokens_vector };

        auto r = wccff::parser::parse_unary_node(tokens);

        REQUIRE(r.has_value());
        REQUIRE(std::holds_alternative<wccff::parser::bitwise_complement_operator>(r.value()->op) == true);
    }
}
