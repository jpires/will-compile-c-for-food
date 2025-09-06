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

#include "semantic_analysis.h"
#include "semantic_analysis/identifier_resolution.h"
#include "semantic_analysis/labelled_statements.h"
#include "semantic_analysis/loop_labelling.h"
#include "semantic_analysis/type_checker.h"
#include "visitor.h"

#include <algorithm>
#include <ranges>

namespace wccff::sema {

static std::unexpected<semantic_error> generate_unknown_variable(identifier var_name)
{
    auto msg = fmt::format("Unknown variable '{}'", var_name);
    return std::unexpected<semantic_error>(msg);
}

identifier identifier_map::add(const identifier &name, linkage link)
{
    auto unique_name = link == linkage::internal ? generate_unique_name(name) : name;
    symbol s{ name, unique_name, link };

    m_map.back().insert({ name.name, s });

    return unique_name;
}

void identifier_map::create_scope()
{
    m_map.emplace_back();
    m_scope_counter++;
}

void identifier_map::destroy_scope()
{
    m_map.pop_back();
    m_scope_counter--;
}

std::optional<identifier_map::symbol> identifier_map::find(const identifier &name, scopes on_current_scope) const
{
    // Lambda to check if an identifier with the expected linkage exists in a scope.
    auto f = [&name](const std::unordered_map<std::string, symbol> &map) -> std::optional<symbol> {
        if (auto it = map.find(name.name); it != map.end())
        {
            return it->second;
        }

        return std::nullopt;
    };

    // In this situation, we want to get the symbol in the current scope.
    // So, we only need to check the latest entry, which is the inner most scope
    if (on_current_scope == scopes::current_scope)
    {
        return f(m_map.back());
    }

    auto reverse_map = std::ranges::reverse_view{ m_map };
    auto a = std::ranges::find_if(reverse_map, [&name](const auto &map) { return map.contains(name.name); });
    if (a == reverse_map.end())
    {
        return std::nullopt;
    }
    return f(*a);

    // We want to look at all scopes, and stop when we find the first entry.
    // return std::ranges::any_of(m_map, [&](const auto &map) { return f(map); });
}

identifier identifier_map::generate_unique_name(const identifier &name)
{
    return { fmt::format("var.{}.{}", name.name, m_counter++) };
}

std::expected<std::tuple<parser::program, symbol_table::symbol_table>, semantic_error> analyse(
  const parser::program &input)
{
    identifier_map variable_map;
    auto var_result = variable_resolution::process_program(input, variable_map);
    if (var_result.has_value() == false)
    {
        return std::unexpected{ var_result.error() };
    }

    symbol_table::symbol_table symbol_table;
    auto type_check_result = type_checker::process_program(var_result.value(), symbol_table);
    if (type_check_result.has_value() == false)
    {
        return std::unexpected{ type_check_result.error() };
    }

    auto labelled_result = labelled_statements::process_program(type_check_result.value());
    if (labelled_result.has_value() == false)
    {
        return std::unexpected{ labelled_result.error() };
    }

    auto loop_labelling_result = loop_labelling::process_program(labelled_result.value());
    if (loop_labelling_result.has_value() == false)
    {
        return std::unexpected{ loop_labelling_result.error() };
    }

    return std::make_tuple(std::move(loop_labelling_result.value()), symbol_table);
}

std::expected<parser::expression, semantic_error> convert_by_assignment(const parser::expression &exp,
                                                                        const type &target)
{
    if (get_type(exp) == target)
    {
        return parser::copy_expression(exp);
    }

    if (is_arithmetic(get_type(exp)) && is_arithmetic(target))
    {
        return convert_to(exp, target);
    }

    if (is_null_pointer_constant(exp) && is_pointer(target))
    {
        return convert_to(exp, target);
    }

    auto msg = fmt::format("Cannot convert type for assignment");
    return std::unexpected{ semantic_error{ msg } };
}

bool is_lvalue(const parser::expression &e)
{
    return std::visit(visitor{
                        [&](const std::unique_ptr<parser::dereference> &n) { return true; },
                        [&](const parser::var &n) { return true; },
                        [&](const constant &n) { return false; },
                        [&](const auto &n) { return false; },
                      },
                      e);
}

bool is_null_pointer_constant(const wccff::constant &exp)
{
    return std::visit(visitor{
                        [](const wccff::int_constant &n) { return n.value ? false : true; },
                        [](const wccff::long_constant &n) { return n.value ? false : true; },
                        [](const wccff::unsigned_int_constant &n) { return n.value ? false : true; },
                        [](const wccff::unsigned_long_constant &n) { return n.value ? false : true; },
                        [](const auto &) { return false; },
                      },
                      exp);
}

bool is_null_pointer_constant(const wccff::parser::expression &exp)
{
    return std::visit(visitor{
                        [](const wccff::constant &n) { return is_null_pointer_constant(n); },
                        [](const auto &) { return false; },
                      },
                      exp);
}

std::expected<type, semantic_error> get_common_pointer_type(const parser::expression &e1, const parser::expression &e2)
{
    auto e1_t = get_type(e1);
    auto e2_t = get_type(e2);
    if (e1_t == e2_t)
    {
        return e1_t;
    }
    if (is_null_pointer_constant(e1))
    {
        return e2_t;
    }
    if (is_null_pointer_constant(e2))
    {
        return e1_t;
    }

    auto msg = fmt::format("Expression has incompatible types.");
    return std::unexpected{ semantic_error{ msg } };
}

} // namespace wccff::sema
