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

#ifndef CODEGEN_H
#define CODEGEN_H

#include "parser.h"
#include "symbol_table.h"
#include "tacky.h"
#include <compare>
#include <string>
#include <variant>
#include <vector>

namespace wccff::assembly_generation {

struct identifier
{
    std::string name;
    bool operator==(const identifier &) const = default;
};

struct immediate
{
    int64_t value;
};

struct ax
{
};
struct cx
{
};
struct dx
{
};
struct di
{
};
struct si
{
};
struct R8
{
};
struct R9
{
};
struct R10
{
};
struct R11
{
};
struct SP
{
};

using reg = std::variant<ax, cx, dx, di, si, R8, R9, R10, R11, SP>;

struct pseudo
{
    identifier name;
};
struct stack
{
    immediate value;
};
struct data
{
    identifier name;
};

using operand = std::variant<immediate, reg, pseudo, stack, data>;

struct neg_op
{
};
struct not_op
{
};

using unary_operator = std::variant<not_op, neg_op>;

struct add
{
};

struct sub
{
};
struct mul
{
};
struct binary_and
{
};
struct binary_or
{
};
struct binary_xor
{
};
struct left_shift
{
};
struct right_shift
{
};
using binary_operator = std::variant<add, sub, mul, binary_and, binary_or, binary_xor, left_shift, right_shift>;

struct E
{
};
struct NE
{
};
struct G
{
};
struct GE
{
};
struct L
{
};
struct LE
{
};
using cond_code = std::variant<E, NE, G, GE, L, LE>;

struct unary
{
    unary_operator op;
    operand dst;
    assembly_type type;
};
struct binary
{
    binary_operator op;
    operand src;
    operand dst;
    assembly_type type;
};

struct cmp
{
    operand lhs;
    operand rhs;
    assembly_type type;
};

struct idiv
{
    operand src;
    assembly_type type;
};
struct cdq
{
    assembly_type type;
};
struct jmp
{
    identifier name;
};
struct jmpcc
{
    cond_code cond;
    identifier name;
};
struct setcc
{
    cond_code cond;
    operand dst;
};
struct label
{
    identifier name;
};

struct mov_instruction
{
    operand src;
    operand dst;
    assembly_type type;
};

struct movx
{
    operand src;
    operand dst;
};

struct ret_instruction
{
};

struct push
{
    operand src;
};
struct call
{
    identifier fun_name;
};
using instruction = std::
  variant<mov_instruction, movx, unary, binary, cmp, idiv, cdq, jmp, jmpcc, setcc, label, push, call, ret_instruction>;

struct function
{
    identifier name;
    std::vector<instruction> instructions;
    int32_t stack_size;
    bool is_global;
};

struct static_variable
{
    identifier name;
    bool is_global;
    int alignment;
    wccff::initial init;
};

using top_level = std::variant<function, static_variable>;
struct program
{
    std::vector<top_level> functions;
};

assembly_type get_assembly_type(const wccff::constant &v);
assembly_type get_assembly_type(const tacky::val &v, const wccff::symbol_table::symbol_table &table);
assembly_type get_assembly_type(const tacky::var &v, const wccff::symbol_table::symbol_table &table);

bool is_larger_immediate(const operand &op);
bool is_memory_operand(const operand &o);

std::vector<instruction> process_statement(const wccff::tacky::copy_statement &stmt,
                                           const wccff::symbol_table::symbol_table &t);
std::vector<instruction> process_statement(const wccff::tacky::return_statement &stmt,
                                           const wccff::symbol_table::symbol_table &t);
std::vector<instruction> process_statement(const wccff::tacky::binary_statement &stmt,
                                           const wccff::symbol_table::symbol_table &t);
std::vector<instruction> process_statement(const wccff::tacky::unary_statement &stmt,
                                           const wccff::symbol_table::symbol_table &t);
std::vector<instruction> process_statement(const tacky::instruction &i, const wccff::symbol_table::symbol_table &t);
std::vector<instruction> process_statement(const std::vector<tacky::instruction> &s,
                                           const wccff::symbol_table::symbol_table &t);
std::vector<instruction> process_statement(const tacky::sing_extend &i);
std::vector<instruction> process_statement(const tacky::truncate &i);
std::vector<instruction> fun_call(const tacky::fun_call &i, const wccff::symbol_table::symbol_table &t);

function process_function(const wccff::tacky::function_definition &f, const symbol_table::symbol_table &t);
top_level process_top_level(const wccff::tacky::top_level &f, const wccff::symbol_table::symbol_table &t);
static_variable process_static_variable(const wccff::tacky::static_variable &f,
                                        const wccff::symbol_table::symbol_table &t);
program process(const wccff::tacky::program &program, const wccff::symbol_table::symbol_table &t);

void replace_pseudo_registers(program &node, symbol_table::backend_symbol_table &t);

std::optional<std::vector<instruction>> fixing_up_instructions_binary(const binary &n);

std::optional<std::vector<instruction>> fixing_up_instructions11(const cmp &n);
std::optional<std::vector<instruction>> fixing_up_instructions11(const mov_instruction &n);
std::optional<std::vector<instruction>> fixing_up_instructions11(const movx &n);
std::optional<std::vector<instruction>> fixing_up_instructions11(const push &n);

void fixing_up_instructions(std::vector<instruction> &node);
void fixing_up_instructions(function &node);
void fixing_up_instructions(program &program);

std::string pretty_print(const assembly_type &node);
std::string pretty_print(const binary &node);
std::string pretty_print(const binary_operator &node);
std::string pretty_print(const call &node);
std::string pretty_print(const cdq &node);
std::string pretty_print(const cmp &node);
std::string pretty_print(const cond_code &node);
std::string pretty_print(const data &node);
std::string pretty_print(const function &node);
std::string pretty_print(const identifier &node);
std::string pretty_print(const idiv &node);
std::string pretty_print(const immediate &node);
std::string pretty_print(const instruction &node);
std::string pretty_print(const jmp &node);
std::string pretty_print(const jmpcc &node);
std::string pretty_print(const label &node);
std::string pretty_print(const mov_instruction &node);
std::string pretty_print(const movx &node);
std::string pretty_print(const operand &node);
std::string pretty_print(const program &program);
std::string pretty_print(const pseudo &node);
std::string pretty_print(const push &program);
std::string pretty_print(const reg &node);
std::string pretty_print(const ret_instruction &node);
std::string pretty_print(const setcc &node);
std::string pretty_print(const top_level &node);
std::string pretty_print(const stack &node);
std::string pretty_print(const static_variable &node);
std::string pretty_print(const std::vector<instruction> &node);
std::string pretty_print(const unary &node);
std::string pretty_print(const unary_operator &node);

} // namespace wccff::assembly_generation

#endif // CODEGEN_H
