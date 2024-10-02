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

#ifndef COMPILER_H
#define COMPILER_H

#include <filesystem>

namespace wccff {
enum class stop_phase
{
    no_stop,
    lexer,
    parser,
    tacky,
    codegen
};

bool compile(const std::filesystem::path &source_filename,
             const std::filesystem::path &output_filename,
             stop_phase stop);
} // namespace wccff
#endif // COMPILER_H
