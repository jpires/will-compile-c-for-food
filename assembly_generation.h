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
struct R10
{
};

using reg = std::variant<ax, R10>;

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

struct unary
{
    unary_operator op;
    operand dst;
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

using instruction = std::variant<mov_instruction, unary, allocate_stack, ret_instruction>;

struct function
{
    identifier name;
    std::vector<instruction> instructions;
};

struct program
{
    function function;
};

program process(const wccff::parser::program &program);
program process(const wccff::tacky::program &program);

void fixing_up_instructions(std::vector<instruction> &node);
void fixing_up_instructions(function &node);
void replace_pseudo_registers(program &node);

void fixing_up_instructions(program &program);

std::string pretty_print(const program &program);
} // namespace wccff::assembly_generation

#endif // CODEGEN_H
