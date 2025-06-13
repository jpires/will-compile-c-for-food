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

#include "common.h"

namespace wccff {
std::unique_ptr<fun_type> copy_fun_type(const std::unique_ptr<fun_type> &n)
{
    std::vector<type> new_params;
    for (const auto &p : n->params)
    {
        new_params.push_back(copy_type(p));
    }
    return std::make_unique<fun_type>(std::move(new_params), copy_type(n->return_type));
}
type copy_type(const type &n)
{
    return std::visit(visitor{
                        [](const int_type) -> type { return int_type{}; },
                        [](const long_type) -> type { return long_type{}; },
                        [](const std::unique_ptr<fun_type> &n) -> type { return copy_fun_type(n); },
                        [](const void_type) -> type { return void_type{}; },
                      },
                      n);
}
std::optional<type> copy_optional_type(const std::optional<type> &n)
{
    if (n.has_value())
    {
        return copy_type(n.value());
    }
    return std::nullopt;
}

initial get_default_initial(const type &t)
{
    return std::visit(
      visitor{
        [](const int_type) -> initial { return int_initial{ 0 }; },
        [](const long_type) -> initial { return long_initial{ 0 }; },
        [](const void_type) -> initial { throw std::runtime_error("void type has no default value"); },
        [](const std::unique_ptr<fun_type> &) -> initial { throw std::runtime_error("fun type has no default value"); },
      },
      t);
}

std::string pretty_print(const initial &i)
{
    return std::visit(visitor{
                        [](const int_initial &n) { return std::to_string(n.value); },
                        [](const long_initial &n) { return std::to_string(n.value); },
                      },
                      i);
}

} // namespace wccff
