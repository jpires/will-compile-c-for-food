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

#include "parser_helpers.h"

#include "parser_data.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("is_constant_expression_of")
{
    auto int_const = wccff::testing::get_int_constant(55);
    REQUIRE(wccff::testing::is_constant_expression_of<wccff::parser::int_constant>(int_const, 55));
    REQUIRE(wccff::testing::is_constant_expression_of<wccff::parser::long_constant>(int_const, 55) == false);
    REQUIRE(wccff::testing::is_constant_expression_of<wccff::parser::int_constant>(int_const, 43) == false);

    auto long_const = wccff::testing::get_long_constant(66);
    REQUIRE(wccff::testing::is_constant_expression_of<wccff::parser::long_constant>(long_const, 66));
    REQUIRE(wccff::testing::is_constant_expression_of<wccff::parser::int_constant>(long_const, 66) == false);
    REQUIRE(wccff::testing::is_constant_expression_of<wccff::parser::long_constant>(long_const, 43) == false);
}
