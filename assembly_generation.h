#ifndef CODEGEN_H
#define CODEGEN_H

#include "parser.h"
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
    int32_t value;
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
struct R10
{
};
struct R11
{
};

using reg = std::variant<ax, cx, dx, R10, R11>;

struct pseudo
{
    identifier name;
};
struct stack
{
    immediate value;
};
using operand = std::variant<immediate, reg, pseudo, stack>;

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
};
struct binary
{
    binary_operator op;
    operand src;
    operand dst;
};

struct cmp
{
    operand lhs;
    operand rhs;
};

struct idiv
{
    operand src;
};
struct cdq
{
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
};

struct ret_instruction
{
};

struct allocate_stack
{
    immediate size;
};

using instruction = std::
  variant<mov_instruction, unary, binary, cmp, idiv, cdq, jmp, jmpcc, setcc, label, allocate_stack, ret_instruction>;

struct function
{
    identifier name;
    std::vector<instruction> instructions;
};

struct program
{
    function function;
};

std::vector<instruction> process_statement(const wccff::tacky::copy_statement &stmt);
std::vector<instruction> process_statement(const wccff::tacky::return_statement &stmt);
std::vector<instruction> process_statement(const wccff::tacky::binary_statement &stmt);
std::vector<instruction> process_statement(const wccff::tacky::unary_statement &stmt);
std::vector<instruction> process_statement(const tacky::instruction &i);
std::vector<instruction> process_statement(const std::vector<tacky::instruction> &s);
function process_function(const wccff::tacky::function_definition &f);
program process(const wccff::tacky::program &program);

void replace_pseudo_registers(program &node);

void fixing_up_instructions(std::vector<instruction> &node);
void fixing_up_instructions(function &node);
void fixing_up_instructions(program &program);

std::string pretty_print(const cmp &node);
std::string pretty_print(const cond_code &node);
std::string pretty_print(const jmp &node);
std::string pretty_print(const jmpcc &node);
std::string pretty_print(const label &node);
std::string pretty_print(const setcc &node);

std::string pretty_print(const identifier &node);
std::string pretty_print(const unary_operator &node);
std::string pretty_print(const immediate &node);
std::string pretty_print(const reg &node);
std::string pretty_print(const pseudo &node);
std::string pretty_print(const stack &node);
std::string pretty_print(const operand &node);
std::string pretty_print(const mov_instruction &node);
std::string pretty_print(const unary &node);
std::string pretty_print(const allocate_stack &node);
std::string pretty_print(const ret_instruction &node);
std::string pretty_print(const instruction &node);
std::string pretty_print(const std::vector<instruction> &node);
std::string pretty_print(const function &node);
std::string pretty_print(const program &program);
} // namespace wccff::assembly_generation

#endif // CODEGEN_H
