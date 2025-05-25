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

#ifndef PARSER_HELPERS_H
#define PARSER_HELPERS_H

#include "parser.h"
namespace wccff::testing {

template<typename Type>
constexpr bool is_constant_expression_of(const wccff::parser::expression &exp, int64_t value)
{
    if (std::holds_alternative<wccff::constant>(exp) == false)
    {
        return false;
    }

    auto &inner = std::get<wccff::constant>(exp);
    if (std::holds_alternative<Type>(inner) == false)
    {
        return false;
    }
    return std::get<Type>(inner).value == value;
}

} // namespace wccff::testing

#endif // PARSER_HELPERS_H
