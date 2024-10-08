#include "../assembly_generation.h"
#include "../parser.h"
#include "../tacky.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Binary Operations", "[assembly_generation]")
{
    SECTION("binary_and")
    {
        wccff::tacky::constant src1{ 1 };
        wccff::tacky::constant src2{ 2 };
        wccff::tacky::var dst{ "tacky-1" };
        wccff::tacky::binary_statement stmt{ wccff::tacky::binary_and_operator{}, src1, src2, dst };

        auto instructions = wccff::assembly_generation::process_statement(stmt);
        REQUIRE(instructions.size() == 2);
        REQUIRE(std::holds_alternative<wccff::assembly_generation::mov_instruction>(instructions.at(0)));
        auto inst1 = std::get<wccff::assembly_generation::mov_instruction>(instructions.at(0));
        REQUIRE(std::holds_alternative<wccff::assembly_generation::immediate>(inst1.src));
        REQUIRE(std::get<wccff::assembly_generation::immediate>(inst1.src).value == 1);
        REQUIRE(std::holds_alternative<wccff::assembly_generation::pseudo>(inst1.dst));
        REQUIRE(std::get<wccff::assembly_generation::pseudo>(inst1.dst).name.name == "tacky-1");

        REQUIRE(std::holds_alternative<wccff::assembly_generation::binary>(instructions.at(1)));
        auto inst2 = std::get<wccff::assembly_generation::binary>(instructions.at(1));
        REQUIRE(std::holds_alternative<wccff::assembly_generation::binary_and>(inst2.op));
        REQUIRE(std::holds_alternative<wccff::assembly_generation::immediate>(inst2.src));
        REQUIRE(std::get<wccff::assembly_generation::immediate>(inst2.src).value == 2);
        REQUIRE(std::holds_alternative<wccff::assembly_generation::pseudo>(inst2.dst));
        REQUIRE(std::get<wccff::assembly_generation::pseudo>(inst2.dst).name.name == "tacky-1");
    }

    SECTION("binary_or")
    {
        wccff::tacky::constant src1{ 1 };
        wccff::tacky::constant src2{ 2 };
        wccff::tacky::var dst{ "tacky-1" };
        wccff::tacky::binary_statement stmt{ wccff::tacky::binary_or_operator{}, src1, src2, dst };

        auto instructions = wccff::assembly_generation::process_statement(stmt);
        REQUIRE(instructions.size() == 2);
        REQUIRE(std::holds_alternative<wccff::assembly_generation::mov_instruction>(instructions.at(0)));
        auto inst1 = std::get<wccff::assembly_generation::mov_instruction>(instructions.at(0));
        REQUIRE(std::holds_alternative<wccff::assembly_generation::immediate>(inst1.src));
        REQUIRE(std::get<wccff::assembly_generation::immediate>(inst1.src).value == 1);
        REQUIRE(std::holds_alternative<wccff::assembly_generation::pseudo>(inst1.dst));
        REQUIRE(std::get<wccff::assembly_generation::pseudo>(inst1.dst).name.name == "tacky-1");

        REQUIRE(std::holds_alternative<wccff::assembly_generation::binary>(instructions.at(1)));
        auto inst2 = std::get<wccff::assembly_generation::binary>(instructions.at(1));
        REQUIRE(std::holds_alternative<wccff::assembly_generation::binary_or>(inst2.op));
        REQUIRE(std::holds_alternative<wccff::assembly_generation::immediate>(inst2.src));
        REQUIRE(std::get<wccff::assembly_generation::immediate>(inst2.src).value == 2);
        REQUIRE(std::holds_alternative<wccff::assembly_generation::pseudo>(inst2.dst));
        REQUIRE(std::get<wccff::assembly_generation::pseudo>(inst2.dst).name.name == "tacky-1");
    }

    SECTION("binary_xor")
    {
        wccff::tacky::constant src1{ 1 };
        wccff::tacky::constant src2{ 2 };
        wccff::tacky::var dst{ "tacky-1" };
        wccff::tacky::binary_statement stmt{ wccff::tacky::binary_xor_operator{}, src1, src2, dst };

        auto instructions = wccff::assembly_generation::process_statement(stmt);
        REQUIRE(instructions.size() == 2);
        REQUIRE(std::holds_alternative<wccff::assembly_generation::mov_instruction>(instructions.at(0)));
        auto inst1 = std::get<wccff::assembly_generation::mov_instruction>(instructions.at(0));
        REQUIRE(std::holds_alternative<wccff::assembly_generation::immediate>(inst1.src));
        REQUIRE(std::get<wccff::assembly_generation::immediate>(inst1.src).value == 1);
        REQUIRE(std::holds_alternative<wccff::assembly_generation::pseudo>(inst1.dst));
        REQUIRE(std::get<wccff::assembly_generation::pseudo>(inst1.dst).name.name == "tacky-1");

        REQUIRE(std::holds_alternative<wccff::assembly_generation::binary>(instructions.at(1)));
        auto inst2 = std::get<wccff::assembly_generation::binary>(instructions.at(1));
        REQUIRE(std::holds_alternative<wccff::assembly_generation::binary_xor>(inst2.op));
        REQUIRE(std::holds_alternative<wccff::assembly_generation::immediate>(inst2.src));
        REQUIRE(std::get<wccff::assembly_generation::immediate>(inst2.src).value == 2);
        REQUIRE(std::holds_alternative<wccff::assembly_generation::pseudo>(inst2.dst));
        REQUIRE(std::get<wccff::assembly_generation::pseudo>(inst2.dst).name.name == "tacky-1");
    }

    SECTION("left_shift")
    {
        wccff::tacky::constant src1{ 1 };
        wccff::tacky::constant src2{ 2 };
        wccff::tacky::var dst{ "tacky-1" };
        wccff::tacky::binary_statement stmt{ wccff::tacky::left_shift_operator{}, src1, src2, dst };

        auto instructions = wccff::assembly_generation::process_statement(stmt);
        REQUIRE(instructions.size() == 2);
        REQUIRE(std::holds_alternative<wccff::assembly_generation::mov_instruction>(instructions.at(0)));
        auto inst1 = std::get<wccff::assembly_generation::mov_instruction>(instructions.at(0));
        REQUIRE(std::holds_alternative<wccff::assembly_generation::immediate>(inst1.src));
        REQUIRE(std::get<wccff::assembly_generation::immediate>(inst1.src).value == 1);
        REQUIRE(std::holds_alternative<wccff::assembly_generation::pseudo>(inst1.dst));
        REQUIRE(std::get<wccff::assembly_generation::pseudo>(inst1.dst).name.name == "tacky-1");

        REQUIRE(std::holds_alternative<wccff::assembly_generation::binary>(instructions.at(1)));
        auto inst2 = std::get<wccff::assembly_generation::binary>(instructions.at(1));
        REQUIRE(std::holds_alternative<wccff::assembly_generation::left_shift>(inst2.op));
        REQUIRE(std::holds_alternative<wccff::assembly_generation::immediate>(inst2.src));
        REQUIRE(std::get<wccff::assembly_generation::immediate>(inst2.src).value == 2);
        REQUIRE(std::holds_alternative<wccff::assembly_generation::pseudo>(inst2.dst));
        REQUIRE(std::get<wccff::assembly_generation::pseudo>(inst2.dst).name.name == "tacky-1");
    }

    SECTION("right_shift")
    {
        wccff::tacky::constant src1{ 1 };
        wccff::tacky::constant src2{ 2 };
        wccff::tacky::var dst{ "tacky-1" };
        wccff::tacky::binary_statement stmt{ wccff::tacky::right_shift_operator{}, src1, src2, dst };

        auto instructions = wccff::assembly_generation::process_statement(stmt);
        REQUIRE(instructions.size() == 2);
        REQUIRE(std::holds_alternative<wccff::assembly_generation::mov_instruction>(instructions.at(0)));
        auto inst1 = std::get<wccff::assembly_generation::mov_instruction>(instructions.at(0));
        REQUIRE(std::holds_alternative<wccff::assembly_generation::immediate>(inst1.src));
        REQUIRE(std::get<wccff::assembly_generation::immediate>(inst1.src).value == 1);
        REQUIRE(std::holds_alternative<wccff::assembly_generation::pseudo>(inst1.dst));
        REQUIRE(std::get<wccff::assembly_generation::pseudo>(inst1.dst).name.name == "tacky-1");

        REQUIRE(std::holds_alternative<wccff::assembly_generation::binary>(instructions.at(1)));
        auto inst2 = std::get<wccff::assembly_generation::binary>(instructions.at(1));
        REQUIRE(std::holds_alternative<wccff::assembly_generation::right_shift>(inst2.op));
        REQUIRE(std::holds_alternative<wccff::assembly_generation::immediate>(inst2.src));
        REQUIRE(std::get<wccff::assembly_generation::immediate>(inst2.src).value == 2);
        REQUIRE(std::holds_alternative<wccff::assembly_generation::pseudo>(inst2.dst));
        REQUIRE(std::get<wccff::assembly_generation::pseudo>(inst2.dst).name.name == "tacky-1");
    }

    SECTION("equal_operator")
    {
        using namespace wccff;
        tacky::constant src1{ 1 };
        tacky::constant src2{ 2 };
        tacky::var dst{ "tacky-1" };
        tacky::binary_statement stmt{ tacky::equal_operator{}, src1, src2, dst };
        auto instructions = assembly_generation::process_statement(stmt);
        REQUIRE(instructions.size() == 3);

        REQUIRE(std::holds_alternative<assembly_generation::cmp>(instructions.at(0)));
        auto inst1 = std::get<assembly_generation::cmp>(instructions.at(0));
        REQUIRE(std::holds_alternative<assembly_generation::immediate>(inst1.lhs));
        REQUIRE(std::get<assembly_generation::immediate>(inst1.lhs).value == 2);
        REQUIRE(std::holds_alternative<assembly_generation::immediate>(inst1.rhs));
        REQUIRE(std::get<assembly_generation::immediate>(inst1.rhs).value == 1);

        REQUIRE(std::holds_alternative<assembly_generation::mov_instruction>(instructions.at(1)));
        auto inst2 = std::get<assembly_generation::mov_instruction>(instructions.at(1));
        REQUIRE(std::holds_alternative<assembly_generation::immediate>(inst2.src));
        REQUIRE(std::get<assembly_generation::immediate>(inst2.src).value == 0);
        REQUIRE(std::holds_alternative<assembly_generation::pseudo>(inst2.dst));
        REQUIRE(std::get<assembly_generation::pseudo>(inst2.dst).name.name == "tacky-1");

        REQUIRE(std::holds_alternative<assembly_generation::setcc>(instructions.at(2)));
        auto inst3 = std::get<assembly_generation::setcc>(instructions.at(2));
        REQUIRE(std::holds_alternative<assembly_generation::E>(inst3.cond));
        REQUIRE(std::get<assembly_generation::pseudo>(inst3.dst).name.name == "tacky-1");
    }
}

