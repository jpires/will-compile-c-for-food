#include "../parser.h"
#include "../tacky.h"
#include "parser_data.h"
#include "parser_helpers.h"
#include <ApprovalTests.hpp>
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
    SECTION("Parse Negate Operator")
    {
        wccff::lexer::file_location location{ 1, 3 };
        std::vector<wccff::lexer::token> tokens_vector;
        tokens_vector.emplace_back(wccff::lexer::token_type::negation_operator, "-", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "2", location);

        wccff::parser::tokens tokens{ tokens_vector };

        auto r = wccff::parser::parse_unary_node(tokens);

        REQUIRE(r.has_value());
        REQUIRE(std::holds_alternative<wccff::parser::negate_operator>(r.value()->op) == true);
        REQUIRE(wccff::testing::is_constant_expression_of<wccff::parser::int_constant>(r.value()->exp, 2));
    }

    SECTION("Parse Bitwise Complement Operator")
    {
        wccff::lexer::file_location location{ 1, 3 };
        std::vector<wccff::lexer::token> tokens_vector;
        tokens_vector.emplace_back(wccff::lexer::token_type::bitwise_complement_operator, "~", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "2", location);

        wccff::parser::tokens tokens{ tokens_vector };

        auto r = wccff::parser::parse_unary_node(tokens);

        REQUIRE(r.has_value());
        REQUIRE(std::holds_alternative<wccff::parser::bitwise_complement_operator>(r.value()->op) == true);
        REQUIRE(wccff::testing::is_constant_expression_of<wccff::parser::int_constant>(r.value()->exp, 2));
    }

    SECTION("Parse two Unary Operator in a row")
    {
        SECTION("Two bitwise complement operators")
        {
            wccff::lexer::file_location location{ 1, 3 };
            std::vector<wccff::lexer::token> tokens_vector;
            tokens_vector.emplace_back(wccff::lexer::token_type::bitwise_complement_operator, "~", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::bitwise_complement_operator, "~", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "2", location);

            wccff::parser::tokens tokens{ tokens_vector };

            auto r = wccff::parser::parse_unary_node(tokens);

            REQUIRE(r.has_value());
            REQUIRE(std::holds_alternative<wccff::parser::bitwise_complement_operator>(r.value()->op) == true);
            REQUIRE(std::holds_alternative<std::unique_ptr<wccff::parser::unary_node>>(r.value()->exp) == true);

            auto inner_expression = std::move(std::get<std::unique_ptr<wccff::parser::unary_node>>(r.value()->exp));
            REQUIRE(std::holds_alternative<wccff::parser::bitwise_complement_operator>(inner_expression->op) == true);
            REQUIRE(wccff::testing::is_constant_expression_of<wccff::parser::int_constant>(inner_expression->exp, 2));
        }

        SECTION("Negate and bitwise complement operators")
        {
            wccff::lexer::file_location location{ 1, 3 };
            std::vector<wccff::lexer::token> tokens_vector;
            tokens_vector.emplace_back(wccff::lexer::token_type::negation_operator, "-", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::bitwise_complement_operator, "~", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "2", location);

            wccff::parser::tokens tokens{ tokens_vector };

            auto r = wccff::parser::parse_unary_node(tokens);

            REQUIRE(r.has_value());
            REQUIRE(std::holds_alternative<wccff::parser::negate_operator>(r.value()->op) == true);
            REQUIRE(std::holds_alternative<std::unique_ptr<wccff::parser::unary_node>>(r.value()->exp) == true);

            auto inner_expression = std::move(std::get<std::unique_ptr<wccff::parser::unary_node>>(r.value()->exp));
            REQUIRE(std::holds_alternative<wccff::parser::bitwise_complement_operator>(inner_expression->op) == true);
            REQUIRE(wccff::testing::is_constant_expression_of<wccff::parser::int_constant>(inner_expression->exp, 2));
        }
    }

    SECTION("Binary Operators")
    {
        SECTION("Plus operator")
        {
            wccff::lexer::file_location location{ 0, 0 };
            std::vector<wccff::lexer::token> tokens_vector;
            tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "1", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::plus_operator, "+", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "2", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::semicolon, ";", location);
            wccff::parser::tokens tokens{ tokens_vector };

            auto r = wccff::parser::parse_expression(tokens);
            REQUIRE(r.has_value());
            REQUIRE(std::holds_alternative<std::unique_ptr<wccff::parser::binary_node>>(r.value()) == true);
            auto &exp = std::get<std::unique_ptr<wccff::parser::binary_node>>(r.value());

            REQUIRE(std::holds_alternative<wccff::parser::plus_operator>(exp->op));
            REQUIRE(wccff::testing::is_constant_expression_of<wccff::parser::int_constant>(exp->left, 1));
            REQUIRE(wccff::testing::is_constant_expression_of<wccff::parser::int_constant>(exp->right, 2));
        }

        SECTION("Subtraction operator")
        {
            wccff::lexer::file_location location{ 0, 0 };
            std::vector<wccff::lexer::token> tokens_vector;
            tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "2", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::negation_operator, "-", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "1", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::semicolon, ";", location);
            wccff::parser::tokens tokens{ tokens_vector };

            auto r = wccff::parser::parse_expression(tokens);
            REQUIRE(r.has_value());
            REQUIRE(std::holds_alternative<std::unique_ptr<wccff::parser::binary_node>>(r.value()) == true);
            auto &exp = std::get<std::unique_ptr<wccff::parser::binary_node>>(r.value());

            REQUIRE(std::holds_alternative<wccff::parser::subtract_operator>(exp->op));
            REQUIRE(wccff::testing::is_constant_expression_of<wccff::parser::int_constant>(exp->left, 2));
            REQUIRE(wccff::testing::is_constant_expression_of<wccff::parser::int_constant>(exp->right, 1));
        }

        SECTION("Multiplication operator")
        {
            wccff::lexer::file_location location{ 0, 0 };
            std::vector<wccff::lexer::token> tokens_vector;
            tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "2", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::multiplication_operator, "*", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "1", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::semicolon, ";", location);
            wccff::parser::tokens tokens{ tokens_vector };

            auto r = wccff::parser::parse_expression(tokens);
            REQUIRE(r.has_value());
            REQUIRE(std::holds_alternative<std::unique_ptr<wccff::parser::binary_node>>(r.value()) == true);
            auto &exp = std::get<std::unique_ptr<wccff::parser::binary_node>>(r.value());

            REQUIRE(std::holds_alternative<wccff::parser::multiply_operator>(exp->op));
            REQUIRE(wccff::testing::is_constant_expression_of<wccff::parser::int_constant>(exp->left, 2));
            REQUIRE(wccff::testing::is_constant_expression_of<wccff::parser::int_constant>(exp->right, 1));
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
            tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "1", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::plus_operator, "+", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "3", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::negation_operator, "-", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "5", location);
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

            REQUIRE(wccff::testing::is_constant_expression_of<wccff::parser::int_constant>(left->left, 1));
            REQUIRE(wccff::testing::is_constant_expression_of<wccff::parser::int_constant>(left->right, 3));
            REQUIRE(wccff::testing::is_constant_expression_of<wccff::parser::int_constant>(exp->right, 5));
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
            tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "1", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::multiplication_operator, "*", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "3", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::negation_operator, "-", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "5", location);
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

            REQUIRE(wccff::testing::is_constant_expression_of<wccff::parser::int_constant>(left->left, 1));
            REQUIRE(wccff::testing::is_constant_expression_of<wccff::parser::int_constant>(left->right, 3));
            REQUIRE(wccff::testing::is_constant_expression_of<wccff::parser::int_constant>(exp->right, 5));
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
            tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "2", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::plus_operator, "+", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "3", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::multiplication_operator, "*", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "4", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::semicolon, ";", location);
            wccff::parser::tokens tokens{ tokens_vector };

            auto r = wccff::parser::parse_expression(tokens);
            REQUIRE(r.has_value());
            REQUIRE(std::holds_alternative<std::unique_ptr<wccff::parser::binary_node>>(r.value()) == true);
            auto &exp = std::get<std::unique_ptr<wccff::parser::binary_node>>(r.value());

            REQUIRE(std::holds_alternative<wccff::parser::plus_operator>(exp->op));
            REQUIRE(std::holds_alternative<std::unique_ptr<wccff::parser::binary_node>>(exp->right));
            auto &right = std::get<std::unique_ptr<wccff::parser::binary_node>>(exp->right);

            REQUIRE(wccff::testing::is_constant_expression_of<wccff::parser::int_constant>(exp->left, 2));
            REQUIRE(wccff::testing::is_constant_expression_of<wccff::parser::int_constant>(right->left, 3));
            REQUIRE(wccff::testing::is_constant_expression_of<wccff::parser::int_constant>(right->right, 4));
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
            tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "1", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::plus_operator, "+", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "3", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::close_parenthesis, ")", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::multiplication_operator, "*", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "5", location);
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

            REQUIRE(wccff::testing::is_constant_expression_of<wccff::parser::int_constant>(left->left, 1));
            REQUIRE(wccff::testing::is_constant_expression_of<wccff::parser::int_constant>(left->right, 3));
            REQUIRE(wccff::testing::is_constant_expression_of<wccff::parser::int_constant>(exp->right, 5));
        }
    }
}

