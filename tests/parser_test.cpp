#include "../parser.h"
#include "../tacky.h"

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
        tokens_vector.emplace_back(wccff::lexer::token_type::constant, "2", location);
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

    SECTION("Bitwise Or Operator")
    {
        wccff::lexer::file_location location{ 1, 2 };
        std::vector<wccff::lexer::token> tokens_vector;
        tokens_vector.emplace_back(wccff::lexer::token_type::constant, "1", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::bitwise_or_operator, "|", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::constant, "2", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::semicolon, ";", location);

        wccff::parser::tokens tokens{ tokens_vector };
        auto r = wccff::parser::parse_expression(tokens);
        REQUIRE(r.has_value());
        REQUIRE(std::holds_alternative<std::unique_ptr<wccff::parser::binary_node>>(r.value()));
        const auto &exp = std::get<std::unique_ptr<wccff::parser::binary_node>>(r.value());

        REQUIRE(std::holds_alternative<wccff::parser::bitwise_or_operator>(exp->op));
        REQUIRE(std::holds_alternative<wccff::parser::int_constant>(exp->left));
        REQUIRE(std::get<wccff::parser::int_constant>(exp->left).value == 1);
        REQUIRE(std::holds_alternative<wccff::parser::int_constant>(exp->right));
        REQUIRE(std::get<wccff::parser::int_constant>(exp->right).value == 2);
    }
    SECTION("Bitwise Xor Operator")
    {
        wccff::lexer::file_location location{ 1, 2 };
        std::vector<wccff::lexer::token> tokens_vector;
        tokens_vector.emplace_back(wccff::lexer::token_type::constant, "1", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::bitwise_xor_operator, "^", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::constant, "2", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::semicolon, ";", location);

        wccff::parser::tokens tokens{ tokens_vector };
        auto r = wccff::parser::parse_expression(tokens);
        REQUIRE(r.has_value());
        REQUIRE(std::holds_alternative<std::unique_ptr<wccff::parser::binary_node>>(r.value()));
        const auto &exp = std::get<std::unique_ptr<wccff::parser::binary_node>>(r.value());

        REQUIRE(std::holds_alternative<wccff::parser::bitwise_xor_operator>(exp->op));
        REQUIRE(std::holds_alternative<wccff::parser::int_constant>(exp->left));
        REQUIRE(std::get<wccff::parser::int_constant>(exp->left).value == 1);
        REQUIRE(std::holds_alternative<wccff::parser::int_constant>(exp->right));
        REQUIRE(std::get<wccff::parser::int_constant>(exp->right).value == 2);
    }
    SECTION("Left Shift Operator")
    {
        wccff::lexer::file_location location{ 1, 2 };
        std::vector<wccff::lexer::token> tokens_vector;
        tokens_vector.emplace_back(wccff::lexer::token_type::constant, "1", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::left_shift_operator, "<<", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::constant, "2", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::semicolon, ";", location);

        wccff::parser::tokens tokens{ tokens_vector };
        auto r = wccff::parser::parse_expression(tokens);
        REQUIRE(r.has_value());
        REQUIRE(std::holds_alternative<std::unique_ptr<wccff::parser::binary_node>>(r.value()));
        const auto &exp = std::get<std::unique_ptr<wccff::parser::binary_node>>(r.value());

        REQUIRE(std::holds_alternative<wccff::parser::left_shift_operator>(exp->op));
        REQUIRE(std::holds_alternative<wccff::parser::int_constant>(exp->left));
        REQUIRE(std::get<wccff::parser::int_constant>(exp->left).value == 1);
        REQUIRE(std::holds_alternative<wccff::parser::int_constant>(exp->right));
        REQUIRE(std::get<wccff::parser::int_constant>(exp->right).value == 2);
    }
    SECTION("Right Shift Operator")
    {
        wccff::lexer::file_location location{ 1, 2 };
        std::vector<wccff::lexer::token> tokens_vector;
        tokens_vector.emplace_back(wccff::lexer::token_type::constant, "1", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::right_shift_operator, ">>", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::constant, "2", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::semicolon, ";", location);

        wccff::parser::tokens tokens{ tokens_vector };
        auto r = wccff::parser::parse_expression(tokens);
        REQUIRE(r.has_value());
        REQUIRE(std::holds_alternative<std::unique_ptr<wccff::parser::binary_node>>(r.value()));
        const auto &exp = std::get<std::unique_ptr<wccff::parser::binary_node>>(r.value());

        REQUIRE(std::holds_alternative<wccff::parser::right_shift_operator>(exp->op));
        REQUIRE(std::holds_alternative<wccff::parser::int_constant>(exp->left));
        REQUIRE(std::get<wccff::parser::int_constant>(exp->left).value == 1);
        REQUIRE(std::holds_alternative<wccff::parser::int_constant>(exp->right));
        REQUIRE(std::get<wccff::parser::int_constant>(exp->right).value == 2);
    }

    SECTION("Single Assignment")
    {
        wccff::lexer::file_location location{ 1, 3 };
        std::vector<wccff::lexer::token> tokens_vector;

        tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "a", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::assignment_operator, "=", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::constant, "2", location);
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
        tokens_vector.emplace_back(wccff::lexer::token_type::constant, "2", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::semicolon, ";", location);

        wccff::parser::tokens tokens{ tokens_vector };
        auto result = wccff::parser::parse_expression(tokens);
        REQUIRE(result.has_value());
        ApprovalTests::Approvals::verify(wccff::parser::pretty_print(result.value()));
    }
}

