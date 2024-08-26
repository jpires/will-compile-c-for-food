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

struct binary_complement_operator
{
};
struct negate_operator
{
};

using unary_operator = std::variant<binary_complement_operator, negate_operator>;

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

using instruction = std::variant<return_statement, unary_statement>;

struct function_definition
{
    identifier name;
    std::vector<instruction> instructions;
};

struct program
{
    function_definition function;
};
val process_expression(const wccff::parser::expression &exp, std::vector<instruction> &instructions);
program process(const parser::program &input);

std::string pretty_print(const unary_operator &val, int32_t ident = 0);
std::string pretty_print(const constant &val, int32_t ident = 0);
std::string pretty_print(const var &val, int32_t ident = 0);
std::string pretty_print(const val &val, int32_t ident = 0);
std::string pretty_print(const return_statement &instruction, int32_t ident = 0);
std::string pretty_print(const unary_statement &instruction, int32_t ident = 0);
std::string pretty_print(const instruction &instruction, int32_t ident = 0);
std::string pretty_print(const std::vector<instruction> &instructions, int32_t ident = 0);
std::string pretty_print(const function_definition &f, int32_t ident = 0);
std::string pretty_print(const program &p, int32_t ident = 0);

} // namespace wccff::tacky
#endif // TACKY_H