TEST_CASE("Parser complex test", "[parser]")
{
    SECTION("Main function returns 2")
    {
        auto directoryDisposer = ApprovalTests::Approvals::useApprovalsSubdirectory("parser_tests");
        wccff::lexer::file_location location{ 1, 2 };
        std::vector<wccff::lexer::token> tokens_vector;
        tokens_vector.emplace_back(wccff::lexer::token_type::int_keyword, "int", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "main", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::open_parenthesis, "(", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::void_keyword, "void", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::close_parenthesis, ")", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::open_brace, "{", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::return_keyword, "return", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "2", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::semicolon, ";", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::close_brace, "}", location);

        wccff::parser::tokens tokens{ tokens_vector };

        auto r = wccff::parser::parse(tokens);
        REQUIRE(r.has_value());
        ApprovalTests::Approvals::verify(pretty_print(r.value()));
    }
}

TEST_CASE("Binary Operators", "[parser]")
{
    auto directoryDisposer = ApprovalTests::Approvals::useApprovalsSubdirectory("parser_tests");
    SECTION("Bitwise And Operator")
    {
        wccff::lexer::file_location location{ 1, 2 };
        std::vector<wccff::lexer::token> tokens_vector;
        tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "1", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::bitwise_and_operator, "&", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "2", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::semicolon, ";", location);

        wccff::parser::tokens tokens{ tokens_vector };
        auto r = wccff::parser::parse_expression(tokens);
        REQUIRE(r.has_value());
        REQUIRE(std::holds_alternative<std::unique_ptr<wccff::parser::binary_node>>(r.value()));
        const auto &exp = std::get<std::unique_ptr<wccff::parser::binary_node>>(r.value());

        REQUIRE(std::holds_alternative<wccff::parser::bitwise_and_operator>(exp->op));
        REQUIRE(wccff::testing::is_constant_expression_of<wccff::parser::int_constant>(exp->left, 1));
        REQUIRE(wccff::testing::is_constant_expression_of<wccff::parser::int_constant>(exp->right, 2));
    }

    SECTION("Bitwise Or Operator")
    {
        wccff::lexer::file_location location{ 1, 2 };
        std::vector<wccff::lexer::token> tokens_vector;
        tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "1", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::bitwise_or_operator, "|", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "2", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::semicolon, ";", location);

        wccff::parser::tokens tokens{ tokens_vector };
        auto r = wccff::parser::parse_expression(tokens);
        REQUIRE(r.has_value());
        REQUIRE(std::holds_alternative<std::unique_ptr<wccff::parser::binary_node>>(r.value()));
        const auto &exp = std::get<std::unique_ptr<wccff::parser::binary_node>>(r.value());

        REQUIRE(std::holds_alternative<wccff::parser::bitwise_or_operator>(exp->op));
        REQUIRE(wccff::testing::is_constant_expression_of<wccff::parser::int_constant>(exp->left, 1));
        REQUIRE(wccff::testing::is_constant_expression_of<wccff::parser::int_constant>(exp->right, 2));
    }
    SECTION("Bitwise Xor Operator")
    {
        wccff::lexer::file_location location{ 1, 2 };
        std::vector<wccff::lexer::token> tokens_vector;
        tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "1", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::bitwise_xor_operator, "^", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "2", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::semicolon, ";", location);

        wccff::parser::tokens tokens{ tokens_vector };
        auto r = wccff::parser::parse_expression(tokens);
        REQUIRE(r.has_value());
        REQUIRE(std::holds_alternative<std::unique_ptr<wccff::parser::binary_node>>(r.value()));
        const auto &exp = std::get<std::unique_ptr<wccff::parser::binary_node>>(r.value());

        REQUIRE(std::holds_alternative<wccff::parser::bitwise_xor_operator>(exp->op));
        REQUIRE(wccff::testing::is_constant_expression_of<wccff::parser::int_constant>(exp->left, 1));
        REQUIRE(wccff::testing::is_constant_expression_of<wccff::parser::int_constant>(exp->right, 2));
    }
    SECTION("Left Shift Operator")
    {
        wccff::lexer::file_location location{ 1, 2 };
        std::vector<wccff::lexer::token> tokens_vector;
        tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "1", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::left_shift_operator, "<<", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "2", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::semicolon, ";", location);

        wccff::parser::tokens tokens{ tokens_vector };
        auto r = wccff::parser::parse_expression(tokens);
        REQUIRE(r.has_value());
        REQUIRE(std::holds_alternative<std::unique_ptr<wccff::parser::binary_node>>(r.value()));
        const auto &exp = std::get<std::unique_ptr<wccff::parser::binary_node>>(r.value());

        REQUIRE(std::holds_alternative<wccff::parser::left_shift_operator>(exp->op));
        REQUIRE(wccff::testing::is_constant_expression_of<wccff::parser::int_constant>(exp->left, 1));
        REQUIRE(wccff::testing::is_constant_expression_of<wccff::parser::int_constant>(exp->right, 2));
    }
    SECTION("Right Shift Operator")
    {
        wccff::lexer::file_location location{ 1, 2 };
        std::vector<wccff::lexer::token> tokens_vector;
        tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "1", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::right_shift_operator, ">>", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "2", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::semicolon, ";", location);

        wccff::parser::tokens tokens{ tokens_vector };
        auto r = wccff::parser::parse_expression(tokens);
        REQUIRE(r.has_value());
        REQUIRE(std::holds_alternative<std::unique_ptr<wccff::parser::binary_node>>(r.value()));
        const auto &exp = std::get<std::unique_ptr<wccff::parser::binary_node>>(r.value());

        REQUIRE(std::holds_alternative<wccff::parser::right_shift_operator>(exp->op));
        REQUIRE(wccff::testing::is_constant_expression_of<wccff::parser::int_constant>(exp->left, 1));
        REQUIRE(wccff::testing::is_constant_expression_of<wccff::parser::int_constant>(exp->right, 2));
    }

    SECTION("Single Assignment")
    {
        wccff::lexer::file_location location{ 1, 3 };
        std::vector<wccff::lexer::token> tokens_vector;

        tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "a", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::assignment_operator, "=", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "2", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::semicolon, ";", location);

        wccff::parser::tokens tokens{ tokens_vector };
        auto result = wccff::parser::parse_expression(tokens);
        REQUIRE(result.has_value());
        ApprovalTests::Approvals::verify(wccff::parser::pretty_print(result.value()));
    }

    SECTION("Multiple Assignment")
    {
        wccff::lexer::file_location location{ 1, 3 };
        std::vector<wccff::lexer::token> tokens_vector;

        tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "a", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::assignment_operator, "=", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "b", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::assignment_operator, "=", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "2", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::semicolon, ";", location);

        wccff::parser::tokens tokens{ tokens_vector };
        auto result = wccff::parser::parse_expression(tokens);
        REQUIRE(result.has_value());
        ApprovalTests::Approvals::verify(wccff::parser::pretty_print(result.value()));
    }
}

