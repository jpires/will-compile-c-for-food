#include "../parser.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Parser", "[parser]")
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

    SECTION("Parse Constant")
    {
        wccff::lexer::file_location location{ 1, 3 };
        std::vector<wccff::lexer::token> tokens_vector;
        tokens_vector.emplace_back(wccff::lexer::token_type::constant, "123", location);

        wccff::parser::tokens tokens{ tokens_vector };

        auto r = wccff::parser::parse_constant(tokens);

        REQUIRE(r.has_value());
        REQUIRE(r->value == 123);
    }

    SECTION("Parse Negate Operator")
    {
        wccff::lexer::file_location location{ 1, 3 };
        std::vector<wccff::lexer::token> tokens_vector;
        tokens_vector.emplace_back(wccff::lexer::token_type::negation_operator, "-", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::constant, "2", location);

        wccff::parser::tokens tokens{ tokens_vector };

        auto r = wccff::parser::parse_unary_node(tokens);

        REQUIRE(r.has_value());
        REQUIRE(std::holds_alternative<wccff::parser::negate_operator>(r.value()->op) == true);
        REQUIRE(std::holds_alternative<wccff::parser::int_constant>(r.value()->exp) == true);
        REQUIRE(std::get<wccff::parser::int_constant>(r.value()->exp).value == 2);
    }

    SECTION("Parse Bitwise Complement Operator")
    {
        wccff::lexer::file_location location{ 1, 3 };
        std::vector<wccff::lexer::token> tokens_vector;
        tokens_vector.emplace_back(wccff::lexer::token_type::bitwise_complement_operator, "~", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::constant, "2", location);

        wccff::parser::tokens tokens{ tokens_vector };

        auto r = wccff::parser::parse_unary_node(tokens);

        REQUIRE(r.has_value());
        REQUIRE(std::holds_alternative<wccff::parser::bitwise_complement_operator>(r.value()->op) == true);
        REQUIRE(std::holds_alternative<wccff::parser::int_constant>(r.value()->exp) == true);
        REQUIRE(std::get<wccff::parser::int_constant>(r.value()->exp).value == 2);
    }

    SECTION("Parse two Unary Operator in a row")
    {
        SECTION("Two bitwise complement operators")
        {
            wccff::lexer::file_location location{ 1, 3 };
            std::vector<wccff::lexer::token> tokens_vector;
            tokens_vector.emplace_back(wccff::lexer::token_type::bitwise_complement_operator, "~", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::bitwise_complement_operator, "~", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::constant, "2", location);

            wccff::parser::tokens tokens{ tokens_vector };

            auto r = wccff::parser::parse_unary_node(tokens);

            REQUIRE(r.has_value());
            REQUIRE(std::holds_alternative<wccff::parser::bitwise_complement_operator>(r.value()->op) == true);
            REQUIRE(std::holds_alternative<std::unique_ptr<wccff::parser::unary_node>>(r.value()->exp) == true);

            auto inner_expression = std::move(std::get<std::unique_ptr<wccff::parser::unary_node>>(r.value()->exp));
            REQUIRE(std::holds_alternative<wccff::parser::bitwise_complement_operator>(inner_expression->op) == true);
            REQUIRE(std::get<wccff::parser::int_constant>(inner_expression->exp).value == 2);
        }

        SECTION("Negate and bitwise complement operators")
        {
            wccff::lexer::file_location location{ 1, 3 };
            std::vector<wccff::lexer::token> tokens_vector;
            tokens_vector.emplace_back(wccff::lexer::token_type::negation_operator, "-", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::bitwise_complement_operator, "~", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::constant, "2", location);

            wccff::parser::tokens tokens{ tokens_vector };

            auto r = wccff::parser::parse_unary_node(tokens);

            REQUIRE(r.has_value());
            REQUIRE(std::holds_alternative<wccff::parser::negate_operator>(r.value()->op) == true);
            REQUIRE(std::holds_alternative<std::unique_ptr<wccff::parser::unary_node>>(r.value()->exp) == true);

            auto inner_expression = std::move(std::get<std::unique_ptr<wccff::parser::unary_node>>(r.value()->exp));
            REQUIRE(std::holds_alternative<wccff::parser::bitwise_complement_operator>(inner_expression->op) == true);
            REQUIRE(std::get<wccff::parser::int_constant>(inner_expression->exp).value == 2);
        }
    }
}

TEST_CASE("Parser complex test", "[parser]")
{
    SECTION("Main function returns 2")
    {
        wccff::lexer::file_location location{ 1, 2 };
        std::vector<wccff::lexer::token> tokens_vector;
        tokens_vector.emplace_back(wccff::lexer::token_type::int_keyword, "int", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "main", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::open_parenthesis, "(", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::void_keyword, "void", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::close_parenthesis, ")", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::open_brace, "{", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::return_keyword, "return", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::constant, "2", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::semicolon, ";", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::close_brace, "}", location);

        wccff::parser::tokens tokens{ tokens_vector };

        auto r = wccff::parser::parse(tokens);
        REQUIRE(r.has_value());
        REQUIRE(r->f.function_name.name == "main");
        REQUIRE(std::holds_alternative<wccff::parser::return_node>(r->f.body));
        auto ret_node = std::move(std::get<wccff::parser::return_node>(r->f.body));
        REQUIRE(std::holds_alternative<wccff::parser::int_constant>(ret_node.e));
        REQUIRE(std::get<wccff::parser::int_constant>(ret_node.e).value == 2);
    }
}
