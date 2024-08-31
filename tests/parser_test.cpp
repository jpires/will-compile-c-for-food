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

    SECTION("Binary Operators")
    {
        SECTION("Plus operator")
        {
            wccff::lexer::file_location location{ 0, 0 };
            std::vector<wccff::lexer::token> tokens_vector;
            tokens_vector.emplace_back(wccff::lexer::token_type::constant, "1", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::plus_operator, "~", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::constant, "2", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::semicolon, ";", location);
            wccff::parser::tokens tokens{ tokens_vector };

            auto r = wccff::parser::parse_expression(tokens);
            REQUIRE(r.has_value());
            REQUIRE(std::holds_alternative<std::unique_ptr<wccff::parser::binary_node>>(r.value()) == true);
            auto &exp = std::get<std::unique_ptr<wccff::parser::binary_node>>(r.value());

            REQUIRE(std::holds_alternative<wccff::parser::plus_operator>(exp->op));
            REQUIRE(std::holds_alternative<wccff::parser::int_constant>(exp->left));
            auto &left = std::get<wccff::parser::int_constant>(exp->left);
            REQUIRE(left.value == 1);

            REQUIRE(std::holds_alternative<wccff::parser::int_constant>(exp->right));
            auto &right = std::get<wccff::parser::int_constant>(exp->right);
            REQUIRE(right.value == 2);
        }

        SECTION("Subtraction operator")
        {
            wccff::lexer::file_location location{ 0, 0 };
            std::vector<wccff::lexer::token> tokens_vector;
            tokens_vector.emplace_back(wccff::lexer::token_type::constant, "2", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::negation_operator, "-", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::constant, "1", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::semicolon, ";", location);
            wccff::parser::tokens tokens{ tokens_vector };

            auto r = wccff::parser::parse_expression(tokens);
            REQUIRE(r.has_value());
            REQUIRE(std::holds_alternative<std::unique_ptr<wccff::parser::binary_node>>(r.value()) == true);
            auto &exp = std::get<std::unique_ptr<wccff::parser::binary_node>>(r.value());

            REQUIRE(std::holds_alternative<wccff::parser::subtract_operator>(exp->op));
            REQUIRE(std::holds_alternative<wccff::parser::int_constant>(exp->left));
            auto &left = std::get<wccff::parser::int_constant>(exp->left);
            REQUIRE(left.value == 2);

            REQUIRE(std::holds_alternative<wccff::parser::int_constant>(exp->right));
            auto &right = std::get<wccff::parser::int_constant>(exp->right);
            REQUIRE(right.value == 1);
        }

        SECTION("Multiplication operator")
        {
            wccff::lexer::file_location location{ 0, 0 };
            std::vector<wccff::lexer::token> tokens_vector;
            tokens_vector.emplace_back(wccff::lexer::token_type::constant, "2", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::multiplication_operator, "*", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::constant, "1", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::semicolon, ";", location);
            wccff::parser::tokens tokens{ tokens_vector };

            auto r = wccff::parser::parse_expression(tokens);
            REQUIRE(r.has_value());
            REQUIRE(std::holds_alternative<std::unique_ptr<wccff::parser::binary_node>>(r.value()) == true);
            auto &exp = std::get<std::unique_ptr<wccff::parser::binary_node>>(r.value());

            REQUIRE(std::holds_alternative<wccff::parser::multiply_operator>(exp->op));
            REQUIRE(std::holds_alternative<wccff::parser::int_constant>(exp->left));
            auto &left = std::get<wccff::parser::int_constant>(exp->left);
            REQUIRE(left.value == 2);

            REQUIRE(std::holds_alternative<wccff::parser::int_constant>(exp->right));
            auto &right = std::get<wccff::parser::int_constant>(exp->right);
            REQUIRE(right.value == 1);
        }

        SECTION("1 + 3 - 5")
        {
            // This generates the following tree
            //          -
            //         / \
            //        +   5
            //       / \
            //      1  3
            wccff::lexer::file_location location{ 0, 0 };
            std::vector<wccff::lexer::token> tokens_vector;
            tokens_vector.emplace_back(wccff::lexer::token_type::constant, "1", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::plus_operator, "+", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::constant, "3", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::negation_operator, "-", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::constant, "5", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::semicolon, ";", location);
            wccff::parser::tokens tokens{ tokens_vector };

            auto r = wccff::parser::parse_expression(tokens);
            REQUIRE(r.has_value());
            REQUIRE(std::holds_alternative<std::unique_ptr<wccff::parser::binary_node>>(r.value()) == true);
            auto &exp = std::get<std::unique_ptr<wccff::parser::binary_node>>(r.value());

            REQUIRE(std::holds_alternative<wccff::parser::subtract_operator>(exp->op));

            REQUIRE(std::holds_alternative<std::unique_ptr<wccff::parser::binary_node>>(exp->left));
            auto &left = std::get<std::unique_ptr<wccff::parser::binary_node>>(exp->left);

            REQUIRE(std::holds_alternative<wccff::parser::plus_operator>(left->op));
            REQUIRE(std::holds_alternative<wccff::parser::int_constant>(left->left));
            REQUIRE(std::get<wccff::parser::int_constant>(left->left).value == 1);
            REQUIRE(std::holds_alternative<wccff::parser::int_constant>(left->right));
            REQUIRE(std::get<wccff::parser::int_constant>(left->right).value == 3);

            REQUIRE(std::holds_alternative<wccff::parser::int_constant>(exp->right));
            auto &right = std::get<wccff::parser::int_constant>(exp->right);
            REQUIRE(right.value == 5);
        }

        SECTION("1 * 3 - 5")
        {
            // This generates the following tree
            //          -
            //         / \
            //        *   5
            //       / \
            //      1  3
            wccff::lexer::file_location location{ 0, 0 };
            std::vector<wccff::lexer::token> tokens_vector;
            tokens_vector.emplace_back(wccff::lexer::token_type::constant, "1", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::multiplication_operator, "*", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::constant, "3", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::negation_operator, "-", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::constant, "5", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::semicolon, ";", location);
            wccff::parser::tokens tokens{ tokens_vector };

            auto r = wccff::parser::parse_expression(tokens);
            REQUIRE(r.has_value());
            REQUIRE(std::holds_alternative<std::unique_ptr<wccff::parser::binary_node>>(r.value()) == true);
            auto &exp = std::get<std::unique_ptr<wccff::parser::binary_node>>(r.value());

            REQUIRE(std::holds_alternative<wccff::parser::subtract_operator>(exp->op));

            REQUIRE(std::holds_alternative<std::unique_ptr<wccff::parser::binary_node>>(exp->left));
            auto &left = std::get<std::unique_ptr<wccff::parser::binary_node>>(exp->left);

            REQUIRE(std::holds_alternative<wccff::parser::multiply_operator>(left->op));
            REQUIRE(std::holds_alternative<wccff::parser::int_constant>(left->left));
            REQUIRE(std::get<wccff::parser::int_constant>(left->left).value == 1);
            REQUIRE(std::holds_alternative<wccff::parser::int_constant>(left->right));
            REQUIRE(std::get<wccff::parser::int_constant>(left->right).value == 3);

            REQUIRE(std::holds_alternative<wccff::parser::int_constant>(exp->right));
            auto &right = std::get<wccff::parser::int_constant>(exp->right);
            REQUIRE(right.value == 5);
        }

        SECTION("2 + 3 * 4")
        {
            // This generates the following tree
            //          +
            //         / \
            //        2   *
            //           / \
            //           3  4
            wccff::lexer::file_location location{ 0, 0 };
            std::vector<wccff::lexer::token> tokens_vector;
            tokens_vector.emplace_back(wccff::lexer::token_type::constant, "2", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::plus_operator, "+", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::constant, "3", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::multiplication_operator, "*", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::constant, "4", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::semicolon, ";", location);
            wccff::parser::tokens tokens{ tokens_vector };

            auto r = wccff::parser::parse_expression(tokens);
            REQUIRE(r.has_value());
            REQUIRE(std::holds_alternative<std::unique_ptr<wccff::parser::binary_node>>(r.value()) == true);
            auto &exp = std::get<std::unique_ptr<wccff::parser::binary_node>>(r.value());

            REQUIRE(std::holds_alternative<wccff::parser::plus_operator>(exp->op));

            REQUIRE(std::holds_alternative<wccff::parser::int_constant>(exp->left));
            REQUIRE(std::get<wccff::parser::int_constant>(exp->left).value == 2);

            REQUIRE(std::holds_alternative<std::unique_ptr<wccff::parser::binary_node>>(exp->right));
            auto &right = std::get<std::unique_ptr<wccff::parser::binary_node>>(exp->right);

            REQUIRE(std::holds_alternative<wccff::parser::multiply_operator>(right->op));
            REQUIRE(std::holds_alternative<wccff::parser::int_constant>(right->left));
            REQUIRE(std::get<wccff::parser::int_constant>(right->left).value == 3);
            REQUIRE(std::holds_alternative<wccff::parser::int_constant>(right->right));
            REQUIRE(std::get<wccff::parser::int_constant>(right->right).value == 4);
        }

        SECTION("(1 + 3) * 5")
        {
            // This generates the following tree
            //          *
            //         / \
            //        +   5
            //       / \
            //      1  3
            wccff::lexer::file_location location{ 0, 0 };
            std::vector<wccff::lexer::token> tokens_vector;
            tokens_vector.emplace_back(wccff::lexer::token_type::open_parenthesis, "(", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::constant, "1", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::plus_operator, "+", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::constant, "3", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::close_parenthesis, ")", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::multiplication_operator, "*", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::constant, "5", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::semicolon, ";", location);
            wccff::parser::tokens tokens{ tokens_vector };

            auto r = wccff::parser::parse_expression(tokens);
            REQUIRE(r.has_value());
            REQUIRE(std::holds_alternative<std::unique_ptr<wccff::parser::binary_node>>(r.value()) == true);
            auto &exp = std::get<std::unique_ptr<wccff::parser::binary_node>>(r.value());

            REQUIRE(std::holds_alternative<wccff::parser::multiply_operator>(exp->op));

            REQUIRE(std::holds_alternative<std::unique_ptr<wccff::parser::binary_node>>(exp->left));
            auto &left = std::get<std::unique_ptr<wccff::parser::binary_node>>(exp->left);

            REQUIRE(std::holds_alternative<wccff::parser::plus_operator>(left->op));
            REQUIRE(std::holds_alternative<wccff::parser::int_constant>(left->left));
            REQUIRE(std::get<wccff::parser::int_constant>(left->left).value == 1);
            REQUIRE(std::holds_alternative<wccff::parser::int_constant>(left->right));
            REQUIRE(std::get<wccff::parser::int_constant>(left->right).value == 3);

            REQUIRE(std::holds_alternative<wccff::parser::int_constant>(exp->right));
            auto &right = std::get<wccff::parser::int_constant>(exp->right);
            REQUIRE(right.value == 5);
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

TEST_CASE("Binary Operators", "[parser]")
{
    SECTION("Bitwise And Operator")
    {
        wccff::lexer::file_location location{ 1, 2 };
        std::vector<wccff::lexer::token> tokens_vector;
        tokens_vector.emplace_back(wccff::lexer::token_type::constant, "1", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::bitwise_and_operator, "&", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::constant, "2", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::semicolon, ";", location);

        wccff::parser::tokens tokens{ tokens_vector };
        auto r = wccff::parser::parse_expression(tokens);
        REQUIRE(r.has_value());
        REQUIRE(std::holds_alternative<std::unique_ptr<wccff::parser::binary_node>>(r.value()));
        const auto &exp = std::get<std::unique_ptr<wccff::parser::binary_node>>(r.value());

        REQUIRE(std::holds_alternative<wccff::parser::bitwise_and_operator>(exp->op));
        REQUIRE(std::holds_alternative<wccff::parser::int_constant>(exp->left));
        REQUIRE(std::get<wccff::parser::int_constant>(exp->left).value == 1);
        REQUIRE(std::holds_alternative<wccff::parser::int_constant>(exp->right));
        REQUIRE(std::get<wccff::parser::int_constant>(exp->right).value == 2);
    }
}