TEST_CASE("Parse Constants", "[parser]")
{
    SECTION("Invalid")
    {
        SECTION("Wrong token")
        {
            wccff::lexer::file_location location{ 1, 3 };
            std::vector<wccff::lexer::token> tokens_vector;
            tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "foo", location);

            wccff::parser::tokens tokens{ tokens_vector };

            auto r = wccff::parser::parse_constant(tokens);

            REQUIRE(r.has_value() == false);
        }

        SECTION("Out of range")
        {
            wccff::lexer::file_location location{ 1, 3 };
            std::vector<wccff::lexer::token> tokens_vector;
            // The token is 9223372036854775807 + 1.
            tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "9223372036854775808", location);

            wccff::parser::tokens tokens{ tokens_vector };

            auto r = wccff::parser::parse_constant(tokens);

            REQUIRE(r.has_value() == false);
        }

        SECTION("Unable to parse")
        {
            wccff::lexer::file_location location{ 1, 3 };
            std::vector<wccff::lexer::token> tokens_vector;
            // Check the behaviour for when the text isn't parsable as an integer value
            tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "xpto", location);

            wccff::parser::tokens tokens{ tokens_vector };

            auto r = wccff::parser::parse_constant(tokens);

            REQUIRE(r.has_value() == false);
        }
    }
    SECTION("Int")
    {
        SECTION("Small int")
        {
            wccff::lexer::file_location location{ 1, 3 };
            std::vector<wccff::lexer::token> tokens_vector;
            tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "123", location);

            wccff::parser::tokens tokens{ tokens_vector };

            auto r = wccff::parser::parse_constant(tokens);

            REQUIRE(r.has_value());
            REQUIRE(std::holds_alternative<wccff::parser::int_constant>(r.value()));
            REQUIRE(std::get<wccff::parser::int_constant>(r.value()).value == 123);
        }

        SECTION("max int32 value")
        {
            wccff::lexer::file_location location{ 1, 3 };
            std::vector<wccff::lexer::token> tokens_vector;
            tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "2147483647", location);

            wccff::parser::tokens tokens{ tokens_vector };

            auto r = wccff::parser::parse_constant(tokens);

            REQUIRE(r.has_value());
            REQUIRE(std::holds_alternative<wccff::parser::int_constant>(r.value()));
            REQUIRE(std::get<wccff::parser::int_constant>(r.value()).value == 2'147'483'647);
        }

        SECTION("Bigger than max int32 value")
        {
            wccff::lexer::file_location location{ 1, 3 };
            std::vector<wccff::lexer::token> tokens_vector;
            tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "2147483648", location);

            wccff::parser::tokens tokens{ tokens_vector };

            auto r = wccff::parser::parse_constant(tokens);

            REQUIRE(r.has_value());
            REQUIRE(std::holds_alternative<wccff::parser::long_constant>(r.value()));
            REQUIRE(std::get<wccff::parser::long_constant>(r.value()).value == 2'147'483'648);
        }
    }

    SECTION("Long")
    {
        SECTION("Small long")
        {
            wccff::lexer::file_location location{ 1, 3 };
            std::vector<wccff::lexer::token> tokens_vector;
            tokens_vector.emplace_back(wccff::lexer::token_type::long_constant, "123l", location);

            wccff::parser::tokens tokens{ tokens_vector };

            auto r = wccff::parser::parse_constant(tokens);

            REQUIRE(r.has_value());
            REQUIRE(std::holds_alternative<wccff::parser::long_constant>(r.value()));
            REQUIRE(std::get<wccff::parser::long_constant>(r.value()).value == 123);
        }

        SECTION("max int32 value")
        {
            wccff::lexer::file_location location{ 1, 3 };
            std::vector<wccff::lexer::token> tokens_vector;
            tokens_vector.emplace_back(wccff::lexer::token_type::long_constant, "2147483647l", location);

            wccff::parser::tokens tokens{ tokens_vector };

            auto r = wccff::parser::parse_constant(tokens);

            REQUIRE(r.has_value());
            REQUIRE(std::holds_alternative<wccff::parser::long_constant>(r.value()));
            REQUIRE(std::get<wccff::parser::long_constant>(r.value()).value == 2'147'483'647);
        }

        SECTION("Bigger than max int32 value")
        {
            wccff::lexer::file_location location{ 1, 3 };
            std::vector<wccff::lexer::token> tokens_vector;
            tokens_vector.emplace_back(wccff::lexer::token_type::long_constant, "2147483648l", location);

            wccff::parser::tokens tokens{ tokens_vector };

            auto r = wccff::parser::parse_constant(tokens);

            REQUIRE(r.has_value());
            REQUIRE(std::holds_alternative<wccff::parser::long_constant>(r.value()));
            REQUIRE(std::get<wccff::parser::long_constant>(r.value()).value == 2'147'483'648);
        }
    }
}

TEST_CASE("Parse Statements", "[parser]")
{
    using wccff::parser::pretty_print;

    auto directoryDisposer = ApprovalTests::Approvals::useApprovalsSubdirectory("parser_tests");
    wccff::lexer::file_location location{ 0, 0 };

    SECTION("Labelled Statement")
    {
        std::vector<wccff::lexer::token> tokens_vector;
        tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "label1", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::colon, ":", location);

        tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "var1", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::compound_plus, "+=", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "42", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::semicolon, ";", location);

        wccff::parser::tokens tokens{ tokens_vector };
        auto result = wccff::parser::parse_statement(tokens);
        REQUIRE(result.has_value());

        ApprovalTests::Approvals::verify(pretty_print(result.value()));
    }
    SECTION("Goto Statement")
    {
        std::vector<wccff::lexer::token> tokens_vector;
        tokens_vector.emplace_back(wccff::lexer::token_type::goto_keyword, "goto", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "label1", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::semicolon, ";", location);

        wccff::parser::tokens tokens{ tokens_vector };
        auto result = wccff::parser::parse_statement(tokens);
        REQUIRE(result.has_value());

        ApprovalTests::Approvals::verify(pretty_print(result.value()));
    }
}

