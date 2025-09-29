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
#include <cmath>
#include <cstdint>
#include <fmt/format.h>
#include <memory>
#include <optional>
#include <source_location>
#include <variant>
#include <vector>

namespace wccff {

std::string get_not_implemented_message(std::source_location loc = std::source_location::current());

struct identifier
{
    std::string name;
    bool operator==(const identifier &other) const = default;
};

/// \brief =
struct assignment_operator
{
};
/// \brief &=
struct compound_bitwise_and_operator
{
};
/// \brief |=
struct compound_bitwise_or_operator
{
};
/// \brief ^=
struct compound_bitwise_xor_operator
{
};
/// \brief /=
struct compound_divide_operator
{
};
/// \brief <<=
struct compound_left_shift_operator
{
};
/// \brief *=
struct compound_multiply_operator
{
};
/// \brief +=
struct compound_plus_operator
{
};
/// \brief %=
struct compound_remainder_operator
{
};
/// \brief >>=
struct compound_right_shift_operator
{
};
/// \brief -=
struct compound_subtract_operator
{
};

using assign_operator = std::variant<assignment_operator,
                                     compound_bitwise_and_operator,
                                     compound_bitwise_or_operator,
                                     compound_bitwise_xor_operator,
                                     compound_divide_operator,
                                     compound_left_shift_operator,
                                     compound_multiply_operator,
                                     compound_plus_operator,
                                     compound_remainder_operator,
                                     compound_right_shift_operator,
                                     compound_subtract_operator>;

bool is_compound_operation(const assign_operator &node);
std::string pretty_print(const assign_operator &node, int32_t ident = 0);

/// \brief &
struct bitwise_and_operator
{
};
/// \brief |
struct bitwise_or_operator
{
};
/// \brief ^
struct bitwise_xor_operator
{
};
/// \brief /
struct divide_operator
{
};
/// \brief ==
struct equals_operator
{
};
/// \brief >
struct greater_than_operator
{
};
/// \brief >=
struct greater_than_or_equal_operator
{
};
/// \brief <<
struct left_shift_operator
{
};
/// \brief <
struct less_than_operator
{
};
/// \brief <=
struct less_than_or_equal_operator
{
};
/// \brief &&
struct logical_and_operator
{
};
/// \brief ||
struct logical_or_operator
{
};
/// \brief *
struct multiply_operator
{
};
/// \brief !=
struct not_equals_operator
{
};
/// \brief +
struct plus_operator
{
};
/// \brief %
struct remainder_operator
{
};
/// \brief >>
struct right_shift_operator
{
};
/// \brief -
struct subtract_operator
{
};

using binary_operator = std::variant<bitwise_and_operator,
                                     bitwise_or_operator,
                                     bitwise_xor_operator,
                                     divide_operator,
                                     equals_operator,
                                     greater_than_operator,
                                     greater_than_or_equal_operator,
                                     left_shift_operator,
                                     less_than_operator,
                                     less_than_or_equal_operator,
                                     logical_and_operator,
                                     logical_or_operator,
                                     multiply_operator,
                                     not_equals_operator,
                                     plus_operator,
                                     remainder_operator,
                                     right_shift_operator,
                                     subtract_operator>;

binary_operator to_binary_operator(const assign_operator &op);
std::string pretty_print(const binary_operator &node, int32_t ident = 0);

/// \brief ~
struct bitwise_complement_operator
{
};
/// \brief -
struct negate_operator
{
};
/// \brief !
struct logical_not_operator
{
};
/// \brief --
struct prefix_decrement_operator
{
};
/// \brief ++
struct prefix_increment_operator
{
};
/// \brief --
struct postfix_decrement_operator
{
};
/// \brief ++
struct postfix_increment_operator
{
};
using unary_operator = std::variant<bitwise_complement_operator,
                                    logical_not_operator,
                                    negate_operator,
                                    postfix_decrement_operator,
                                    postfix_increment_operator,
                                    prefix_decrement_operator,
                                    prefix_increment_operator>;

std::string pretty_print(const unary_operator &node, int32_t ident = 0);

struct double_type
{
};
struct int_type
{
};
struct fun_type;
struct long_type
{
};
struct pointer;
struct unsigned_int_type
{
};
struct unsigned_long_type
{
};
struct void_type
{
};

using type = std::variant<int_type,
                          long_type,
                          std::unique_ptr<fun_type>,
                          std::unique_ptr<pointer>,
                          unsigned_int_type,
                          unsigned_long_type,
                          void_type,
                          double_type>;

struct fun_type
{
    std::vector<type> params;
    type return_type;

    bool operator==(const fun_type &other) const = default;
};

struct pointer
{
    type referenced;

