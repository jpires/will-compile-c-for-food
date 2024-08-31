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
    }
}
