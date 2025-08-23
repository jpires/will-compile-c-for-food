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

#include "parser.h"
#include <utility>

namespace wccff::testing {

constexpr identifier get_identifier(std::string name = "foo");

constexpr wccff::parser::param get_param(identifier value = get_identifier(), type type = void_type{});

constexpr identifier get_identifier(std::string name)
{
    return identifier{ std::move(name) };
}

constexpr wccff::int_constant get_int_constant(int32_t value = 42)
{
    return wccff::int_constant{ value };
}

constexpr wccff::long_constant get_long_constant(int64_t value = 42)
{
    return wccff::long_constant{ value };
}

constexpr std::unique_ptr<wccff::parser::binary_node> get_binary_node(
  wccff::binary_operator op = wccff::plus_operator{},
  wccff::parser::expression exp1 = get_int_constant(),
  wccff::parser::expression exp2 = get_int_constant())
{
    return std::make_unique<wccff::parser::binary_node>(op, std::move(exp1), std::move(exp2));
}

constexpr wccff::parser::block get_block()
{
    std::vector<wccff::parser::block_item> items;
    items.emplace_back(wccff::parser::return_node{ wccff::int_constant{ 42 } });
    return { std::move(items) };
}

constexpr wccff::type get_function_type(wccff::type ret_type = wccff::int_type{}, std::vector<wccff::type> params = {})
{
    return std::make_unique<wccff::fun_type>(std::move(params), std::move(ret_type));
}

constexpr std::unique_ptr<wccff::parser::unary_node> get_unary_node(wccff::unary_operator op = wccff::negate_operator{},
                                                                    wccff::parser::expression exp = get_int_constant())
{
    return std::make_unique<wccff::parser::unary_node>(op, std::move(exp));
}

constexpr wccff::parser::var get_var(identifier value = get_identifier())
{
    return wccff::parser::var{ std::move(value) };
}

constexpr wccff::parser::param get_param(identifier value, wccff::type type)
{
    return wccff::parser::param{ std::move(value), std::move(type) };
}
} // namespace wccff::testing
