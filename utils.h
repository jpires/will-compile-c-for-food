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

#ifndef UTILS_H
#define UTILS_H
#include <fmt/format.h>

namespace wccff {
template<typename... Args>
auto format_indented(int indent, fmt::format_string<Args...> format_str, Args &&...args)
{
    auto prefix = fmt::format("{:{}}", "", indent);
    auto msg = fmt::format(format_str, std::forward<Args>(args)...);
    return fmt::format("{}{}", prefix, msg);
}
} // namespace wccff

#endif // UTILS_H
