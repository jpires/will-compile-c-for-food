/*
 * Will Compile C for Food, a toy C compiler
 * Copyright (C) 2025  João Pires
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

#ifndef TYPES_H
#define TYPES_H

#include "visitor.h"
#include <cstdint>
#include <optional>
#include <variant>
#include <vector>

namespace wccff {

struct int_type
{
};
struct fun_type;
struct long_type
{
};
struct void_type
{
};

using type = std::variant<int_type, long_type, std::unique_ptr<fun_type>, void_type>;

struct fun_type
{
    std::vector<type> params;
    type return_type;

    bool operator==(const fun_type &other) const = default;
};

constexpr bool operator==(const type &lhs, const type &rhs)
{
    return std::visit(visitor{
                        [](const std::unique_ptr<fun_type> &l, const std::unique_ptr<fun_type> &r) { return *l == *r; },
                        [](const int_type &, const int_type &) { return true; },
                        [](const long_type &, const long_type &) { return true; },
                        [](const void_type &, const void_type &) { return true; },
                        [](const auto &, const auto &) { return false; },
                      },
                      lhs,
                      rhs);
}

std::unique_ptr<fun_type> copy_fun_type(const std::unique_ptr<fun_type> &n);
type copy_type(const type &n);
std::optional<type> copy_optional_type(const std::optional<type> &n);

struct int_initial
{
    int32_t value;
};
struct long_initial
{
    int64_t value;
};

using initial = std::variant<int_initial, long_initial>;

initial get_default_initial(const type &t);

struct int_constant
{
    int32_t value;
};

struct long_constant
{
    int64_t value;
};

using constant = std::variant<int_constant, long_constant>;

} // namespace wccff

#endif // TYPES_H
