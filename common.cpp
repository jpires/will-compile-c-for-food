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
#include "utils.h"
#include <fmt/format.h>

namespace wccff {

std::string get_not_implemented_message(std::source_location loc)
{
    return fmt::format("{}: Not implemented", loc.function_name());
}

std::string pretty_print(const binary_operator &node, int32_t ident)
{
    using wccff::format_indented;
    return std::visit(
      wccff::visitor{
        [ident](const assignment_operator &) { return format_indented(ident, "Assignment"); },
        [ident](const bitwise_and_operator &) { return format_indented(ident, "Bitwise And"); },
        [ident](const bitwise_or_operator &) { return format_indented(ident, "Bitwise Or"); },
        [ident](const bitwise_xor_operator &) { return format_indented(ident, "Bitwise Xor"); },
        [ident](const compound_bitwise_and_operator &) { return format_indented(ident, "Compound Bitwise And"); },
        [ident](const compound_bitwise_or_operator &) { return format_indented(ident, "Compound Bitwise Or"); },
        [ident](const compound_bitwise_xor_operator &) { return format_indented(ident, "Compound Bitwise Xor"); },
        [ident](const compound_divide_operator &) { return format_indented(ident, "Compound Division"); },
        [ident](const compound_left_shift_operator &) { return format_indented(ident, "Compound Left Shift"); },
        [ident](const compound_multiply_operator &) { return format_indented(ident, "Compound Multiplication"); },
        [ident](const compound_plus_operator &) { return format_indented(ident, "Compound Plus"); },
        [ident](const compound_remainder_operator &) { return format_indented(ident, "Compound Remainder"); },
        [ident](const compound_right_shift_operator &) { return format_indented(ident, "Compound Right Shift"); },
        [ident](const compound_subtract_operator &) { return format_indented(ident, "Compound Minus"); },
        [ident](const divide_operator &) { return format_indented(ident, "Divide"); },
        [ident](const equals_operator &) { return format_indented(ident, "Equals"); },
        [ident](const greater_than_operator &) { return format_indented(ident, "Greater Than"); },
        [ident](const greater_than_or_equal_operator &) { return format_indented(ident, "Greater Than or Equals"); },
        [ident](const left_shift_operator &) { return format_indented(ident, "Left Shift"); },
        [ident](const less_than_operator &) { return format_indented(ident, "Less Than"); },
        [ident](const less_than_or_equal_operator &) { return format_indented(ident, "Less Than or Equals"); },
        [ident](const logical_and_operator &) { return format_indented(ident, "Logic And"); },
        [ident](const logical_or_operator &) { return format_indented(ident, "Logic Or"); },
        [ident](const multiply_operator &) { return format_indented(ident, "Multiply"); },
        [ident](const not_equals_operator &) { return format_indented(ident, "Not Equals"); },
        [ident](const plus_operator &) { return format_indented(ident, "Plus"); },
        [ident](const remainder_operator &) { return format_indented(ident, "Remainder"); },
        [ident](const right_shift_operator &) { return format_indented(ident, "Right Shift"); },
        [ident](const subtract_operator &) { return format_indented(ident, "Subtract"); },
      },
      node);
}

std::string pretty_print(const unary_operator &node, int32_t ident)
{
    using wccff::format_indented;
    return std::visit(
      wccff::visitor{
        [ident](const bitwise_complement_operator &) { return format_indented(ident, "Complement"); },
        [ident](const logical_not_operator &) { return format_indented(ident, "Not"); },
        [ident](const negate_operator &) { return format_indented(ident, "Negate"); },
        [ident](const postfix_decrement_operator &) { return format_indented(ident, "Postfix Decrement"); },
        [ident](const postfix_increment_operator &) { return format_indented(ident, "Postfix Increment"); },
        [ident](const prefix_decrement_operator &) { return format_indented(ident, "Prefix Decrement"); },
        [ident](const prefix_increment_operator &) { return format_indented(ident, "Prefix Increment"); },
      },
      node);
}

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
                        [](const double_type) -> type { return double_type{}; },
                        [](const int_type) -> type { return int_type{}; },
                        [](const long_type) -> type { return long_type{}; },
                        [](const std::unique_ptr<fun_type> &n) -> type { return copy_fun_type(n); },
                        [](const unsigned_int_type) -> type { return unsigned_int_type{}; },
                        [](const unsigned_long_type) -> type { return unsigned_long_type{}; },
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

type get_common_type(const type &t1, const type &t2)
{
    // 6.3.1.8 Usual arithmetic conversions
    if (t1 == t2)
    {
        return copy_type(t1);
    }

    if (double_type{} == t1 || double_type{} == t2)
    {
        return double_type{};
    }

    if (get_type_size(t1) == get_type_size(t2))
    {
        if (is_signed_type(t1))
        {
            return copy_type(t2);
        }
        return copy_type(t1);
    }

    if (get_type_size(t1) > get_type_size(t2))
    {
        return copy_type(t1);
    }
    return copy_type(t2);
}

int32_t get_type_size(const type &t)
{
    return std::visit(
      visitor{
        [](const int_type &) { return 4; },
        [](const long_type &) { return 8; },
        [](const unsigned_int_type &) { return 4; },
        [](const unsigned_long_type &) { return 8; },
        [](const auto &) -> int32_t { throw std::runtime_error("Trying to get size of non-integral type"); },
      },
      t);
}

bool is_signed_type(const type &t)
{
    return std::visit(
      visitor{
        [](const int_type &) { return true; },
        [](const long_type &) { return true; },
        [](const unsigned_int_type &) { return false; },
        [](const unsigned_long_type &) { return false; },
        [](const auto &) -> bool { throw std::runtime_error("Trying to get signess of non-integral type"); },
      },
      t);
}

initial get_default_initial(const type &t)
{
    return std::visit(
      visitor{
        [](const double_type) -> initial { return double_initial{ 0.0 }; },
        [](const int_type) -> initial { return int_initial{ 0 }; },
        [](const long_type) -> initial { return long_initial{ 0 }; },
        [](const unsigned_int_type) -> initial { return long_initial{ 0 }; },
        [](const unsigned_long_type) -> initial { return long_initial{ 0 }; },
        [](const void_type) -> initial { throw std::runtime_error("void type has no default value"); },
        [](const std::unique_ptr<fun_type> &) -> initial { throw std::runtime_error("fun type has no default value"); },
      },
      t);
}

std::string pretty_print(const initial &i)
{
    return std::visit(visitor{
                        [](const double_initial &n) { return std::to_string(n.value); },
                        [](const int_initial &n) { return std::to_string(n.value); },
                        [](const long_initial &n) { return std::to_string(n.value); },
                        [](const unsigned_int_initial &n) { return std::to_string(n.value); },
                        [](const unsigned_long_initial &n) { return std::to_string(n.value); },
                      },
                      i);
}

} // namespace wccff
