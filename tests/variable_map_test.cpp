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
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Variable Map", "[variable_map]")
{
    using wccff::parser::identifier;

    wccff::sema::variable_map vm;

    SECTION("Empty Map")
    {
        REQUIRE(vm.contains(identifier("dont_exist")) == false);
        REQUIRE(vm.contains(identifier("dont_exist"), wccff::sema::variable_map::scopes::current_scope) == false);
    }

    SECTION("One entry")
    {
        REQUIRE(vm.contains(identifier("entry_one")) == false);
        auto unique_name = vm.add(identifier("entry_one"));
        REQUIRE(vm.contains(identifier("entry_one")));
        REQUIRE(vm.get_unique_name(identifier("entry_one")).name == unique_name.name);
    }

    SECTION("Two entries")
    {
        REQUIRE(vm.contains(identifier("entry_one")) == false);
        auto unique_name = vm.add(identifier("entry_one"));
        REQUIRE(vm.contains(identifier("entry_one")));
        REQUIRE(vm.get_unique_name(identifier("entry_one")).name == unique_name.name);

        REQUIRE(vm.contains(identifier("entry_two")) == false);
        auto unique_name2 = vm.add(identifier("entry_two"));
        REQUIRE(vm.contains(identifier("entry_two")));
        REQUIRE(vm.get_unique_name(identifier("entry_two")).name == unique_name2.name);
    }

    SECTION("Scopes")
    {
        REQUIRE(vm.contains(identifier("entry_one")) == false);
        auto unique_name = vm.add(identifier("entry_one"));
        REQUIRE(vm.contains(identifier("entry_one")));
        REQUIRE(vm.get_unique_name(identifier("entry_one")).name == unique_name.name);

        vm.create_scope();
        REQUIRE(vm.contains(identifier("entry_one")) == true);
        REQUIRE(vm.contains(identifier("entry_one"), wccff::sema::variable_map::scopes::current_scope) == false);
        auto unique_name1 = vm.add(identifier("entry_one"));
        REQUIRE(vm.contains(identifier("entry_one")));
        REQUIRE(vm.contains(identifier("entry_one"), wccff::sema::variable_map::scopes::current_scope));
        REQUIRE(vm.get_unique_name(identifier("entry_one")).name == unique_name1.name);

        vm.destroy_scope();
        REQUIRE(vm.contains(identifier("entry_one")));
        REQUIRE(vm.contains(identifier("entry_one"), wccff::sema::variable_map::scopes::current_scope));
        REQUIRE(vm.get_unique_name(identifier("entry_one")).name == unique_name.name);
    }
}