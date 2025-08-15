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

#include "symbol_table.h"
namespace wccff::symbol_table {
void backend_symbol_table::build(const symbol_table &table)
{
    for (const auto &entry : table)
    {
        const auto &type = entry.second.type;
        auto s = std::visit(
          visitor{
            [](const func_attributes &a) -> asm_symbtab_entry { return fun_entry{ a.is_defined }; },
            [&](const local_attributes &a) -> asm_symbtab_entry { return obj_entry{ get_assembly_type(type), false }; },
            [&](const static_attributes &a) -> asm_symbtab_entry { return obj_entry{ get_assembly_type(type), true }; },
          },
          entry.second.attrs);

        add(parser::identifier{ entry.first }, s);
    }
}
assembly_type backend_symbol_table::get_assembly_type(const wccff::type &t)
{
    return std::visit(visitor{
                        [](const double_type &) -> assembly_type { return double_asm{}; },
                        [](const int_type &) -> assembly_type { return long_word{}; },
                        [](const long_type &) -> assembly_type { return quad_word{}; },
                        [](const unsigned_int_type &) -> assembly_type { return long_word{}; },
                        [](const unsigned_long_type &) -> assembly_type { return quad_word{}; },
                        [](const auto &) -> assembly_type { throw std::logic_error("Not implemented"); },
                      },
                      t);
}

int32_t backend_symbol_table::calculate_offset(const assembly_type &t)
{
    return std::visit(visitor{
                        [](const long_word &) { return 4; },
                        [&](const quad_word &) { return m_offset % 8 == 0 ? 8 : (8 + std::abs(m_offset) % 8); },
                        [&](const double_asm &) { return m_offset % 8 == 0 ? 8 : (8 + std::abs(m_offset) % 8); },
                      },
                      t);
}
} // namespace wccff::symbol_table