TEST_CASE("Loop Statements", "[parser]")
{
    using wccff::parser::pretty_print;

    auto directoryDisposer = ApprovalTests::Approvals::useApprovalsSubdirectory("parser_tests");
    wccff::lexer::file_location location{ 0, 0 };

    SECTION("do while loops")
    {
        SECTION("complete")
        {
            std::vector<wccff::lexer::token> tokens_vector;
            tokens_vector.emplace_back(wccff::lexer::token_type::do_keyword, "do", location);

            tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "i", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::compound_plus, "+=", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "1", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::semicolon, ";", location);

            tokens_vector.emplace_back(wccff::lexer::token_type::while_keyword, "while", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::open_parenthesis, "(", location);

            tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "a", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::less_than_operator, "<", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "10", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::close_parenthesis, ")", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::semicolon, ";", location);

            wccff::parser::tokens tokens{ tokens_vector };
            auto result = wccff::parser::parse_do_while(tokens);
            REQUIRE(result.has_value());

            ApprovalTests::Approvals::verify(pretty_print(result.value()));
        }
    }
    SECTION("for loops")
    {
        SECTION("good loops")
        {
            SECTION("complete")
            {
                std::vector<wccff::lexer::token> tokens_vector;
                tokens_vector.emplace_back(wccff::lexer::token_type::for_keyword, "for", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::open_parenthesis, "(", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::int_keyword, "int", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "a", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::assignment_operator, "=", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "0", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::semicolon, ";", location);

                tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "a", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::less_than_operator, "<", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "10", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::semicolon, ";", location);

                tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "a", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::compound_plus, "+=", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "1", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::close_parenthesis, ")", location);

                tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "i", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::compound_plus, "+=", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "1", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::semicolon, ";", location);

                wccff::parser::tokens tokens{ tokens_vector };
                auto result = wccff::parser::parse_for_statement(tokens);
                REQUIRE(result.has_value());

                ApprovalTests::Approvals::verify(pretty_print(result.value()));
            }
            SECTION("without_init")
            {
                std::vector<wccff::lexer::token> tokens_vector;
                tokens_vector.emplace_back(wccff::lexer::token_type::for_keyword, "for", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::open_parenthesis, "(", location);

                tokens_vector.emplace_back(wccff::lexer::token_type::semicolon, ";", location);

                tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "a", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::less_than_operator, "<", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "10", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::semicolon, ";", location);

                tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "a", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::compound_plus, "+=", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "1", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::close_parenthesis, ")", location);

                tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "i", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::compound_plus, "+=", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "1", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::semicolon, ";", location);

                wccff::parser::tokens tokens{ tokens_vector };
                auto result = wccff::parser::parse_for_statement(tokens);
                REQUIRE(result.has_value());

                ApprovalTests::Approvals::verify(pretty_print(result.value()));
            }
            SECTION("without_condition")
            {
                std::vector<wccff::lexer::token> tokens_vector;
                tokens_vector.emplace_back(wccff::lexer::token_type::for_keyword, "for", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::open_parenthesis, "(", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::int_keyword, "int", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "a", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::assignment_operator, "=", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "0", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::semicolon, ";", location);

                tokens_vector.emplace_back(wccff::lexer::token_type::semicolon, ";", location);

                tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "a", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::compound_plus, "+=", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "1", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::close_parenthesis, ")", location);

                tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "i", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::compound_plus, "+=", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "1", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::semicolon, ";", location);

                wccff::parser::tokens tokens{ tokens_vector };
                auto result = wccff::parser::parse_for_statement(tokens);
                REQUIRE(result.has_value());

                ApprovalTests::Approvals::verify(pretty_print(result.value()));
            }
            SECTION("without_post")
            {
                std::vector<wccff::lexer::token> tokens_vector;
                tokens_vector.emplace_back(wccff::lexer::token_type::for_keyword, "for", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::open_parenthesis, "(", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::int_keyword, "int", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "a", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::assignment_operator, "=", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "0", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::semicolon, ";", location);

                tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "a", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::less_than_operator, "<", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "10", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::semicolon, ";", location);

                tokens_vector.emplace_back(wccff::lexer::token_type::close_parenthesis, ")", location);

                tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "i", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::compound_plus, "+=", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "1", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::semicolon, ";", location);

                wccff::parser::tokens tokens{ tokens_vector };
                auto result = wccff::parser::parse_for_statement(tokens);
                REQUIRE(result.has_value());

                ApprovalTests::Approvals::verify(pretty_print(result.value()));
            }
        }
    }

    SECTION("while loops")
    {
        SECTION("complete")
        {
            std::vector<wccff::lexer::token> tokens_vector;

            tokens_vector.emplace_back(wccff::lexer::token_type::while_keyword, "while", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::open_parenthesis, "(", location);

            tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "a", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::less_than_operator, "<", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "10", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::close_parenthesis, ")", location);

            tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "i", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::compound_plus, "+=", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "1", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::semicolon, ";", location);

            wccff::parser::tokens tokens{ tokens_vector };
            auto result = wccff::parser::parse_while_statement(tokens);
            REQUIRE(result.has_value());

            ApprovalTests::Approvals::verify(pretty_print(result.value()));
        }
    }
}

TEST_CASE("parse_argument_list", "[parser]")
{
    using namespace wccff;
    auto directoryDisposer = ApprovalTests::Approvals::useApprovalsSubdirectory("parser_tests");

    wccff::lexer::file_location location{ 0, 0 };
    SECTION("No Arguments")
    {
        std::vector<wccff::lexer::token> tokens_vector;
        tokens_vector.emplace_back(wccff::lexer::token_type::close_parenthesis, ")", location);

        wccff::parser::tokens tokens{ tokens_vector };
        auto result = wccff::parser::parse_argument_list(tokens);
        REQUIRE(result.has_value());
        REQUIRE(result.value().empty());
    }

    SECTION("One Argument")
    {
        std::vector<wccff::lexer::token> tokens_vector;
        tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "42", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::close_parenthesis, ")", location);

        wccff::parser::tokens tokens{ tokens_vector };
        auto result = wccff::parser::parse_argument_list(tokens);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(wccff::testing::is_constant_expression_of<wccff::parser::int_constant>(result.value()[0], 42));
    }

    SECTION("Three Argument")
    {
        std::vector<wccff::lexer::token> tokens_vector;
        tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "42", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::comma, ",", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "43", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::comma, ",", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "44", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::close_parenthesis, ")", location);

        wccff::parser::tokens tokens{ tokens_vector };
        auto result = wccff::parser::parse_argument_list(tokens);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 3);
        REQUIRE(wccff::testing::is_constant_expression_of<wccff::parser::int_constant>(result.value()[0], 42));
        REQUIRE(wccff::testing::is_constant_expression_of<wccff::parser::int_constant>(result.value()[1], 43));
        REQUIRE(wccff::testing::is_constant_expression_of<wccff::parser::int_constant>(result.value()[2], 44));
    }

    SECTION("One Argument with complex expression")
    {
        std::vector<wccff::lexer::token> tokens_vector;
        tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "42", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::plus_operator, "+", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "43", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::close_parenthesis, ")", location);

        wccff::parser::tokens tokens{ tokens_vector };
        auto result = wccff::parser::parse_argument_list(tokens);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(std::holds_alternative<std::unique_ptr<parser::binary_node>>(result.value()[0]));
        auto &node = std::get<std::unique_ptr<parser::binary_node>>(result.value()[0]);
        REQUIRE(std::holds_alternative<parser::plus_operator>(node->op));
        REQUIRE(wccff::testing::is_constant_expression_of<wccff::parser::int_constant>(node->left, 42));
        REQUIRE(wccff::testing::is_constant_expression_of<wccff::parser::int_constant>(node->right, 43));
    }

    SECTION("Invalid Argument")
    {
        std::vector<wccff::lexer::token> tokens_vector;
        tokens_vector.emplace_back(wccff::lexer::token_type::int_keyword, "int", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "a", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::assignment_operator, "=", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "44", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::close_parenthesis, ")", location);

        wccff::parser::tokens tokens{ tokens_vector };
        auto result = wccff::parser::parse_argument_list(tokens);
        REQUIRE(result.has_value() == false);
    }

    SECTION("Invalid Comma")
    {
        std::vector<wccff::lexer::token> tokens_vector;
        tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "44", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::comma, ",", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::close_parenthesis, ")", location);

        wccff::parser::tokens tokens{ tokens_vector };
        auto result = wccff::parser::parse_argument_list(tokens);
        REQUIRE(result.has_value() == false);
    }
}

