/*
 * Will Compile C for Food, a toy C compiler
 * Copyright (C) 2024  João Pires
 * https://github.com/jpires/will-compile-c-for-food
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "../semantic_analysis.h"
#include "semantic_analysis/identifier_resolution.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Variable Resolution", "[semantic_analysis]")
{
    using namespace wccff;

    wccff::sema::variable_map map;
    using wccff::parser::assignment_operator;
    SECTION("resolve_assignment_node")
    {
        parser::int_constant value1{ 42 };
        parser::int_constant value2{ 43 };

        auto original = std::make_unique<parser::assignment_node>(assignment_operator{}, value1, value2);
        auto result = sema::variable_resolution::process_assignment_node(original, map);
        REQUIRE(result.has_value() == false);
    }
}
