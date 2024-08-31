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
struct negate_operator
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
using unary_operator = std::variant<binary_complement_operator, negate_operator>;
using binary_operator = std::variant<plus_operator,
                                     subtract_operator,
                                     multiply_operator,
                                     divide_operator,
                                     remainder_operator,
                                     binary_and_operator>;
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

using instruction = std::variant<return_statement, unary_statement, binary_statement>;

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
std::string pretty_print(const instruction &instruction, int32_t ident = 0);
std::string pretty_print(const std::vector<instruction> &instructions, int32_t ident = 0);
std::string pretty_print(const function_definition &f, int32_t ident = 0);
std::string pretty_print(const program &p, int32_t ident = 0);

} // namespace wccff::tacky
#endif // TACKY_H