TEST_CASE("parse_cast_expression", "[parser]")
{
    using namespace wccff;
    wccff::lexer::file_location location{ 0, 0 };
    std::vector<wccff::lexer::token> tokens_vector;
    tokens_vector.emplace_back(wccff::lexer::token_type::open_parenthesis, "(", location);
    tokens_vector.emplace_back(wccff::lexer::token_type::int_keyword, "int", location);
    tokens_vector.emplace_back(wccff::lexer::token_type::close_parenthesis, ")", location);
    tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "42", location);

    wccff::parser::tokens tokens{ tokens_vector };
    auto restult = parse_cast_expression(tokens);
    REQUIRE(restult.has_value());
}
TEST_CASE("parse_function_call", "[parser]")
{
    using namespace wccff;
    auto directoryDisposer = ApprovalTests::Approvals::useApprovalsSubdirectory("parser_tests");
    wccff::lexer::file_location location{ 0, 0 };

    SECTION("No Arguments")
    {
        std::vector<wccff::lexer::token> tokens_vector;
        tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "func1", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::open_parenthesis, "(", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::close_parenthesis, ")", location);

        wccff::parser::tokens tokens{ tokens_vector };
        auto result = wccff::parser::parse_function_call(tokens);
        REQUIRE(result.has_value());
        REQUIRE(result.value()->name.name == "func1");
        REQUIRE(result.value()->arguments.empty());
    }
    SECTION("One Argument")
    {
        std::vector<wccff::lexer::token> tokens_vector;
        tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "func1", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::open_parenthesis, "(", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "42", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::close_parenthesis, ")", location);

        wccff::parser::tokens tokens{ tokens_vector };
        auto result = wccff::parser::parse_function_call(tokens);
        REQUIRE(result.has_value());
        REQUIRE(result.value()->name.name == "func1");
        REQUIRE(result.value()->arguments.size() == 1);
        REQUIRE(
          wccff::testing::is_constant_expression_of<wccff::parser::int_constant>(result.value()->arguments[0], 42));
    }
}

TEST_CASE("parse_function_declaration", "[parser]")
{
    using namespace wccff;
    auto directoryDisposer = ApprovalTests::Approvals::useApprovalsSubdirectory("parser_tests");

    wccff::lexer::file_location location{ 0, 0 };
    SECTION("No Arguments, no Body")
    {
        std::vector<wccff::lexer::token> tokens_vector;
        tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "func", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::open_parenthesis, "(", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::close_parenthesis, ")", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::semicolon, ";", location);

        wccff::parser::tokens tokens{ tokens_vector };
        auto result = wccff::parser::parse_function_declaration(tokens, {});
        REQUIRE(result.has_value());
        REQUIRE(result.value().name.name == "func");
        REQUIRE(result.value().arguments.empty());
        REQUIRE(result.value().body.has_value() == false);
    }

    SECTION("No Arguments, no Body")
    {
        std::vector<wccff::lexer::token> tokens_vector;
        tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "func", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::open_parenthesis, "(", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::close_parenthesis, ")", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::open_brace, "{", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::return_keyword, "return", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::int_constant, "42", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::semicolon, ";", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::close_brace, "}", location);

        wccff::parser::tokens tokens{ tokens_vector };
        auto result = wccff::parser::parse_function_declaration(tokens, {});
        REQUIRE(result.has_value());
        REQUIRE(result.value().name.name == "func");
        REQUIRE(result.value().arguments.empty());
        REQUIRE(result.value().body.has_value());
    }
}