TEST_CASE("Parse Statements")
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
        tokens_vector.emplace_back(wccff::lexer::token_type::constant, "42", location);
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

TEST_CASE("Loop Statements")
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
            tokens_vector.emplace_back(wccff::lexer::token_type::constant, "1", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::semicolon, ";", location);

            tokens_vector.emplace_back(wccff::lexer::token_type::while_keyword, "while", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::open_parenthesis, "(", location);

            tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "a", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::less_than_operator, "<", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::constant, "10", location);
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
                tokens_vector.emplace_back(wccff::lexer::token_type::constant, "0", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::semicolon, ";", location);

                tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "a", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::less_than_operator, "<", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::constant, "10", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::semicolon, ";", location);

                tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "a", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::compound_plus, "+=", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::constant, "1", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::close_parenthesis, ")", location);

                tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "i", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::compound_plus, "+=", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::constant, "1", location);
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
                tokens_vector.emplace_back(wccff::lexer::token_type::constant, "10", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::semicolon, ";", location);

                tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "a", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::compound_plus, "+=", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::constant, "1", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::close_parenthesis, ")", location);

                tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "i", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::compound_plus, "+=", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::constant, "1", location);
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
                tokens_vector.emplace_back(wccff::lexer::token_type::constant, "0", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::semicolon, ";", location);

                tokens_vector.emplace_back(wccff::lexer::token_type::semicolon, ";", location);

                tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "a", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::compound_plus, "+=", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::constant, "1", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::close_parenthesis, ")", location);

                tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "i", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::compound_plus, "+=", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::constant, "1", location);
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
                tokens_vector.emplace_back(wccff::lexer::token_type::constant, "0", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::semicolon, ";", location);

                tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "a", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::less_than_operator, "<", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::constant, "10", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::semicolon, ";", location);

                tokens_vector.emplace_back(wccff::lexer::token_type::close_parenthesis, ")", location);

                tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "i", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::compound_plus, "+=", location);
                tokens_vector.emplace_back(wccff::lexer::token_type::constant, "1", location);
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
            tokens_vector.emplace_back(wccff::lexer::token_type::constant, "10", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::close_parenthesis, ")", location);

            tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "i", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::compound_plus, "+=", location);
            tokens_vector.emplace_back(wccff::lexer::token_type::constant, "1", location);
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
        tokens_vector.emplace_back(wccff::lexer::token_type::constant, "42", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::close_parenthesis, ")", location);

        wccff::parser::tokens tokens{ tokens_vector };
        auto result = wccff::parser::parse_argument_list(tokens);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(std::holds_alternative<parser::int_constant>(result.value()[0]));
        REQUIRE(std::get<parser::int_constant>(result.value()[0]).value == 42);
    }

    SECTION("Three Argument")
    {
        std::vector<wccff::lexer::token> tokens_vector;
        tokens_vector.emplace_back(wccff::lexer::token_type::constant, "42", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::comma, ",", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::constant, "43", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::comma, ",", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::constant, "44", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::close_parenthesis, ")", location);

        wccff::parser::tokens tokens{ tokens_vector };
        auto result = wccff::parser::parse_argument_list(tokens);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 3);
        REQUIRE(std::holds_alternative<parser::int_constant>(result.value()[0]));
        REQUIRE(std::get<parser::int_constant>(result.value()[0]).value == 42);
        REQUIRE(std::holds_alternative<parser::int_constant>(result.value()[1]));
        REQUIRE(std::get<parser::int_constant>(result.value()[1]).value == 43);
        REQUIRE(std::holds_alternative<parser::int_constant>(result.value()[2]));
        REQUIRE(std::get<parser::int_constant>(result.value()[2]).value == 44);
    }

    SECTION("One Argument with complex expression")
    {
        std::vector<wccff::lexer::token> tokens_vector;
        tokens_vector.emplace_back(wccff::lexer::token_type::constant, "42", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::plus_operator, "+", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::constant, "43", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::close_parenthesis, ")", location);

        wccff::parser::tokens tokens{ tokens_vector };
        auto result = wccff::parser::parse_argument_list(tokens);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(std::holds_alternative<std::unique_ptr<parser::binary_node>>(result.value()[0]));
        auto &node = std::get<std::unique_ptr<parser::binary_node>>(result.value()[0]);
        REQUIRE(std::holds_alternative<parser::plus_operator>(node->op));
        REQUIRE(std::holds_alternative<parser::int_constant>(node->left));
        REQUIRE(std::get<parser::int_constant>(node->left).value == 42);
        REQUIRE(std::holds_alternative<parser::int_constant>(node->right));
        REQUIRE(std::get<parser::int_constant>(node->right).value == 43);
    }

    SECTION("Invalid Argument")
    {
        std::vector<wccff::lexer::token> tokens_vector;
        tokens_vector.emplace_back(wccff::lexer::token_type::int_keyword, "int", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "a", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::assignment_operator, "=", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::constant, "44", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::close_parenthesis, ")", location);

        wccff::parser::tokens tokens{ tokens_vector };
        auto result = wccff::parser::parse_argument_list(tokens);
        REQUIRE(result.has_value() == false);
    }

    SECTION("Invalid Comma")
    {
        std::vector<wccff::lexer::token> tokens_vector;
        tokens_vector.emplace_back(wccff::lexer::token_type::constant, "44", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::comma, ",", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::close_parenthesis, ")", location);

        wccff::parser::tokens tokens{ tokens_vector };
        auto result = wccff::parser::parse_argument_list(tokens);
        REQUIRE(result.has_value() == false);
    }
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
        tokens_vector.emplace_back(wccff::lexer::token_type::constant, "42", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::close_parenthesis, ")", location);

        wccff::parser::tokens tokens{ tokens_vector };
        auto result = wccff::parser::parse_function_call(tokens);
        REQUIRE(result.has_value());
        REQUIRE(result.value()->name.name == "func1");
        REQUIRE(result.value()->arguments.size() == 1);
        REQUIRE(std::holds_alternative<parser::int_constant>(result.value()->arguments.at(0)));
        REQUIRE(std::get<parser::int_constant>(result.value()->arguments.at(0)).value == 42);
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
        tokens_vector.emplace_back(wccff::lexer::token_type::constant, "42", location);
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
    auto directoryDisposer = ApprovalTests::Approvals::useApprovalsSubdirectory("parser_tests");

    wccff::lexer::file_location location{ 0, 0 };
    SECTION("No Arguments")
    {
        std::vector<wccff::lexer::token> tokens_vector;
        tokens_vector.emplace_back(wccff::lexer::token_type::close_parenthesis, ")", location);

        wccff::parser::tokens tokens{ tokens_vector };
        auto result = wccff::parser::parse_params_list(tokens);
        REQUIRE(result.has_value());
        REQUIRE(result.value().empty());
    }

    SECTION("One Argument")
    {
        std::vector<wccff::lexer::token> tokens_vector;
        tokens_vector.emplace_back(wccff::lexer::token_type::int_keyword, "int", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "hello", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::close_parenthesis, ")", location);

        wccff::parser::tokens tokens{ tokens_vector };
        auto result = wccff::parser::parse_params_list(tokens);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value()[0].name == "hello");
    }

    SECTION("Three Argument")
    {
        std::vector<wccff::lexer::token> tokens_vector;
        tokens_vector.emplace_back(wccff::lexer::token_type::int_keyword, "int", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "hello1", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::comma, ",", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::int_keyword, "int", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "hello2", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::comma, ",", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::int_keyword, "int", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::identifier, "hello3", location);
        tokens_vector.emplace_back(wccff::lexer::token_type::close_parenthesis, ")", location);

        wccff::parser::tokens tokens{ tokens_vector };
        auto result = wccff::parser::parse_params_list(tokens);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 3);
        REQUIRE(result.value()[0].name == "hello1");
        REQUIRE(result.value()[1].name == "hello2");
        REQUIRE(result.value()[2].name == "hello3");
    }

    SECTION("Invalid Parameter")
    {
        std::vector<wccff::lexer::token> tokens_vector;
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

    SECTION("declaration")
    {
        using wccff::parser::identifier;
        using wccff::parser::variable_declaration;

        auto name = identifier("var_name");
        SECTION("with_init")
        {
            wccff::parser::int_constant init{ 42 };
            auto dec = variable_declaration(name, init);
            ApprovalTests::Approvals::verify(pretty_print(dec));
        }
        SECTION("without_init")
        {
            auto dec = variable_declaration(name, std::nullopt);
            ApprovalTests::Approvals::verify(pretty_print(dec));
        }
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

    SECTION("function_declaration")
    {
        using wccff::parser::block;
        using wccff::parser::block_item;
        using wccff::parser::function_declaration;
        using wccff::parser::identifier;
        using wccff::parser::int_constant;
        using wccff::parser::return_node;

        auto function_name = identifier{ "function_name" };
        std::vector<identifier> arguments;
        arguments.emplace_back(identifier{ "param_1" });
        arguments.emplace_back(identifier{ "param_2" });

        std::vector<block_item> items;
        items.emplace_back(return_node{ int_constant{ 42 } });
        auto body = block{ std::move(items) };

        SECTION("Full declaration")
        {
            auto f = function_declaration{ function_name, arguments, std::move(body) };
            ApprovalTests::Approvals::verify(pretty_print(f));
        }

        SECTION("No Params")
        {
            auto f = function_declaration{ function_name, {}, std::move(body) };
            ApprovalTests::Approvals::verify(pretty_print(f));
        }

        SECTION("No Body")
        {
            auto f = function_declaration{ function_name, arguments, std::nullopt };
            ApprovalTests::Approvals::verify(pretty_print(f));
        }
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
