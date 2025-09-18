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

#ifndef WILL_COMPILE_C_FOR_FOOD_ASSEMBLY_GENERATION_HELPERS_H
#define WILL_COMPILE_C_FOR_FOOD_ASSEMBLY_GENERATION_HELPERS_H
#include <utility>

#include "assembly_generation.h"
namespace wccff::testing {

inline wccff::assembly_generation::data get_dst_data(wccff::identifier name = wccff::identifier{ "data_dst" })
{
    return { std::move(name) };
}

inline wccff::assembly_generation::immediate get_dst_immediate(int64_t value = 55)
{
    return { value };
}

inline wccff::assembly_generation::memory get_dst_memory(
  const wccff::assembly_generation::reg &base = wccff::assembly_generation::BP{},
  int64_t offset = 16)
{
    return { .base = base, .offset = offset };
}

inline wccff::assembly_generation::pseudo get_dst_pseudo(wccff::identifier name = wccff::identifier{ "pseudo_dst" })
{
    return { std::move(name) };
}

inline wccff::assembly_generation::reg get_dst_reg(
  const wccff::assembly_generation::reg &base = wccff::assembly_generation::R10{})
{
    return base;
}

inline wccff::assembly_generation::memory get_lhs_memory(
  const wccff::assembly_generation::reg &base = wccff::assembly_generation::BP{},
  int64_t offset = 8)
{
    return { .base = base, .offset = offset };
}

inline wccff::assembly_generation::memory get_rhs_memory(
  const wccff::assembly_generation::reg &base = wccff::assembly_generation::BP{},
  int64_t offset = 16)
{
    return { .base = base, .offset = offset };
}

inline wccff::assembly_generation::data get_src_data(wccff::identifier name = wccff::identifier{ "data_src" })
{
    return { std::move(name) };
}

inline wccff::assembly_generation::immediate get_src_immediate(int64_t value = 42)
{
    return { value };
}

inline wccff::assembly_generation::memory get_src_memory(
  const wccff::assembly_generation::reg &base = wccff::assembly_generation::BP{},
  int64_t offset = 8)
{
    return { .base = base, .offset = offset };
}

inline wccff::assembly_generation::pseudo get_src_pseudo(wccff::identifier name = wccff::identifier{ "pseudo_src" })
{
    return { std::move(name) };
}

inline wccff::assembly_generation::reg get_src_reg(
  const wccff::assembly_generation::reg &base = wccff::assembly_generation::R11{})
{
    return base;
}
} // namespace wccff::testing
#endif // WILL_COMPILE_C_FOR_FOOD_ASSEMBLY_GENERATION_HELPERS_H