TEST_CASE("parse_params_list", "[parser]")
{
    using namespace wccff;
    using parser::identifier;
    using parser::int_type;
    using parser::long_type;
    auto directoryDisposer = ApprovalTests::Approvals::useApprovalsSubdirectory("parser_tests");

    wccff::lexer::file_location location{ 0, 0 };
    SECTION("No Arguments")
    {
        std::vector<wccff::lexer::token> tokens_vector;
        tokens_vector.emplace_back(wccff::lexer::token_type::open_parenthesis, "(", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::void_keyword, "void", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::close_parenthesis, ")", location);

        wccff::parser::tokens tokens{ tokens_vector };
        auto result = wccff::parser::parse_params_list(tokens);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value()[0].name == identifier{ "NOT.VALID" });
        REQUIRE(std::holds_alternative<parser::void_type>(result.value()[0].p_type));
        REQUIRE(tokens.get_next_token().has_value() == false);
    }

    SECTION("Int Argument")
    {
        std::vector<wccff::lexer::token> tokens_vector;
        tokens_vector.emplace_back(wccff::lexer::token_type::open_parenthesis, "(", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::int_keyword, "int", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "hello", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::close_parenthesis, ")", location);

        wccff::parser::tokens tokens{ tokens_vector };
        auto result = wccff::parser::parse_params_list(tokens);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value()[0].name == identifier{ "hello" });
        REQUIRE(std::holds_alternative<int_type>(result.value()[0].p_type));
        REQUIRE(tokens.get_next_token().has_value() == false);
    }

    SECTION("Long Argument")
    {
        std::vector<wccff::lexer::token> tokens_vector;
        tokens_vector.emplace_back(wccff::lexer::token_type::open_parenthesis, "(", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::long_keyword, "long", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "hello", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::close_parenthesis, ")", location);

        wccff::parser::tokens tokens{ tokens_vector };
        auto result = wccff::parser::parse_params_list(tokens);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value()[0].name == identifier{ "hello" });
        REQUIRE(std::holds_alternative<long_type>(result.value()[0].p_type));
        REQUIRE(tokens.get_next_token().has_value() == false);
    }

    SECTION("Int Long Argument")
    {
        std::vector<wccff::lexer::token> tokens_vector;
        tokens_vector.emplace_back(wccff::lexer::token_type::open_parenthesis, "(", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::int_keyword, "int", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::long_keyword, "long", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "hello", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::close_parenthesis, ")", location);

        wccff::parser::tokens tokens{ tokens_vector };
        auto result = wccff::parser::parse_params_list(tokens);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value()[0].name == identifier{ "hello" });
        REQUIRE(std::holds_alternative<long_type>(result.value()[0].p_type));
        REQUIRE(tokens.get_next_token().has_value() == false);
    }

    SECTION("Long Int Argument")
    {
        std::vector<wccff::lexer::token> tokens_vector;
        tokens_vector.emplace_back(wccff::lexer::token_type::open_parenthesis, "(", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::long_keyword, "long", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::int_keyword, "int", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "hello", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::close_parenthesis, ")", location);

        wccff::parser::tokens tokens{ tokens_vector };
        auto result = wccff::parser::parse_params_list(tokens);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value()[0].name == identifier{ "hello" });
        REQUIRE(std::holds_alternative<long_type>(result.value()[0].p_type));
        REQUIRE(tokens.get_next_token().has_value() == false);
    }
    SECTION("Three Argument")
    {
        std::vector<wccff::lexer::token> tokens_vector;
        tokens_vector.emplace_back(wccff::lexer::token_type::open_parenthesis, "(", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::int_keyword, "int", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "hello1", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::comma, ",", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::long_keyword, "long", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "hello2", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::comma, ",", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::int_keyword, "int", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::long_keyword, "long", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "hello3", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::close_parenthesis, ")", location);

        wccff::parser::tokens tokens{ tokens_vector };
        auto result = wccff::parser::parse_params_list(tokens);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 3);
        REQUIRE(result.value()[0].name == identifier{ "hello1" });
        REQUIRE(std::holds_alternative<int_type>(result.value()[0].p_type));
        REQUIRE(result.value()[1].name == identifier{ "hello2" });
        REQUIRE(std::holds_alternative<long_type>(result.value()[1].p_type));
        REQUIRE(result.value()[2].name == identifier{ "hello3" });
        REQUIRE(std::holds_alternative<long_type>(result.value()[2].p_type));
        REQUIRE(tokens.get_next_token().has_value() == false);
    }

    SECTION("Invalid Parameter")
    {
        std::vector<wccff::lexer::token> tokens_vector;
        tokens_vector.emplace_back(wccff::lexer::token_type::open_parenthesis, "(", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::void_keyword, "void", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "hello1", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::close_parenthesis, ")", location);

        wccff::parser::tokens tokens{ tokens_vector };
        auto result = wccff::parser::parse_params_list(tokens);
        REQUIRE(result.has_value() == false);
    }

    SECTION("Invalid comma")
    {
        std::vector<wccff::lexer::token> tokens_vector;
        tokens_vector.emplace_back(wccff::lexer::token_type::open_parenthesis, "(", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::int_keyword, "int", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "hello1", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::comma, ",", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::close_parenthesis, ")", location);

        wccff::parser::tokens tokens{ tokens_vector };
        auto result = wccff::parser::parse_params_list(tokens);
        REQUIRE(result.has_value() == false);
    }
}

