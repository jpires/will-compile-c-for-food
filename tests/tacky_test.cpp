#include "../parser.h"
#include "../tacky.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Tacky", "[tacky]")
{
    SECTION("Identifier")
    {
        wccff::parser::identifier id{ "foo" };

        auto result = wccff::tacky::process_identifier(id);
        REQUIRE(result.name == "foo");
    }

    SECTION("Constant")
    {
        wccff::parser::int_constant c{ 42 };

        auto result = wccff::tacky::process_int_constant(c);
        REQUIRE(result.value == 42);
    }

    SECTION("Constant")
    {
        SECTION("Negate Operator")
        {
            wccff::parser::negate_operator c;

            auto result = wccff::tacky::process_unary_operator(c);
            REQUIRE(std::holds_alternative<wccff::tacky::negate_operator>(result));
        }
        SECTION("Bitwise Complement Operator")
        {
            wccff::parser::bitwise_complement_operator c;

            auto result = wccff::tacky::process_unary_operator(c);
            REQUIRE(std::holds_alternative<wccff::tacky::binary_complement_operator>(result));
        }
    }
    SECTION("Unary Node")
    {
        auto inner_expression = wccff::parser::int_constant{ 42 };
        auto node = std::make_unique<wccff::parser::unary_node>(wccff::parser::negate_operator{}, inner_expression);

        std::vector<wccff::tacky::instruction> instructions;
        auto result = wccff::tacky::process_unary_node(node, instructions);

        REQUIRE(instructions.size() == 1);
        REQUIRE(std::holds_alternative<wccff::tacky::unary_statement>(instructions.at(0)));
        auto instruction = std::get<wccff::tacky::unary_statement>(instructions.at(0));
        REQUIRE(std::holds_alternative<wccff::tacky::negate_operator>(instruction.op));
        REQUIRE(std::holds_alternative<wccff::tacky::constant>(instruction.src));
        REQUIRE(std::get<wccff::tacky::constant>(instruction.src).value == 42);
        REQUIRE(std::holds_alternative<wccff::tacky::var>(instruction.dst));
        REQUIRE(std::get<wccff::tacky::var>(instruction.dst).id.name == "tacky-1");
    }
}

TEST_CASE("process_binary_operator", "[tacky]")
{
    using wccff::tacky::process_binary_operator;
    using namespace wccff;

    REQUIRE(std::holds_alternative<tacky::plus_operator>(process_binary_operator(parser::plus_operator{})));
    REQUIRE(std::holds_alternative<tacky::subtract_operator>(process_binary_operator(parser::subtract_operator{})));
    REQUIRE(std::holds_alternative<tacky::multiply_operator>(process_binary_operator(parser::multiply_operator{})));
    REQUIRE(std::holds_alternative<tacky::divide_operator>(process_binary_operator(parser::divide_operator{})));
    REQUIRE(std::holds_alternative<tacky::remainder_operator>(process_binary_operator(parser::remainder_operator{})));
}

TEST_CASE("process_binary_node", "[tacky]")
{
    using namespace wccff;

    SECTION("Plus Operator")
    {
        auto left = wccff::parser::int_constant{ 42 };
        auto right = wccff::parser::int_constant{ 24 };
        auto binary_expr = std::make_unique<parser::binary_node>(parser::plus_operator{}, left, right);

        std::vector<wccff::tacky::instruction> instructions;
        auto result = tacky::process_binary_node(binary_expr, instructions);

        REQUIRE(instructions.size() == 1);
        REQUIRE(std::holds_alternative<tacky::binary_statement>(instructions.at(0)));
        auto inst = std::get<tacky::binary_statement>(instructions.at(0));

        REQUIRE(std::holds_alternative<tacky::plus_operator>(inst.op));
        REQUIRE(std::holds_alternative<tacky::constant>(inst.src1));
        REQUIRE(std::get<tacky::constant>(inst.src1).value == 42);
        REQUIRE(std::get<tacky::constant>(inst.src2).value == 24);
        REQUIRE(std::holds_alternative<tacky::var>(inst.dst));
    }

    SECTION("Plus Operator")
    {
        auto left = wccff::parser::int_constant{ 1 };
        auto right = wccff::parser::int_constant{ 2 };
        auto binary_expr = std::make_unique<parser::binary_node>(parser::bitwise_and_operator{}, left, right);

        std::vector<wccff::tacky::instruction> instructions;
        auto result = tacky::process_binary_node(binary_expr, instructions);

        REQUIRE(instructions.size() == 1);
        REQUIRE(std::holds_alternative<tacky::binary_statement>(instructions.at(0)));
        auto inst = std::get<tacky::binary_statement>(instructions.at(0));

        REQUIRE(std::holds_alternative<tacky::binary_and_operator>(inst.op));
        REQUIRE(std::holds_alternative<tacky::constant>(inst.src1));
        REQUIRE(std::get<tacky::constant>(inst.src1).value == 1);
        REQUIRE(std::get<tacky::constant>(inst.src2).value == 2);
        REQUIRE(std::holds_alternative<tacky::var>(inst.dst));
    }
}
