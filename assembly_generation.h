#ifndef CODEGEN_H
#define CODEGEN_H

#include "parser.h"
#include <string>
#include <variant>
#include <vector>

namespace wccff::assembly_generation {

struct identifier
{
    std::string name;
};

struct immediate
{
    int32_t value;
};

struct register_node
{
};

using operand = std::variant<immediate, register_node>;

struct mov_instruction
{
    operand src;
    operand dst;
};

struct ret_instruction
{
};

using instruction = std::variant<mov_instruction, ret_instruction>;

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

} // namespace wccff::assembly_generation

#endif // CODEGEN_H
