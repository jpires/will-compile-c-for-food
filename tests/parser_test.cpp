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
        REQUIRE(r->f.body.size() == 1);
        auto &block_item1 = r->f.body.at(0);
        REQUIRE(std::holds_alternative<wccff::parser::statement>(block_item1));
        auto &stmt1 = std::get<wccff::parser::statement>(block_item1);
        REQUIRE(std::holds_alternative<wccff::parser::return_node>(stmt1));
        auto &ret_node = std::get<wccff::parser::return_node>(stmt1);
        REQUIRE(std::holds_alternative<wccff::parser::int_constant>(ret_node.e));
        REQUIRE(std::get<wccff::parser::int_constant>(ret_node.e).value == 2);
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

TEST_CASE("parser_pretty_printers", "[parser]")
{
    using wccff::parser::pretty_print;
    auto directoryDisposer = ApprovalTests::Approvals::useApprovalsSubdirectory("parser_tests");

    SECTION("assignment_node")
    {
        using wccff::parser::assignment_node;
        using wccff::parser::int_constant;
        using wccff::parser::var;

        auto value = int_constant{ 55 };
        auto variable = var{ "var_name" };

        auto assignment = std::make_unique<assignment_node>(value, variable);

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

    SECTION("block_items")
    {
        using wccff::parser::declaration;
        using wccff::parser::identifier;
        using wccff::parser::int_constant;
        using wccff::parser::return_node;
        std::vector<wccff::parser::block_item> items;

        identifier var_name{ "var_name" };
        int_constant value{ 55 };
        items.emplace_back(declaration{ var_name, std::nullopt });
        items.emplace_back(return_node{ value });

        ApprovalTests::Approvals::verify(pretty_print(items));
    }

    SECTION("declaration")
    {
        using wccff::parser::declaration;
        using wccff::parser::identifier;

        auto name = identifier("var_name");
        SECTION("with_init")
        {
            wccff::parser::int_constant init{ 42 };
            auto dec = declaration(name, init);
            ApprovalTests::Approvals::verify(pretty_print(dec));
        }
        SECTION("without_init")
        {
            auto dec = declaration(name, std::nullopt);
            ApprovalTests::Approvals::verify(pretty_print(dec));
        }
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
}