TEST_CASE("parser_pretty_printers", "[parser]")
{
    using wccff::parser::pretty_print;
    auto directoryDisposer = ApprovalTests::Approvals::useApprovalsSubdirectory("parser_tests");

    SECTION("assignment_node")
    {
        using wccff::parser::assignment_node;
        using wccff::parser::assignment_operator;
        using wccff::parser::int_constant;
        using wccff::parser::var;

        auto value = int_constant{ 55 };
        auto variable = var{ "var_name" };

        auto assignment = std::make_unique<assignment_node>(assignment_operator{}, value, variable);

        ApprovalTests::Approvals::verify(pretty_print(assignment));
    }

    SECTION("binary_node")
    {
        using wccff::parser::binary_node;
        using wccff::parser::int_constant;
        using wccff::parser::logical_and_operator;
        using wccff::parser::var;

        auto value = int_constant{ 55 };
        auto variable = var{ "var_name" };

        auto assignment = std::make_unique<binary_node>(logical_and_operator{}, value, variable);

        ApprovalTests::Approvals::verify(pretty_print(assignment));
    }

    SECTION("binary_operators")
    {
        REQUIRE(pretty_print(wccff::parser::plus_operator{}) == "Plus");
        REQUIRE(pretty_print(wccff::parser::subtract_operator{}) == "Subtract");
        REQUIRE(pretty_print(wccff::parser::multiply_operator{}) == "Multiply");
        REQUIRE(pretty_print(wccff::parser::divide_operator{}) == "Divide");
        REQUIRE(pretty_print(wccff::parser::remainder_operator{}) == "Remainder");
        REQUIRE(pretty_print(wccff::parser::bitwise_and_operator{}) == "Bitwise And");
        REQUIRE(pretty_print(wccff::parser::bitwise_or_operator{}) == "Bitwise Or");
        REQUIRE(pretty_print(wccff::parser::bitwise_xor_operator{}) == "Bitwise Xor");
        REQUIRE(pretty_print(wccff::parser::left_shift_operator{}) == "Left Shift");
        REQUIRE(pretty_print(wccff::parser::right_shift_operator{}) == "Right Shift");
        REQUIRE(pretty_print(wccff::parser::logical_and_operator{}) == "Logic And");
        REQUIRE(pretty_print(wccff::parser::logical_or_operator{}) == "Logic Or");
        REQUIRE(pretty_print(wccff::parser::equals_operator{}) == "Equals");
        REQUIRE(pretty_print(wccff::parser::not_equals_operator{}) == "Not Equals");
        REQUIRE(pretty_print(wccff::parser::less_than_operator{}) == "Less Than");
        REQUIRE(pretty_print(wccff::parser::less_than_or_equal_operator{}) == "Less Than or Equals");
        REQUIRE(pretty_print(wccff::parser::greater_than_operator{}) == "Greater Than");
        REQUIRE(pretty_print(wccff::parser::greater_than_or_equal_operator{}) == "Greater Than or Equals");
    }

    SECTION("block")
    {
        using wccff::parser::block;
        using wccff::parser::identifier;
        using wccff::parser::int_constant;
        using wccff::parser::return_node;
        using wccff::parser::variable_declaration;
        std::vector<wccff::parser::block_item> items;

        identifier var_name{ "var_name" };
        int_constant value{ 55 };
        items.emplace_back(variable_declaration{ var_name, std::nullopt });
        items.emplace_back(return_node{ value });

        block b{ std::move(items) };

        ApprovalTests::Approvals::verify(pretty_print(b));
    }

    SECTION("break")
    {
        using wccff::parser::break_statement;
        using wccff::parser::identifier;
        identifier var_name{ "label_name" };

        break_statement b{ var_name };

        REQUIRE(pretty_print(b) == "Break(label_name)");
    }

    SECTION("cast_expression")
    {
        using wccff::parser::cast_expression;
        using wccff::parser::identifier;
        using wccff::parser::int_type;
        using wccff::parser::long_constant;
        using wccff::parser::long_type;
        using wccff::parser::var;
        using wccff::testing::get_binary_node;
        using wccff::testing::get_long_constant;
        using wccff::testing::get_unary_node;
        using wccff::testing::get_var;

        std::string result;
        result += pretty_print(std::make_unique<cast_expression>(int_type{}, get_long_constant()));
        result += pretty_print(std::make_unique<cast_expression>(long_type{}, get_var()));
        result += pretty_print(std::make_unique<cast_expression>(long_type{}, get_unary_node()));
        result += pretty_print(std::make_unique<cast_expression>(long_type{}, get_binary_node()));

        ApprovalTests::Approvals::verify(result);
    }
    SECTION("compound_statement")
    {
        using wccff::parser::block;
        using wccff::parser::compound_statement;
        using wccff::parser::identifier;
        using wccff::parser::int_constant;
        using wccff::parser::return_node;
        using wccff::parser::variable_declaration;
        std::vector<wccff::parser::block_item> items;

        identifier var_name{ "var_name" };
        int_constant value{ 55 };
        items.emplace_back(variable_declaration{ var_name, std::nullopt });
        items.emplace_back(return_node{ value });

        block b{ std::move(items) };
        auto stmt = std::make_unique<compound_statement>(std::move(b));

        ApprovalTests::Approvals::verify(pretty_print(stmt));
    }

    SECTION("continue")
    {
        using wccff::parser::continue_statement;
        using wccff::parser::identifier;
        identifier var_name{ "label_name" };

        continue_statement b{ var_name };

        REQUIRE(pretty_print(b) == "Continue(label_name)");
    }

    SECTION("do_while")
    {
        using wccff::parser::binary_node;
        using wccff::parser::do_while_statement;
        using wccff::parser::equals_operator;
        using wccff::parser::identifier;
        using wccff::parser::int_constant;
        using wccff::parser::return_node;

        auto loop_name = identifier("loop_name");
        auto body = return_node{ int_constant{ 42 } };
        auto conditional = std::make_unique<binary_node>(equals_operator{}, int_constant{ 1 }, int_constant{ 1 });

        auto stmt = std::make_unique<do_while_statement>(std::move(body), std::move(conditional), loop_name);
        ApprovalTests::Approvals::verify(pretty_print(stmt));
    }

    SECTION("function")
    {
        using wccff::parser::function;
        using wccff::parser::identifier;

        auto name = identifier("var_name");
        SECTION("without_items")
        {
            std::vector<wccff::parser::block_item> items;

            auto dec = function{ name, std::move(items) };
            ApprovalTests::Approvals::verify(pretty_print(dec));
        }
        SECTION("with_items")
        {
            wccff::parser::int_constant ret_value{ 32 };
            std::vector<wccff::parser::block_item> items;
            wccff::parser::return_node ret{ ret_value };
            items.emplace_back(std::move(ret));
            auto dec = function{ name, std::move(items) };
            ApprovalTests::Approvals::verify(pretty_print(dec));
        }
    }

    SECTION("for_loop")
    {
        using wccff::parser::binary_node;
        using wccff::parser::equals_operator;
        using wccff::parser::expression;
        using wccff::parser::for_init;
        using wccff::parser::for_statement;
        using wccff::parser::identifier;
        using wccff::parser::init_expression;
        using wccff::parser::int_constant;
        using wccff::parser::plus_operator;
        using wccff::parser::return_node;

        identifier loop_name{ "loop_name" };
        SECTION("without init")
        {
            for_init init = init_expression{ std::nullopt };
            auto conditional = std::make_unique<binary_node>(equals_operator{}, int_constant{ 1 }, int_constant{ 2 });
            auto post = std::make_unique<binary_node>(plus_operator{}, int_constant{ 3 }, int_constant{ 4 });
            auto body = return_node{ int_constant{ 42 } };

            auto stmt = std::make_unique<for_statement>(std::move(init),
                                                        std::move(conditional),
                                                        std::move(post),
                                                        std::move(body),
                                                        loop_name);
            ApprovalTests::Approvals::verify(pretty_print(stmt));
        }
        SECTION("without condition")
        {
            for_init init = init_expression{
                std::make_unique<binary_node>(equals_operator{}, int_constant{ 1 }, int_constant{ 2 })
            };
            auto conditional = std::nullopt;
            auto post = std::make_unique<binary_node>(plus_operator{}, int_constant{ 3 }, int_constant{ 4 });
            auto body = return_node{ int_constant{ 42 } };

            auto stmt = std::make_unique<for_statement>(std::move(init),
                                                        std::move(conditional),
                                                        std::move(post),
                                                        std::move(body),
                                                        loop_name);
            ApprovalTests::Approvals::verify(pretty_print(stmt));
        }
        SECTION("without post")
        {
            for_init init = init_expression{
                std::make_unique<binary_node>(equals_operator{}, int_constant{ 1 }, int_constant{ 2 })
            };
            auto conditional = std::make_unique<binary_node>(equals_operator{}, int_constant{ 1 }, int_constant{ 2 });
            auto post = std::nullopt;
            auto body = return_node{ int_constant{ 42 } };

            auto stmt = std::make_unique<for_statement>(std::move(init),
                                                        std::move(conditional),
                                                        std::move(post),
                                                        std::move(body),
                                                        loop_name);
            ApprovalTests::Approvals::verify(pretty_print(stmt));
        }
        SECTION("without all")
        {
            for_init init = init_expression{ std::nullopt };
            auto conditional = std::nullopt;
            auto post = std::nullopt;
            auto body = return_node{ int_constant{ 42 } };

            auto stmt = std::make_unique<for_statement>(std::move(init),
                                                        std::move(conditional),
                                                        std::move(post),
                                                        std::move(body),
                                                        loop_name);
            ApprovalTests::Approvals::verify(pretty_print(stmt));
        }
        SECTION("with all")
        {
            for_init init = init_expression{
                std::make_unique<binary_node>(equals_operator{}, int_constant{ 1 }, int_constant{ 2 })
            };
            auto conditional = std::make_unique<binary_node>(equals_operator{}, int_constant{ 1 }, int_constant{ 2 });
            auto post = std::make_unique<binary_node>(plus_operator{}, int_constant{ 3 }, int_constant{ 4 });
            auto body = return_node{ int_constant{ 42 } };

            auto stmt = std::make_unique<for_statement>(std::move(init),
                                                        std::move(conditional),
                                                        std::move(post),
                                                        std::move(body),
                                                        loop_name);
            ApprovalTests::Approvals::verify(pretty_print(stmt));
        }
    }

    SECTION("fun_type")
    {
        using wccff::parser::fun_type;
        using wccff::parser::int_type;
        using wccff::parser::long_type;
        using wccff::parser::type;
        using wccff::parser::void_type;

        std::string result;

        // A fun_type without parameters, Return type Int
        std::vector<type> params;
        params.emplace_back(void_type{});
        auto no_params = std::make_unique<fun_type>(std::move(params), int_type{});
        result = pretty_print(no_params);
        result += '\n';

        // A fun_type with one Int parameter, Return type Int
        params.emplace_back(int_type{});
        auto one_int_param = std::make_unique<fun_type>(std::move(params), int_type{});
        result += pretty_print(one_int_param);
        result += '\n';

        // A fun_type with one Long parameter, Return type Int
        params.emplace_back(long_type{});
        auto one_long_param = std::make_unique<fun_type>(std::move(params), int_type{});
        result += pretty_print(one_long_param);
        result += '\n';

        // A fun_type with one Long parameter, Return type Long
        params.emplace_back(long_type{});
        auto one_long_param_ret_long = std::make_unique<fun_type>(std::move(params), long_type{});
        result += pretty_print(one_long_param_ret_long);
        result += '\n';

        // A fun_type with two parameter, one Int and one Long, Return type Long
        params.emplace_back(int_type{});
        params.emplace_back(long_type{});
        auto two_params_ret_long = std::make_unique<fun_type>(std::move(params), long_type{});
        result += pretty_print(two_params_ret_long);
        result += '\n';

        ApprovalTests::Approvals::verify(result);
    }

    SECTION("function_declaration")
    {
        using wccff::parser::block;
        using wccff::parser::block_item;
        using wccff::parser::function_declaration;
        using wccff::parser::identifier;
        using wccff::parser::int_constant;
        using wccff::parser::int_type;
        using wccff::parser::long_type;
        using wccff::parser::return_node;
        using wccff::parser::void_type;
        using wccff::testing::get_block;
        using wccff::testing::get_function_type;
        using wccff::testing::get_identifier;
        using enum wccff::lexer::token_type;

        auto get_args_types = [](const std::vector<wccff::lexer::token_type> &types) {
            std::vector<wccff::parser::type> args;

            std::ranges::transform(types,
                                   std::back_inserter(args),
                                   [](wccff::lexer::token_type t) -> wccff::parser::type {
                                       switch (t)
                                       {
                                           case int_keyword:
                                               return int_type{};
                                           case long_keyword:
                                               return long_type{};
                                           case void_keyword:
                                               return void_type{};
                                           default:
                                               throw std::runtime_error("unexpected token type");
                                       }
                                   });
            return args;
        };

        std::string result;

        std::vector<wccff::parser::identifier> arguments;
        arguments.emplace_back(wccff::parser::identifier{ "param_1" });
        arguments.emplace_back(wccff::parser::identifier{ "param_2" });

        result += pretty_print(
          function_declaration{ get_identifier("function_name"),
                                arguments,
                                get_block(),
                                get_function_type(long_type{}, get_args_types({ int_keyword, long_keyword })),
                                wccff::parser::storage_class::no_storage });
        result += '\n';

        result += pretty_print(function_declaration{ get_identifier("function_name"),
                                                     {},
                                                     get_block(),
                                                     get_function_type(int_type{}, get_args_types({ void_keyword })),
                                                     wccff::parser::storage_class::extern_storage });
        result += '\n';

        result += pretty_print(function_declaration{ get_identifier("function_name"),
                                                     {},
                                                     std::nullopt,
                                                     get_function_type(int_type{}, get_args_types({ void_keyword })),
                                                     wccff::parser::storage_class::static_storage });
        result += '\n';

        ApprovalTests::Approvals::verify(result);
    }

    SECTION("goto")
    {
        using wccff::parser::goto_statement;
        REQUIRE(pretty_print(goto_statement{ "var_name" }) == "GoTo(var_name)");
        REQUIRE(pretty_print(goto_statement{ "var_name" }, 4) == "    GoTo(var_name)");
    }

    SECTION("identifier")
    {
        using wccff::parser::identifier;
        REQUIRE(pretty_print(identifier{ "var_name" }) == "var_name");
        REQUIRE(pretty_print(identifier{ "var_name" }, 4) == "    var_name");
    }

    SECTION("int_constant")
    {
        using wccff::parser::int_constant;
        REQUIRE(pretty_print(int_constant{ 55 }) == "IntConstant(55)");
        REQUIRE(pretty_print(int_constant{ 55 }, 4) == "    IntConstant(55)");
    }

    SECTION("labbeled_statement")
    {
        using wccff::parser::identifier;
        using wccff::parser::int_constant;
        using wccff::parser::labelled_statement;
        using wccff::parser::return_node;
        using wccff::parser::statement;

        return_node stmt{ int_constant{ 55 } };

        auto labelled = std::make_unique<labelled_statement>(identifier{ "label_name" }, std::move(stmt));
        ApprovalTests::Approvals::verify(pretty_print(labelled));
    }

    SECTION("return_node")
    {
        using wccff::parser::int_constant;
        using wccff::parser::return_node;

        auto ret = return_node{ int_constant{ 55 } };
        ApprovalTests::Approvals::verify(pretty_print(ret));
    }
    SECTION("storage_class")
    {
        using wccff::parser::storage_class;
        REQUIRE(pretty_print(storage_class::extern_storage) == "Storage(extern)");
        REQUIRE(pretty_print(storage_class::static_storage) == "Storage(static)");
        REQUIRE(pretty_print(storage_class::no_storage) == "Storage(none)");
    }

    SECTION("type")
    {
        SECTION("Int type")
        {
            REQUIRE(pretty_print(wccff::parser::int_type{}) == "Int");
            REQUIRE(pretty_print(wccff::parser::int_type{}, 4) == "    Int");
        }
        SECTION("Long type")
        {
            REQUIRE(pretty_print(wccff::parser::long_type{}) == "Long");
            REQUIRE(pretty_print(wccff::parser::long_type{}, 4) == "    Long");
        }
        SECTION("Void type")
        {
            REQUIRE(pretty_print(wccff::parser::void_type{}, 4) == "    Void");
        }
    }

    SECTION("unary_node")
    {
        using wccff::parser::int_constant;
        using wccff::parser::negate_operator;
        using wccff::parser::unary_node;
        using wccff::parser::var;

        auto value = int_constant{ 55 };
        auto variable = var{ "var_name" };

        auto assignment = std::make_unique<unary_node>(negate_operator{}, value);

        ApprovalTests::Approvals::verify(pretty_print(assignment));
    }
    SECTION("unary_operators")
    {
        REQUIRE(pretty_print(wccff::parser::bitwise_complement_operator{}) == "Complement");
        REQUIRE(pretty_print(wccff::parser::negate_operator{}) == "Negate");
        REQUIRE(pretty_print(wccff::parser::logical_not_operator{}) == "Not");
    }

    SECTION("var")
    {
        using wccff::parser::var;
        auto variable = var{ "var_name" };

        REQUIRE(pretty_print(variable) == "Var(var_name)");
    }

    SECTION("variable_declaration")
    {
        using wccff::parser::identifier;
        using wccff::parser::int_type;
        using wccff::parser::long_type;
        using wccff::parser::storage_class;
        using wccff::parser::variable_declaration;
        using wccff::testing::get_identifier;
        using wccff::testing::get_int_constant;
        using wccff::testing::get_long_constant;

        std::string result;

        result = pretty_print(
          variable_declaration(get_identifier("var_name"), get_int_constant(), int_type{}, storage_class::no_storage));
        result += '\n';
        result += pretty_print(variable_declaration(get_identifier("var_name"),
                                                    get_long_constant(),
                                                    long_type{},
                                                    storage_class::extern_storage));
        result += '\n';
        result += pretty_print(
          variable_declaration(get_identifier("var_name"), std::nullopt, long_type{}, storage_class::static_storage));

        ApprovalTests::Approvals::verify(result);
    }

    SECTION("while")
    {
        using wccff::parser::binary_node;
        using wccff::parser::equals_operator;
        using wccff::parser::identifier;
        using wccff::parser::int_constant;
        using wccff::parser::return_node;
        using wccff::parser::while_statement;

        auto loop_name = identifier("loop_name");
        auto body = return_node{ int_constant{ 42 } };
        auto conditional = std::make_unique<binary_node>(equals_operator{}, int_constant{ 1 }, int_constant{ 1 });

        auto stmt = std::make_unique<while_statement>(std::move(conditional), std::move(body), loop_name);
        ApprovalTests::Approvals::verify(pretty_print(stmt));
    }
}
