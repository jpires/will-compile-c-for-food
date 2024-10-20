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
#include "semantic_analysis/loop_labelling.h"
#include "semantic_analysis/variable_resolution.h"
#include "visitor.h"

#include <algorithm>
#include <ranges>

namespace wccff::sema {

static std::unexpected<semantic_error> generate_unknown_variable(parser::identifier var_name)
{
    auto msg = fmt::format("Unknown variable '{}'", var_name.name);
    return std::unexpected<semantic_error>(msg);
}

parser::identifier variable_map::add(const parser::identifier &name)
{
    auto unique_name = generate_unique_name(name);
    symbol s{ name.name, unique_name.name };

    m_map.back().insert({ name.name, s });

    return unique_name;
}
bool variable_map::contains(const parser::identifier &name, scopes on_current_scope) const
{
    // In this situation, we want to know if this variable exists in the current scope.
    // So, we only need to check the latest entry.
    if (on_current_scope == scopes::current_scope)
    {
        return m_map.back().contains(name.name);
    }

    // We want to look at all scopes, going in reverse order as they were declared.
    // i.e. a variable that is declared in two scope, then we want the innermost definition
    // of that variable.
    return std::ranges::any_of(m_map, [&](const auto &pair) { return pair.contains(name.name); });
}
void variable_map::create_scope()
{
    m_map.emplace_back();
    m_scope_counter++;
}
void variable_map::destroy_scope()
{
    m_map.pop_back();
    m_scope_counter--;
}
parser::identifier variable_map::get_unique_name(const parser::identifier &name) const
{
    for (auto const &pair : std::ranges::reverse_view(m_map))
    {
        if (pair.contains(name.name))
        {
            return { pair.at(name.name).unique_name };
        }
    }
    throw std::runtime_error("variable map does not exist");
}
parser::identifier variable_map::generate_unique_name(const parser::identifier &name)
{
    return { fmt::format("var-{}-{}", name.name, m_counter++) };
}

std::expected<parser::program, semantic_error> analyse(const parser::program &input)
{
    variable_map variable_map;
    auto var_result = variable_resolution::process_program(input, variable_map);
    if (var_result.has_value() == false)
    {
        return std::unexpected{ var_result.error() };
    }

    return loop_labelling::process_program(var_result.value());
}

bool is_lvalue(const std::unique_ptr<parser::binary_node> &e)
{
    return false;
}
bool is_lvalue(const std::unique_ptr<parser::unary_node> &e)
{
    return is_lvalue(e->exp);
}
bool is_lvalue(const parser::expression &e)
{
    return std::visit(visitor{
                        [&](const std::unique_ptr<parser::assignment_node> &n) { return false; },
                        [&](const std::unique_ptr<parser::conditional_node> &n) { return false; },
                        [&](const std::unique_ptr<parser::binary_node> &n) { return is_lvalue(n); },
                        [&](const std::unique_ptr<parser::unary_node> &n) { return is_lvalue(n); },
                        [&](const parser::var &n) { return true; },
                        [&](const parser::int_constant &n) { return false; },
                      },
                      e);
}

}