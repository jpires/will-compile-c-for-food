#include "../parser.h"
#include "../tacky.h"
#include "parser_data.h"
#include <ApprovalTests.hpp>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Tacky", "[tacky]")
{
    using wccff::identifier;
    auto directoryDisposer = ApprovalTests::Approvals::useApprovalsSubdirectory("tacky_results");

    wccff::symbol_table::symbol_table table;

    SECTION("Constant")
    {
        wccff::int_constant c{ 42 };

        auto result = wccff::tacky::process_constant(c);
        REQUIRE(std::holds_alternative<wccff::int_constant>(result));
        REQUIRE(std::get<wccff::int_constant>(result).value == 42);
    }

    SECTION("process_unary_operator")
    {
        using namespace wccff;
        using wccff::bitwise_complement_operator;
        using wccff::negate_operator;
        using wccff::tacky::process_unary_operator;

        REQUIRE(
          std::holds_alternative<bitwise_complement_operator>(process_unary_operator(bitwise_complement_operator{})));
        REQUIRE(std::holds_alternative<logical_not_operator>(process_unary_operator(logical_not_operator{})));
        REQUIRE(std::holds_alternative<negate_operator>(process_unary_operator(negate_operator{})));
        REQUIRE_THROWS(process_unary_operator(postfix_decrement_operator{}));
        REQUIRE_THROWS(process_unary_operator(postfix_increment_operator{}));
        REQUIRE_THROWS(process_unary_operator(prefix_decrement_operator{}));
        REQUIRE_THROWS(process_unary_operator(prefix_increment_operator{}));
    }
    SECTION("process_binary_operator")
    {
        using namespace wccff;
        using wccff::tacky::process_binary_operator;

        REQUIRE(std::holds_alternative<plus_operator>(process_binary_operator(wccff::plus_operator{})));
        REQUIRE(std::holds_alternative<subtract_operator>(process_binary_operator(wccff::subtract_operator{})));
        REQUIRE(std::holds_alternative<multiply_operator>(process_binary_operator(wccff::multiply_operator{})));
        REQUIRE(std::holds_alternative<divide_operator>(process_binary_operator(wccff::divide_operator{})));
        REQUIRE(std::holds_alternative<remainder_operator>(process_binary_operator(wccff::remainder_operator{})));
    }

    SECTION("process_unary_node")
    {
        using wccff::parser::unary_node;
        using wccff::tacky::process_unary_node;

        std::vector<wccff::tacky::instruction> instructions;
        auto node = std::make_unique<unary_node>(wccff::bitwise_complement_operator{},
                                                 wccff::testing::get_int_constant(42),
                                                 wccff::int_type{});
        auto result = process_unary_node(node, instructions, table);
        REQUIRE(std::holds_alternative<wccff::tacky::plain_operand>(result));
        // REQUIRE(std::get<wccff::tacky::var>(result).id.name == "tacky-1");
        REQUIRE(table.get(identifier{ "tacky-1" }).has_value());
        REQUIRE(table.get(identifier{ "tacky-1" })->type == wccff::int_type{});

        auto pretty_result = wccff::tacky::pretty_print(instructions);
        pretty_result += "\n";

        instructions.clear();
        node = std::make_unique<unary_node>(wccff::logical_not_operator{},
                                            wccff::testing::get_long_constant(42),
                                            wccff::long_type{});
        result = process_unary_node(node, instructions, table);
        REQUIRE(std::holds_alternative<wccff::tacky::plain_operand>(result));
        // REQUIRE(std::get<wccff::tacky::var>(result).id.name == "tacky-2");
        REQUIRE(table.get({ "tacky-2" }).has_value());
        REQUIRE(table.get({ "tacky-2" })->type == wccff::long_type{});

        pretty_result += wccff::tacky::pretty_print(instructions);
        pretty_result += "\n";

        instructions.clear();
        node = std::make_unique<unary_node>(wccff::negate_operator{}, wccff::testing::get_var(), wccff::long_type{});
        result = process_unary_node(node, instructions, table);
        REQUIRE(std::holds_alternative<wccff::tacky::plain_operand>(result));
        // REQUIRE(std::get<wccff::tacky::var>(result).id.name == "tacky-3");
        REQUIRE(table.get(identifier{ "tacky-3" }).has_value());
        REQUIRE(table.get(identifier{ "tacky-3" })->type == wccff::long_type{});

        pretty_result += wccff::tacky::pretty_print(instructions);
        pretty_result += "\n";

        ApprovalTests::Approvals::verify(pretty_result);
    }

    SECTION("process_binary_node")
    {
        using wccff::parser::binary_node;
        using wccff::tacky::process_binary_node;
        using wccff::testing::get_int_constant;
        using wccff::testing::get_long_constant;

        std::vector<wccff::tacky::instruction> instructions;

        auto binary_expr = std::make_unique<binary_node>(wccff::plus_operator{},
                                                         get_int_constant(42),
                                                         get_int_constant(24),
                                                         wccff::int_type{});

        auto result = process_binary_node(binary_expr, instructions, table);
        REQUIRE(std::holds_alternative<wccff::tacky::plain_operand>(result));
        // REQUIRE(std::get<wccff::tacky::var>(result).id.name == "tacky-4");
        REQUIRE(table.get(identifier{ "tacky-4" }).has_value());
        REQUIRE(table.get(identifier{ "tacky-4" })->type == wccff::int_type{});
        auto pretty_result = wccff::tacky::pretty_print(instructions);
        pretty_result += "\n";

        instructions.clear();
        binary_expr = std::make_unique<binary_node>(wccff::bitwise_and_operator{},
                                                    get_int_constant(1),
                                                    get_int_constant(2),
                                                    wccff::int_type{});

        result = process_binary_node(binary_expr, instructions, table);
        REQUIRE(std::holds_alternative<wccff::tacky::plain_operand>(result));
        // REQUIRE(std::get<wccff::tacky::var>(result).id.name == "tacky-5");
        REQUIRE(table.get(identifier{ "tacky-5" }).has_value());
        REQUIRE(table.get(identifier{ "tacky-5" })->type == wccff::int_type{});

        pretty_result += wccff::tacky::pretty_print(instructions);
        pretty_result += "\n";

        instructions.clear();
        binary_expr = std::make_unique<binary_node>(wccff::bitwise_or_operator{},
                                                    get_long_constant(3),
                                                    get_int_constant(4),
                                                    wccff::long_type{});

        result = process_binary_node(binary_expr, instructions, table);
        REQUIRE(std::holds_alternative<wccff::tacky::plain_operand>(result));
        // REQUIRE(std::get<wccff::tacky::var>(result).id.name == "tacky-6");
        REQUIRE(table.get(identifier{ "tacky-6" }).has_value());
        REQUIRE(table.get(identifier{ "tacky-6" })->type == wccff::long_type{});

        pretty_result += wccff::tacky::pretty_print(instructions);
        pretty_result += "\n";

        instructions.clear();
        binary_expr = std::make_unique<binary_node>(wccff::bitwise_xor_operator{},
                                                    get_int_constant(5),
                                                    get_long_constant(6),
                                                    wccff::long_type{});

        result = process_binary_node(binary_expr, instructions, table);
        REQUIRE(std::holds_alternative<wccff::tacky::plain_operand>(result));
        // REQUIRE(std::get<wccff::tacky::var>(result).id.name == "tacky-7");
        REQUIRE(table.get(identifier{ "tacky-7" }).has_value());
        REQUIRE(table.get(identifier{ "tacky-7" })->type == wccff::long_type{});

        pretty_result += wccff::tacky::pretty_print(instructions);
        pretty_result += "\n";

        instructions.clear();
        binary_expr = std::make_unique<binary_node>(wccff::left_shift_operator{},
                                                    get_long_constant(7),
                                                    get_long_constant(8),
                                                    wccff::long_type{});

        result = process_binary_node(binary_expr, instructions, table);
        REQUIRE(std::holds_alternative<wccff::tacky::plain_operand>(result));
        // REQUIRE(std::get<wccff::tacky::var>(result).id.name == "tacky-8");
        REQUIRE(table.get(identifier{ "tacky-8" }).has_value());
        REQUIRE(table.get(identifier{ "tacky-8" })->type == wccff::long_type{});

        pretty_result += wccff::tacky::pretty_print(instructions);
        pretty_result += "\n";

        instructions.clear();
        binary_expr = std::make_unique<binary_node>(wccff::right_shift_operator{},
                                                    get_long_constant(7),
                                                    get_long_constant(8),
                                                    wccff::long_type{});

        result = process_binary_node(binary_expr, instructions, table);
        REQUIRE(std::holds_alternative<wccff::tacky::plain_operand>(result));
        // REQUIRE(std::get<wccff::tacky::var>(result).id.name == "tacky-9");
        REQUIRE(table.get(identifier{ "tacky-9" }).has_value());
        REQUIRE(table.get(identifier{ "tacky-9" })->type == wccff::long_type{});

        pretty_result += wccff::tacky::pretty_print(instructions);
        pretty_result += "\n";

        instructions.clear();
        binary_expr = std::make_unique<binary_node>(wccff::equals_operator{},
                                                    get_long_constant(9),
                                                    get_long_constant(10),
                                                    wccff::long_type{});

        result = process_binary_node(binary_expr, instructions, table);
        REQUIRE(std::holds_alternative<wccff::tacky::plain_operand>(result));
        // REQUIRE(std::get<wccff::tacky::var>(result).id.name == "tacky-10");
        REQUIRE(table.get(identifier{ "tacky-10" }).has_value());
        REQUIRE(table.get(identifier{ "tacky-10" })->type == wccff::long_type{});

        pretty_result += wccff::tacky::pretty_print(instructions);
        pretty_result += "\n";

        instructions.clear();
        binary_expr = std::make_unique<binary_node>(wccff::not_equals_operator{},
                                                    get_long_constant(11),
                                                    get_long_constant(12),
                                                    wccff::long_type{});

        result = process_binary_node(binary_expr, instructions, table);
        REQUIRE(std::holds_alternative<wccff::tacky::plain_operand>(result));
        // REQUIRE(std::get<wccff::tacky::var>(result).id.name == "tacky-11");
        REQUIRE(table.get(identifier{ "tacky-11" }).has_value());
        REQUIRE(table.get(identifier{ "tacky-11" })->type == wccff::long_type{});

        pretty_result += wccff::tacky::pretty_print(instructions);
        pretty_result += "\n";

        instructions.clear();
        binary_expr = std::make_unique<binary_node>(wccff::less_than_operator{},
                                                    get_long_constant(13),
                                                    get_long_constant(14),
                                                    wccff::long_type{});

        result = process_binary_node(binary_expr, instructions, table);
        REQUIRE(std::holds_alternative<wccff::tacky::plain_operand>(result));
        // REQUIRE(std::get<wccff::tacky::var>(result).id.name == "tacky-12");
        REQUIRE(table.get(identifier{ "tacky-12" }).has_value());
        REQUIRE(table.get(identifier{ "tacky-12" })->type == wccff::long_type{});

        pretty_result += wccff::tacky::pretty_print(instructions);
        pretty_result += "\n";

        instructions.clear();
        binary_expr = std::make_unique<binary_node>(wccff::less_than_or_equal_operator{},
                                                    get_long_constant(15),
                                                    get_long_constant(16),
                                                    wccff::long_type{});

        result = process_binary_node(binary_expr, instructions, table);
        REQUIRE(std::holds_alternative<wccff::tacky::plain_operand>(result));
        // REQUIRE(std::get<wccff::tacky::var>(result).id.name == "tacky-13");
        REQUIRE(table.get(identifier{ "tacky-13" }).has_value());
        REQUIRE(table.get(identifier{ "tacky-13" })->type == wccff::long_type{});

        pretty_result += wccff::tacky::pretty_print(instructions);
        pretty_result += "\n";

        instructions.clear();
        binary_expr = std::make_unique<binary_node>(wccff::greater_than_operator{},
                                                    get_long_constant(17),
                                                    get_long_constant(18),
                                                    wccff::long_type{});

        result = process_binary_node(binary_expr, instructions, table);
        REQUIRE(std::holds_alternative<wccff::tacky::plain_operand>(result));
        // REQUIRE(std::get<wccff::tacky::var>(result).id.name == "tacky-14");
        REQUIRE(table.get(identifier{ "tacky-14" }).has_value());
        REQUIRE(table.get(identifier{ "tacky-14" })->type == wccff::long_type{});

        pretty_result += wccff::tacky::pretty_print(instructions);
        pretty_result += "\n";

        instructions.clear();
        binary_expr = std::make_unique<binary_node>(wccff::greater_than_or_equal_operator{},
                                                    get_long_constant(19),
                                                    get_long_constant(20),
                                                    wccff::long_type{});

        result = process_binary_node(binary_expr, instructions, table);
        REQUIRE(std::holds_alternative<wccff::tacky::plain_operand>(result));
        // REQUIRE(std::get<wccff::tacky::var>(result).id.name == "tacky-15");
        REQUIRE(table.get(identifier{ "tacky-15" }).has_value());
        REQUIRE(table.get(identifier{ "tacky-15" })->type == wccff::long_type{});

        pretty_result += wccff::tacky::pretty_print(instructions);
        pretty_result += "\n";

        ApprovalTests::Approvals::verify(pretty_result);
    }
}
