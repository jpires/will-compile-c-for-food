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

#ifndef TACKY_H
#define TACKY_H

#include "parser.h"
#include <cstdint>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace wccff::tacky {

struct tacky_error
{
    std::string error_message;
};

struct identifier
{
    std::string name;
};

struct binary_and_operator
{
};
struct binary_complement_operator
{
};
struct binary_or_operator
{
};
struct binary_xor_operator
{
};
struct negate_operator
{
};
struct not_operator
{
};

struct plus_operator
{
};
struct subtract_operator
{
};
struct multiply_operator
{
};
struct divide_operator
{
};
struct remainder_operator
{
};
struct left_shift_operator
{
};
struct right_shift_operator
{
};
struct equal_operator
{
};
struct not_equal_operator
{
};
struct less_than_operator
{
};
struct less_than_or_equal_operator
{
};
struct greater_than_operator
{
};
struct greater_than_or_equal_operator
{
};

using unary_operator = std::variant<binary_complement_operator, negate_operator, not_operator>;
using binary_operator = std::variant<plus_operator,
                                     subtract_operator,
                                     multiply_operator,
                                     divide_operator,
                                     remainder_operator,
                                     binary_and_operator,
                                     binary_or_operator,
                                     binary_xor_operator,
                                     left_shift_operator,
                                     right_shift_operator,
                                     equal_operator,
                                     not_equal_operator,
                                     less_than_operator,
                                     less_than_or_equal_operator,
                                     greater_than_operator,
                                     greater_than_or_equal_operator>;
struct constant
{
    int32_t value;
};
struct var
{
    identifier id;
};

using val = std::variant<constant, var>;

struct return_statement
{
    val val;
};

struct unary_statement
{
    unary_statement(unary_operator op_, val src_, val dst_)
      : op(op_)
      , src(std::move(src_))
      , dst(std::move(dst_))
    {
    }
    unary_operator op;
    val src;
    val dst;
};

struct binary_statement
{
    binary_statement(binary_operator op_, val src1_, val src2_, val dst_)
      : op(op_)
      , src1(std::move(src1_))
      , src2(std::move(src2_))
      , dst(std::move(dst_))
    {
    }
    binary_operator op;
    val src1;
    val src2;
    val dst;
};

struct copy_statement
{
    copy_statement(val src_, val dst_)
      : src(std::move(src_))
      , dst(std::move(dst_))
    {
    }
    val src;
    val dst;
};

struct jump_statement
{
    explicit jump_statement(identifier target_)
      : target(std::move(target_))
    {
    }
    identifier target;
};

struct jump_if_zero_statement
{
    explicit jump_if_zero_statement(val condition_, identifier target_)
      : condition(std::move(condition_))
      , target(std::move(target_))
    {
    }
    val condition;
    identifier target;
};

struct jump_if_not_zero_statement
{
    explicit jump_if_not_zero_statement(val condition_, identifier target_)
      : condition(std::move(condition_))
      , target(std::move(target_))
    {
    }
    val condition;
    identifier target;
};

struct label_statement
{
    explicit label_statement(identifier target_)
      : target(std::move(target_))
    {
    }
    identifier target;
};

using instruction = std::variant<return_statement,
                                 unary_statement,
                                 binary_statement,
                                 copy_statement,
                                 jump_statement,
                                 jump_if_zero_statement,
                                 jump_if_not_zero_statement,
                                 label_statement>;

struct function_definition
{
    identifier name;
    std::vector<instruction> instructions;
};

struct program
{
    function_definition function;
};

identifier process_identifier(const parser::identifier &id);
constant process_int_constant(const parser::int_constant &int_con);
unary_operator process_unary_operator(const parser::unary_operator &op);
binary_operator process_binary_operator(const parser::binary_operator &op);
val process_binary_node(const std::unique_ptr<parser::binary_node> &node, std::vector<instruction> &instructions);
val process_unary_node(const std::unique_ptr<parser::unary_node> &node, std::vector<instruction> &instructions);

val process_expression(const wccff::parser::expression &exp, std::vector<instruction> &instructions);
program process(const parser::program &input);

std::string pretty_print(const unary_operator &val, int32_t ident = 0);
std::string pretty_print(const constant &val, int32_t ident = 0);
std::string pretty_print(const var &val, int32_t ident = 0);
std::string pretty_print(const val &val, int32_t ident = 0);
std::string pretty_print(const return_statement &instruction, int32_t ident = 0);
std::string pretty_print(const unary_statement &instruction, int32_t ident = 0);
std::string pretty_print(const binary_statement &i, int32_t ident = 0);
std::string pretty_print(const copy_statement &i, int32_t ident = 0);
std::string pretty_print(const jump_statement &i, int32_t ident = 0);
std::string pretty_print(const jump_if_zero_statement &i, int32_t ident = 0);
std::string pretty_print(const jump_if_not_zero_statement &i, int32_t ident = 0);
std::string pretty_print(const label_statement &i, int32_t ident = 0);
std::string pretty_print(const instruction &instruction, int32_t ident = 0);
std::string pretty_print(const std::vector<instruction> &instructions, int32_t ident = 0);
std::string pretty_print(const function_definition &f, int32_t ident = 0);
std::string pretty_print(const program &p, int32_t ident = 0);

} // namespace wccff::tacky
#endif // TACKY_H
