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

#ifndef CODE_EMISSION_H
#define CODE_EMISSION_H

#include "assembly_generation.h"
#include <filesystem>

namespace wccff::code_emission {

enum class operand_size
{
    one_byte,
    four_bytes,
};
void process(const std::filesystem::path &output_file, const assembly_generation::program &p);

} // namespace wccff::code_emission

#endif // CODE_EMISSION_H
