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
struct XMM0
{
};
struct XMM1
{
};
struct XMM2
{
};
struct XMM3
{
};
struct XMM4
{
};
struct XMM5
{
};
struct XMM6
{
};
struct XMM7
{
};
struct XMM14
{
};
struct XMM15
{
};

using reg =
  std::variant<ax, cx, dx, di, si, R8, R9, R10, R11, SP, XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6, XMM7, XMM14, XMM15>;

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
struct left_shift_aritmetic
{
};
struct right_shift_aritmetic
{
};
struct div_double
{
};
using binary_operator = std::variant<add,
                                     sub,
                                     mul,
                                     binary_and,
                                     binary_or,
                                     binary_xor,
                                     left_shift,
                                     right_shift,
                                     left_shift_aritmetic,
                                     right_shift_aritmetic,
                                     div_double>;

struct A
{
};
struct AE
{
};
struct B
{
};
struct BE
{
};
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
using cond_code = std::variant<E, NE, G, GE, L, LE, A, AE, B, BE>;

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

struct cvtsi2sd
{
    operand src;
    operand dst;
    assembly_type src_type;
};

struct cvttsd2si
{
    operand src;
    operand dst;
    assembly_type dst_type;
};

struct div
{
    operand src;
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

struct mov_zero_extend
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
using instruction = std::variant<mov_instruction,
                                 movx,
                                 mov_zero_extend,
                                 unary,
                                 binary,
                                 cmp,
                                 cvtsi2sd,
                                 cvttsd2si,
                                 idiv,
                                 div,
                                 cdq,
                                 jmp,
                                 jmpcc,
                                 setcc,
                                 label,
                                 push,
                                 call,
                                 ret_instruction>;

struct function
{
    identifier name;
    std::vector<instruction> instructions;
    int32_t stack_size;
    bool is_global;
};

struct static_constant
{
    identifier name;
    int alignment;
    wccff::initial init;
};

struct static_variable
{
    identifier name;
    bool is_global;
    int alignment;
    wccff::initial init;
};

using top_level = std::variant<function, static_constant, static_variable>;
struct program
{
    std::vector<top_level> functions;
};

assembly_type get_assembly_type(const wccff::constant &v);
assembly_type get_assembly_type(const tacky::val &v, const wccff::symbol_table::symbol_table &table);
assembly_type get_assembly_type(const tacky::var &v, const wccff::symbol_table::symbol_table &table);

type get_type(const wccff::constant &v);
type get_type(const tacky::val &v, const wccff::symbol_table::symbol_table &table);
type get_type(const tacky::var &v, const wccff::symbol_table::symbol_table &table);

bool is_larger_immediate(const operand &op);
bool is_memory_operand(const operand &o);

class assembly_generation
{
  public:
    explicit assembly_generation(const wccff::symbol_table::symbol_table &table_)
      : m_constant_count{ 0 }
      , m_label_counter{ 0 }
      , m_table(table_)
    {
    }

    void process_binary_statement_double(const tacky::binary_statement &stmt);
    void process(const tacky::binary_statement &stmt);
    operand process(const constant &n);
    void process(const tacky::copy_statement &stmt);
    operand process(const double_constant &stmt);
    void process(const tacky::double_to_int &stmt);
    void process(const tacky::double_to_uint &stmt);
    void process(const tacky::fun_call &i);
    function process(const tacky::function_definition &f);
    identifier process(const tacky::identifier &id);
    void process(const tacky::int_to_double &stmt);
    void process(const tacky::instruction &i);
    void process(const tacky::jump_if_not_zero_statement &stmt);
    void process(const tacky::jump_if_zero_statement &stmt);
    void process(const tacky::jump_statement &stmt);
    void process(const tacky::label_statement &stmt);
    program process(const tacky::program &program);
    void process(const tacky::return_statement &stmt);
    void process(const tacky::sing_extend &i);
    static_variable process(const tacky::static_variable &f);
    top_level process(const tacky::top_level &f);
    void process(const tacky::truncate &i);
    void process(const tacky::uint_to_double &stmt);
    void process(const tacky::unary_statement &stmt);
    operand process(const wccff::tacky::val &v);
    void process(const std::vector<tacky::instruction> &s);
    void process(const tacky::zero_extend &i);

    const std::vector<instruction> &get_instructions() const { return m_instructions; }
    void reset_instructions() { m_instructions.clear(); }

  private:
    identifier get_new_label();
    std::array<std::vector<std::pair<assembly_type, operand>>, 3> classify_parameters(
      const std::vector<tacky::val> &params);

    std::unordered_map<initial, static_constant> m_constants_map;
    int32_t m_constant_count;
    int32_t m_label_counter;

    std::vector<instruction> m_instructions;
    const wccff::symbol_table::symbol_table &m_table;
};

void replace_pseudo_registers(program &node, symbol_table::backend_symbol_table &t);

std::optional<std::vector<instruction>> fixing_up_instructions_binary(const binary &n);

std::optional<std::vector<instruction>> fixing_up_instructions11(const cmp &n);
std::optional<std::vector<instruction>> fixing_up_instructions11(const cvtsi2sd &n);
std::optional<std::vector<instruction>> fixing_up_instructions11(const cvttsd2si &n);
std::optional<std::vector<instruction>> fixing_up_instructions11(const mov_instruction &n);
std::optional<std::vector<instruction>> fixing_up_instructions11(const mov_zero_extend &n);
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
std::string pretty_print(const cvtsi2sd &node);
std::string pretty_print(const cvttsd2si &node);
std::string pretty_print(const data &node);
std::string pretty_print(const div &node);
std::string pretty_print(const function &node);
std::string pretty_print(const identifier &node);
std::string pretty_print(const idiv &node);
std::string pretty_print(const immediate &node);
std::string pretty_print(const instruction &node);
std::string pretty_print(const jmp &node);
std::string pretty_print(const jmpcc &node);
std::string pretty_print(const label &node);
std::string pretty_print(const mov_instruction &node);
std::string pretty_print(const mov_zero_extend &node);
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
std::string pretty_print(const static_constant &node);
std::string pretty_print(const static_variable &node);
std::string pretty_print(const std::vector<instruction> &node);
std::string pretty_print(const unary &node);
std::string pretty_print(const unary_operator &node);

} // namespace wccff::assembly_generation

#endif // CODEGEN_H
