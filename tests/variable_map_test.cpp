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
#include <unicode-db/unicode_interface.hpp>
#include <unistd.h>

TEST_CASE("Variable Map", "[variable_map]")
{
    using wccff::identifier;

    wccff::sema::identifier_map vm;

    SECTION("Empty Map")
    {
        REQUIRE(vm.find(identifier("dont_exist")) == std::nullopt);
        REQUIRE(vm.find(identifier("dont_exist"), wccff::sema::identifier_map::scopes::current_scope) == std::nullopt);
    }

    SECTION("One entry")
    {
        auto name = identifier("entry_one");
        REQUIRE(vm.find(name) == std::nullopt);
        auto unique_name = vm.add(name);
        REQUIRE(vm.find(name).has_value());
        auto s = vm.find(name).value();
        REQUIRE(s.name == name);
        REQUIRE(s.unique_name == unique_name);
        REQUIRE(s.linkage == wccff::sema::identifier_map::linkage::internal);
    }

    SECTION("Two entries")
    {
        auto name1 = identifier("entry_one");
        REQUIRE(vm.find(name1) == std::nullopt);
        auto unique_name = vm.add(name1);
        REQUIRE(vm.find(name1).has_value());
        auto s1 = vm.find(name1).value();
        REQUIRE(s1.name == name1);
        REQUIRE(s1.unique_name == unique_name);
        REQUIRE(s1.linkage == wccff::sema::identifier_map::linkage::internal);

        auto name2 = identifier("entry_two");
        REQUIRE(vm.find(name2) == std::nullopt);
        auto unique_name2 = vm.add(name2);
        REQUIRE(vm.find(name2).has_value());
        auto s2 = vm.find(name2).value();
        REQUIRE(s2.name == name2);
        REQUIRE(s2.unique_name == unique_name2);
        REQUIRE(s2.linkage == wccff::sema::identifier_map::linkage::internal);
    }

    SECTION("Scopes")
    {
        auto name = identifier("entry_one");
        REQUIRE(vm.find(name) == std::nullopt);
        auto unique_name = vm.add(name);
        REQUIRE(vm.find(name).has_value());

        vm.create_scope();
        REQUIRE(vm.find(name).has_value());
        REQUIRE(vm.find(name, wccff::sema::identifier_map::scopes::current_scope) == std::nullopt);
        auto unique_name1 = vm.add(name);
        REQUIRE(vm.find(name).has_value());
        REQUIRE(vm.find(name, wccff::sema::identifier_map::scopes::current_scope).has_value());
        auto s2 = vm.find(name);
        REQUIRE(s2->name == name);
        REQUIRE(s2->unique_name == unique_name1);

        auto s3 = vm.find(name, wccff::sema::identifier_map::scopes::current_scope);
        REQUIRE(s3->name == name);
        REQUIRE(s3->unique_name == unique_name1);

        vm.destroy_scope();
        REQUIRE(vm.find(name).has_value());
        REQUIRE(vm.find(name, wccff::sema::identifier_map::scopes::current_scope).has_value());
    }

    SECTION("External Linkage")
    {
        SECTION("Add identifier")
        {
            auto name = identifier("entry_one");
            auto unique_name = vm.add(name, wccff::sema::identifier_map::linkage::external);
            REQUIRE(vm.find(name).has_value());
            auto s = vm.find(name).value();
            REQUIRE(s.name == name);
            REQUIRE(s.unique_name == name);
            REQUIRE(s.unique_name == unique_name);
            REQUIRE(s.linkage == wccff::sema::identifier_map::linkage::external);
            REQUIRE(unique_name == name);
        }
    }
}
