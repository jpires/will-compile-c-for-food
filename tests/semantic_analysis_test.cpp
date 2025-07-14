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
#include "semantic_analysis/labelled_statements.h"
#include "semantic_analysis/loop_labelling.h"
#include "semantic_analysis/type_checker.h"
#include <ApprovalTests.hpp>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Variable Resolution", "[semantic_analysis]")
{
    auto directoryDisposer = ApprovalTests::Approvals::useApprovalsSubdirectory("variable_resolution_results");
    using namespace wccff;

    std::filesystem::directory_iterator dir("tests/test_files/");
    for (const auto &entry : std::filesystem::directory_iterator(dir))
    {
        DYNAMIC_SECTION(fmt::format("{}", entry.path().filename().string()))
        {
            auto file = lexer::read_file(entry.path());
            REQUIRE(file.has_value());

            auto lexer_out = lexer::lexer(file.value());
            REQUIRE(lexer_out.has_value());

            auto tokens = parser::tokens{ lexer_out.value() };
            auto ast = parser::parse(tokens);
            REQUIRE(ast.has_value());

            sema::identifier_map variable_map;
            auto sema_result = sema::variable_resolution::process_program(ast.value(), variable_map);
            REQUIRE(sema_result.has_value());
            ApprovalTests::Approvals::verify(pretty_print(sema_result.value()));
        }
    }
}

TEST_CASE("Type Checker", "[semantic_analysis]")
{
    auto directoryDisposer = ApprovalTests::Approvals::useApprovalsSubdirectory("type_check_results");
    using namespace wccff;

    std::filesystem::directory_iterator dir("tests/test_files/");
    for (const auto &entry : std::filesystem::directory_iterator(dir))
    {
        DYNAMIC_SECTION(fmt::format("{}", entry.path().filename().string()))
        {
            auto file = lexer::read_file(entry.path());
            REQUIRE(file.has_value());

            auto lexer_out = lexer::lexer(file.value());
            REQUIRE(lexer_out.has_value());

            auto tokens = parser::tokens{ lexer_out.value() };
            auto ast = parser::parse(tokens);
            REQUIRE(ast.has_value());

            sema::identifier_map variable_map;
            auto var_resolv__result = sema::variable_resolution::process_program(ast.value(), variable_map);
            REQUIRE(var_resolv__result.has_value());

            symbol_table::symbol_table table;
            auto sema_result = sema::type_checker::process_program(var_resolv__result.value(), table);
            REQUIRE(sema_result.has_value());
            ApprovalTests::Approvals::verify(pretty_print(sema_result.value()));
        }
    }
}

TEST_CASE("Labelled Statements", "[semantic_analysis]")
{
    auto directoryDisposer = ApprovalTests::Approvals::useApprovalsSubdirectory("labelled_statements_results");
    using namespace wccff;

    std::filesystem::directory_iterator dir("tests/test_files/");
    for (const auto &entry : std::filesystem::directory_iterator(dir))
    {
        DYNAMIC_SECTION(fmt::format("{}", entry.path().filename().string()))
        {
            auto file = lexer::read_file(entry.path());
            REQUIRE(file.has_value());

            auto lexer_out = lexer::lexer(file.value());
            REQUIRE(lexer_out.has_value());

            auto tokens = parser::tokens{ lexer_out.value() };
            auto ast = parser::parse(tokens);
            REQUIRE(ast.has_value());

            symbol_table::symbol_table table;
            auto sema_result = sema::labelled_statements::process_program(ast.value());
            REQUIRE(sema_result.has_value());
            ApprovalTests::Approvals::verify(pretty_print(sema_result.value()));
        }
    }
}

TEST_CASE("Loop Labelling", "[semantic_analysis]")
{
    auto directoryDisposer = ApprovalTests::Approvals::useApprovalsSubdirectory("loop_labelling_results");
    using namespace wccff;

    std::filesystem::directory_iterator dir("tests/test_files/");
    for (const auto &entry : std::filesystem::directory_iterator(dir))
    {
        DYNAMIC_SECTION(fmt::format("{}", entry.path().filename().string()))
        {
            auto file = lexer::read_file(entry.path());
            REQUIRE(file.has_value());

            auto lexer_out = lexer::lexer(file.value());
            REQUIRE(lexer_out.has_value());

            auto tokens = parser::tokens{ lexer_out.value() };
            auto ast = parser::parse(tokens);
            REQUIRE(ast.has_value());

            symbol_table::symbol_table table;
            auto sema_result = sema::loop_labelling::process_program(ast.value());
            REQUIRE(sema_result.has_value());
            ApprovalTests::Approvals::verify(pretty_print(sema_result.value()));
        }
    }
}
