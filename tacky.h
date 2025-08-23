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
#include "symbol_table.h"
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

struct var
{
    identifier id;
};

using val = std::variant<wccff::constant, var>;

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

struct double_to_int
{
    val src;
    val dst;
};

struct double_to_uint
{
    val src;
    val dst;
};

struct fun_call
{
    identifier fun_name;
    std::vector<val> args;
    val dst;
};

struct int_to_double
{
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

struct sing_extend
{
    val src;
    val dst;
};

struct truncate
{
    val src;
    val dst;
};

struct uint_to_double
{
    val src;
    val dst;
};

struct zero_extend
{
    val src;
    val dst;
};

using instruction = std::variant<return_statement,
                                 unary_statement,
                                 binary_statement,
                                 copy_statement,
                                 double_to_int,
                                 double_to_uint,
                                 jump_statement,
                                 jump_if_zero_statement,
                                 jump_if_not_zero_statement,
                                 int_to_double,
                                 label_statement,
                                 fun_call,
                                 sing_extend,
                                 truncate,
                                 uint_to_double,
                                 zero_extend>;

struct function_definition
{
    identifier name;
    bool global;
    std::vector<identifier> params;
    std::vector<instruction> instructions;
};

struct static_variable
{
    identifier name;
    bool global;
    type type;
    initial init;
};

using top_level = std::variant<function_definition, static_variable>;

struct program
{
    std::vector<top_level> function;
};

var make_temporary_variable(const type &t, symbol_table::symbol_table &table);

val process_assignment_node(const std::unique_ptr<parser::assignment_node> &node,
                            std::vector<instruction> &instructions,
                            symbol_table::symbol_table &table);
val process_binary_node(const std::unique_ptr<parser::binary_node> &node,
                        std::vector<instruction> &instructions,
                        symbol_table::symbol_table &table);
wccff::binary_operator process_binary_operator(const wccff::binary_operator &op);
void process_block(const parser::block &node,
                   std::vector<instruction> &instructions,
                   symbol_table::symbol_table &table);

void process_block_item(const parser::block_item &node,
                        std::vector<instruction> &instructions,
                        symbol_table::symbol_table &table);

void process_break_statement(const parser::break_statement &node, std::vector<instruction> &instructions);

val process_cast_expression(const std::unique_ptr<parser::cast_expression> &node,
                            std::vector<instruction> &instructions,
                            symbol_table::symbol_table &table);

void process_compound_statement(const std::unique_ptr<parser::compound_statement> &node,
                                std::vector<instruction> &instructions,
                                symbol_table::symbol_table &table);

val process_conditional_node(const std::unique_ptr<parser::conditional_node> &node,
                             std::vector<instruction> &instructions,
                             symbol_table::symbol_table &table);

constant process_constant(const constant &node);
void process_continue_statement(const parser::continue_statement &node, std::vector<instruction> &instructions);

void process_declaration(const parser::declaration &node,
                         std::vector<instruction> &instructions,
                         symbol_table::symbol_table &table);

void process_do_while_statement(const std::unique_ptr<parser::do_while_statement> &node,
                                std::vector<instruction> &instructions,
                                symbol_table::symbol_table &table);

val process_expression(const wccff::parser::expression &exp,
                       std::vector<instruction> &instructions,
                       symbol_table::symbol_table &table);

void process_for_init(const parser::for_init &node,
                      std::vector<instruction> &instructions,
                      symbol_table::symbol_table &table);

void process_for_statement(const std::unique_ptr<parser::for_statement> &node,
                           std::vector<instruction> &instructions,
                           symbol_table::symbol_table &table);

val process_function_call(const std::unique_ptr<parser::function_call> &f,
                          std::vector<instruction> &instructions,
                          symbol_table::symbol_table &table);

std::optional<function_definition> process_function_definition(const parser::function_declaration &f,
                                                               symbol_table::symbol_table &table);

void process_goto_statement(const parser::goto_statement &node, std::vector<instruction> &instructions);
void process_if(const std::unique_ptr<parser::if_node> &id,
                std::vector<instruction> &instructions,
                symbol_table::symbol_table &table);

void process_labeled_statement(const std::unique_ptr<parser::labelled_statement> &id,
                               std::vector<instruction> &instructions,
                               symbol_table::symbol_table &table);

val process_prefix_unary(const std::unique_ptr<parser::unary_node> &node,
                         std::vector<instruction> &instructions,
                         symbol_table::symbol_table &table);

val process_postfix_unary(const std::unique_ptr<parser::unary_node> &node,
                          std::vector<instruction> &instructions,
                          symbol_table::symbol_table &table);

void process_return_node(const wccff::parser::return_node &stmt,
                         std::vector<instruction> &instructions,
                         symbol_table::symbol_table &table);

void process_statement(const wccff::parser::statement &s,
                       std::vector<instruction> &instructions,
                       symbol_table::symbol_table &table);
val process_unary_node(const std::unique_ptr<parser::unary_node> &node,
                       std::vector<instruction> &instructions,
                       symbol_table::symbol_table &table);
unary_operator process_unary_operator(const wccff::unary_operator &op);

void process_variable_declaration(const wccff::parser::variable_declaration &s,
                                  std::vector<instruction> &instructions,
                                  symbol_table::symbol_table &table);

void process_while_statement(const std::unique_ptr<parser::while_statement> &node,
                             std::vector<instruction> &instructions,
                             symbol_table::symbol_table &table);

program process(const parser::program &input, symbol_table::symbol_table &table);

std::string pretty_print(const binary_statement &i, int32_t ident = 0);
std::string pretty_print(const constant &val, int32_t ident = 0);
std::string pretty_print(const copy_statement &i, int32_t ident = 0);
std::string pretty_print(const double_to_int &i, int32_t ident = 0);
std::string pretty_print(const double_to_uint &i, int32_t ident = 0);
std::string pretty_print(const initial &node, int32_t ident = 0);
std::string pretty_print(const instruction &instruction, int32_t ident = 0);
std::string pretty_print(const fun_call &f, int32_t ident = 0);
std::string pretty_print(const function_definition &f, int32_t ident = 0);
std::string pretty_print(const int_to_double &instruction, int32_t ident = 0);
std::string pretty_print(const jump_statement &i, int32_t ident = 0);
std::string pretty_print(const jump_if_zero_statement &i, int32_t ident = 0);
std::string pretty_print(const jump_if_not_zero_statement &i, int32_t ident = 0);
std::string pretty_print(const label_statement &i, int32_t ident = 0);
std::string pretty_print(const program &p, int32_t ident = 0);
std::string pretty_print(const return_statement &instruction, int32_t ident = 0);
std::string pretty_print(const sing_extend &node, int32_t ident = 0);
std::string pretty_print(const static_variable &top, int32_t ident = 0);
std::string pretty_print(const std::vector<instruction> &instructions, int32_t ident = 0);
std::string pretty_print(const top_level &top, int32_t ident = 0);
std::string pretty_print(const truncate &node, int32_t ident = 0);
std::string pretty_print(const uint_to_double &instruction, int32_t ident = 0);
std::string pretty_print(const unary_statement &instruction, int32_t ident = 0);
std::string pretty_print(const var &val, int32_t ident = 0);
std::string pretty_print(const val &val, int32_t ident = 0);
std::string pretty_print(const zero_extend &node, int32_t ident = 0);

} // namespace wccff::tacky
#endif // TACKY_H