    bool operator==(const pointer &other) const = default;
};

constexpr bool operator==(const type &lhs, const type &rhs)
{
    return std::visit(visitor{
                        [](const double_type &, const double_type &) { return true; },
                        [](const std::unique_ptr<fun_type> &l, const std::unique_ptr<fun_type> &r) { return *l == *r; },
                        [](const std::unique_ptr<pointer> &l, const std::unique_ptr<pointer> &r) { return *l == *r; },
                        [](const int_type &, const int_type &) { return true; },
                        [](const long_type &, const long_type &) { return true; },
                        [](const unsigned_int_type &, const unsigned_int_type &) { return true; },
                        [](const unsigned_long_type &, const unsigned_long_type &) { return true; },
                        [](const void_type &, const void_type &) { return true; },
                        [](const auto &, const auto &) { return false; },
                      },
                      lhs,
                      rhs);
}

std::unique_ptr<fun_type> copy_fun_type(const std::unique_ptr<fun_type> &n);
type copy_type(const type &n);
std::optional<type> copy_optional_type(const std::optional<type> &n);
type get_common_type(const type &t1, const type &t2);

int32_t get_type_size(const type &t);
bool is_arithmetic(const type &t);
bool is_pointer(const type &t);
bool is_signed_type(const type &t);

/**
 * \brief Stores the initial value for a double variable
 * \remarks The values -0.0 and 0.0 are not equal to each other.
 */
struct double_initial
{
    double value;
    bool operator==(const double_initial &other) const
    {
        if (value == 0.0 && other.value == 0.0)
        {
            return std::signbit(value) == std::signbit(other.value);
        };

        return value == other.value;
    };
};

struct int_initial
{
    int32_t value;
    bool operator==(const int_initial &other) const = default;
};

struct long_initial
{
    int64_t value;
    bool operator==(const long_initial &other) const = default;
};

struct unsigned_int_initial
{
    uint32_t value;
    bool operator==(const unsigned_int_initial &other) const = default;
};

struct unsigned_long_initial
{
    uint64_t value;
    bool operator==(const unsigned_long_initial &other) const = default;
};

using initial = std::variant<int_initial, long_initial, unsigned_int_initial, unsigned_long_initial, double_initial>;

constexpr bool operator==(const initial &left, const initial &right)
{
    return std::visit(visitor{
                        [](const int_initial &l, const int_initial &r) { return l == r; },
                        [](const long_initial &l, const long_initial &r) { return l == r; },
                        [](const unsigned_int_initial &l, const unsigned_int_initial &r) { return l == r; },
                        [](const unsigned_long_initial &l, const unsigned_long_initial &r) { return l == r; },
                        [](const double_initial &l, const double_initial &r) { return l == r; },
                        [](const auto &, const auto &) { return false; },
                      },
                      left,
                      right);
}

std::string pretty_print(const initial &i);

initial get_default_initial(const type &t);

struct double_constant
{
    double value;
};

struct int_constant
{
    int32_t value;
};

struct long_constant
{
    int64_t value;
};

struct unsigned_int_constant
{
    uint32_t value;
};

struct unsigned_long_constant
{
    uint64_t value;
};

using constant =
  std::variant<double_constant, int_constant, long_constant, unsigned_int_constant, unsigned_long_constant>;

struct long_word
{
};
struct quad_word
{
};
struct double_asm
{
};

using assembly_type = std::variant<long_word, quad_word, double_asm>;

} // namespace wccff

template<>
struct std::hash<wccff::int_initial>
{
    std::size_t operator()(const wccff::int_initial &k) const noexcept { return std::hash<int32_t>()(k.value); }
};
template<>
struct std::hash<wccff::long_initial>
{
    std::size_t operator()(const wccff::long_initial &k) const noexcept { return std::hash<int64_t>()(k.value); }
};

template<>
struct std::hash<wccff::unsigned_int_initial>
{
    std::size_t operator()(const wccff::unsigned_int_initial &k) const noexcept
    {
        return std::hash<uint32_t>()(k.value);
    }
};
template<>
struct std::hash<wccff::unsigned_long_initial>
{
    std::size_t operator()(const wccff::unsigned_long_initial &k) const noexcept
    {
        return std::hash<uint64_t>()(k.value);
    }
};

/**
 * \brief Hash specialisation for double_initial
 * \remark The hash for 0.0 and -0.0 are different.
 */
template<>
struct std::hash<wccff::double_initial>
{
    std::size_t operator()(const wccff::double_initial &k) const noexcept
    {
        return *reinterpret_cast<const std::size_t *>(&k.value);
    }
};

template<>
struct std::hash<wccff::initial>
{
    std::size_t operator()(const wccff::initial &k) const noexcept
    {
        return std::visit(
          wccff::visitor{
            [](const wccff::int_initial &k1) { return std::hash<wccff::int_initial>()(k1); },
            [](const wccff::long_initial &k1) { return std::hash<wccff::long_initial>()(k1); },
            [](const wccff::unsigned_int_initial &k1) { return std::hash<wccff::unsigned_int_initial>()(k1); },
            [](const wccff::unsigned_long_initial &k1) { return std::hash<wccff::unsigned_long_initial>()(k1); },
            [](const wccff::double_initial &k1) { return std::hash<wccff::double_initial>()(k1); },
          },
          k);
    }
};

template<>
struct fmt::formatter<wccff::identifier> : formatter<string_view>
{
    template<typename FormatContext>
    auto format(const wccff::identifier &id, FormatContext &ctx) const
    {
        auto str = fmt::format("{}", id.name);
        return formatter<string_view>::format(str, ctx);
    }
};

#endif // TYPES_H