TEST_CASE("Unary Operations", "[assembly_generation]")
{
    using namespace wccff;
    SECTION("not_operator")
    {
        tacky::constant src1{ 1 };
        tacky::var dst{ "tacky-1" };
        tacky::unary_statement stmt{ tacky::not_operator{}, src1, dst };
        auto instructions = assembly_generation::process_statement(stmt);
        REQUIRE(instructions.size() == 3);

        REQUIRE(std::holds_alternative<assembly_generation::cmp>(instructions.at(0)));
        auto inst1 = std::get<assembly_generation::cmp>(instructions.at(0));
        REQUIRE(std::holds_alternative<assembly_generation::immediate>(inst1.lhs));
        REQUIRE(std::get<assembly_generation::immediate>(inst1.lhs).value == 0);
        REQUIRE(std::holds_alternative<assembly_generation::immediate>(inst1.rhs));
        REQUIRE(std::get<assembly_generation::immediate>(inst1.rhs).value == 1);

        REQUIRE(std::holds_alternative<assembly_generation::mov_instruction>(instructions.at(1)));
        auto inst2 = std::get<assembly_generation::mov_instruction>(instructions.at(1));
        REQUIRE(std::holds_alternative<assembly_generation::immediate>(inst2.src));
        REQUIRE(std::get<assembly_generation::immediate>(inst2.src).value == 0);
        REQUIRE(std::holds_alternative<assembly_generation::pseudo>(inst2.dst));
        REQUIRE(std::get<assembly_generation::pseudo>(inst2.dst).name.name == "tacky-1");

        REQUIRE(std::holds_alternative<assembly_generation::setcc>(instructions.at(2)));
        auto inst3 = std::get<assembly_generation::setcc>(instructions.at(2));
        REQUIRE(std::holds_alternative<assembly_generation::E>(inst3.cond));
        REQUIRE(std::get<assembly_generation::pseudo>(inst3.dst).name.name == "tacky-1");
    }
}
