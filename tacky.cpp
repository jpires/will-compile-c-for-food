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

#include "tacky.h"
#include "assembly_generation.h"
#include "utils.h"
#include "visitor.h"
#include <fmt/format.h>

namespace wccff::tacky {

std::string get_temporary_name()
{
    static int counter = 0;
    return fmt::format("tacky-{}", ++counter);
}

identifier get_and_false_label()
{
    static int counter = 0;
    return { fmt::format("and_false_{}", ++counter) };
}

identifier get_and_end_label()
{
    static int counter = 0;
    return { fmt::format("and_end_{}", ++counter) };
}

identifier get_conditional_e2_label()
{
    static int counter = 0;
    return { fmt::format("conditional_e2_{}", ++counter) };
}

identifier get_conditional_end_label()
{
    static int counter = 0;
    return { fmt::format("conditional_end_{}", ++counter) };
}

identifier get_if_else_label()
{
    static int counter = 0;
    return { fmt::format("if_else_{}", ++counter) };
}

identifier get_if_end_label()
{
    static int counter = 0;
    return { fmt::format("if_end_{}", ++counter) };
}

identifier get_or_false_label()
{
    static int counter = 0;
    return { fmt::format("or_true_{}", ++counter) };
}

identifier get_or_end_label()
{
    static int counter = 0;
    return { fmt::format("or_end_{}", ++counter) };
}

identifier get_loop_continue_label(identifier n)
{
    return { fmt::format("continue_{}", n.name) };
}
identifier get_loop_break_label(identifier n)
{
    return { fmt::format("break_{}", n.name) };
}

var make_temporary_variable(const type &t, symbol_table::symbol_table &table)
{
    auto name = get_temporary_name();
    table.add(identifier{ name }, t, symbol_table::local_attributes{});
    return var{ name };
}

val process_assignment_node(const std::unique_ptr<parser::assignment_node> &node,
                            std::vector<instruction> &instructions,
                            symbol_table::symbol_table &table)
{
    auto right = process_expression(node->rhs, instructions, table);
    // The previous pass ensures that the left side of an assignment is a var.
    // If it's not, then terminate.
    auto &left = std::get<parser::var>(node->lhs);
    auto dst = var{ left.name };
    instructions.emplace_back(copy_statement{ right, dst });
    return dst;
}

void process_block(const parser::block &node, std::vector<instruction> &instructions, symbol_table::symbol_table &table)
{
    for (const auto &child : node.items)
    {
        process_block_item(child, instructions, table);
    }
}

val process_conditional_node(const std::unique_ptr<parser::conditional_node> &node,
                             std::vector<instruction> &instructions,
                             symbol_table::symbol_table &table)
{
    auto e2_label = identifier{ get_conditional_e2_label() };
    auto end_label = identifier{ get_conditional_end_label() };
    auto result = make_temporary_variable(get_type(node), table);
    auto c = process_expression(node->condition, instructions, table);

    instructions.emplace_back(jump_if_zero_statement{ c, e2_label });
    auto r_e1 = process_expression(node->e1, instructions, table);
    instructions.emplace_back(copy_statement{ r_e1, result });
    instructions.emplace_back(jump_statement{ end_label });
    instructions.emplace_back(label_statement{ e2_label });
    auto r_e2 = process_expression(node->e2, instructions, table);
    instructions.emplace_back(copy_statement{ r_e2, result });
    instructions.emplace_back(label_statement{ end_label });

    return result;
}

constant process_constant(const constant &node)
{
    return std::visit(visitor{
                        [](const int_constant &c) -> constant { return c; },
                        [](const long_constant &c) -> constant { return c; },
                        [](const unsigned_int_constant &c) -> constant { return c; },
                        [](const unsigned_long_constant &c) -> constant { return c; },
                        [](const double_constant &c) -> constant { return c; },
                        [](const auto &) -> constant { throw std::runtime_error{ "Not Implemented" }; },
                      },
                      node);
}

void process_if(const std::unique_ptr<parser::if_node> &node,
                std::vector<instruction> &instructions,
                symbol_table::symbol_table &table)
{
    // Create both else and end label, so that the numbering don't get out of sync.
    // i.e. both labels will have the same number no matter if the 'if' doesn't have an 'else'
    auto else_label = identifier{ get_if_else_label() };
    auto end_label = identifier{ get_if_end_label() };
    auto result = process_expression(node->op, instructions, table);

    if (node->else_stmt.has_value())
    {
        instructions.emplace_back(jump_if_zero_statement{ result, else_label });
        process_statement(node->then_stmt, instructions, table);
        instructions.emplace_back(jump_statement{ end_label });
        instructions.emplace_back(label_statement{ else_label });
        process_statement(node->else_stmt.value(), instructions, table);
        instructions.emplace_back(label_statement{ end_label });
    }
    else
    {
        instructions.emplace_back(jump_if_zero_statement{ result, end_label });
        process_statement(node->then_stmt, instructions, table);
        instructions.emplace_back(label_statement{ end_label });
    }
}

unary_operator process_unary_operator(const wccff::unary_operator &op)
{
    return std::visit(visitor{
                        [](const wccff::postfix_decrement_operator &) -> unary_operator {
                            throw std::logic_error("logical_not_operator");
                        },
                        [](const wccff::postfix_increment_operator &) -> unary_operator {
                            throw std::logic_error("logical_not_operator");
                        },
                        [](const wccff::prefix_decrement_operator &) -> unary_operator {
                            throw std::logic_error("logical_not_operator");
                        },
                        [](const wccff::prefix_increment_operator &) -> unary_operator {
                            throw std::logic_error("logical_not_operator");
                        },
                        [](const auto &n) -> unary_operator { return n; },
                      },
                      op);
}

wccff::binary_operator process_binary_operator(const wccff::binary_operator &op)
{
    return std::visit(
      visitor{
        [](const wccff::logical_and_operator &) -> binary_operator {
            throw std::logic_error("logical and operator Not implemented");
        },
        [](const wccff::logical_or_operator &) -> binary_operator {
            throw std::logic_error("logical or operator Not implemented");
        },
        [](const wccff::assignment_operator &) -> binary_operator {
            throw std::logic_error("Assignment operator not converted");
        },
        [](const wccff::compound_plus_operator &) -> binary_operator { return plus_operator{}; },
        [](const wccff::compound_subtract_operator &) -> binary_operator { return subtract_operator{}; },
        [](const wccff::compound_multiply_operator &) -> binary_operator { return multiply_operator{}; },
        [](const wccff::compound_divide_operator &) -> binary_operator { return divide_operator{}; },
        [](const wccff::compound_remainder_operator &) -> binary_operator { return remainder_operator{}; },
        [](const wccff::compound_bitwise_and_operator &) -> binary_operator { return bitwise_and_operator{}; },
        [](const wccff::compound_bitwise_or_operator &) -> binary_operator { return bitwise_or_operator{}; },
        [](const wccff::compound_bitwise_xor_operator &) -> binary_operator { return bitwise_xor_operator{}; },
        [](const wccff::compound_left_shift_operator &) -> binary_operator { return left_shift_operator{}; },
        [](const wccff::compound_right_shift_operator &) -> binary_operator { return right_shift_operator{}; },
        [](const auto &n) -> binary_operator { return n; },
      },
      op);
}

val process_prefix_unary(const std::unique_ptr<parser::unary_node> &node,
                         std::vector<instruction> &instructions,
                         symbol_table::symbol_table &table)
{
    binary_operator op;
    if (std::holds_alternative<prefix_increment_operator>(node->op))
    {
        op = plus_operator{};
    }
    else
    {
        op = subtract_operator{};
    }
    auto src = process_expression(node->exp, instructions, table);
    auto dst = make_temporary_variable(get_type(node), table);
    // ToDo: Change the int_constant to a generic constant
    instructions.emplace_back(binary_statement{ op, src, int_constant{ 1 }, dst });
    instructions.emplace_back(copy_statement(dst, src));
    return dst;
}

val process_postfix_unary(const std::unique_ptr<parser::unary_node> &node,
                          std::vector<instruction> &instructions,
                          symbol_table::symbol_table &table)
{
    binary_operator op;
    if (std::holds_alternative<postfix_increment_operator>(node->op))
    {
        op = plus_operator{};
    }
    else
    {
        op = subtract_operator{};
    }
    auto src = process_expression(node->exp, instructions, table);
    auto dst = make_temporary_variable(get_type(node), table);
    auto tmp = make_temporary_variable(get_type(node), table);
    instructions.emplace_back(copy_statement(src, dst));
    // ToDo: Change the int_constant to a generic constant
    instructions.emplace_back(binary_statement{ op, src, int_constant{ 1 }, tmp });
    instructions.emplace_back(copy_statement(tmp, src));
    return dst;
}

val process_unary_node(const std::unique_ptr<parser::unary_node> &node,
                       std::vector<instruction> &instructions,
                       symbol_table::symbol_table &table)
{
    if (std::holds_alternative<prefix_increment_operator>(node->op) ||
        std::holds_alternative<prefix_decrement_operator>(node->op))
    {
        return process_prefix_unary(node, instructions, table);
    }

    if (std::holds_alternative<postfix_increment_operator>(node->op) ||
        std::holds_alternative<postfix_decrement_operator>(node->op))
    {
        return process_postfix_unary(node, instructions, table);
    }

    auto src = process_expression(node->exp, instructions, table);
    auto dst = make_temporary_variable(get_type(node), table);
    auto op = process_unary_operator(node->op);
    instructions.emplace_back(unary_statement{ op, src, dst });
    return dst;
}

val process_binary_and(const std::unique_ptr<parser::binary_node> &node,
                       std::vector<instruction> &instructions,
                       symbol_table::symbol_table &table)
{
    auto false_end_label = get_and_false_label();
    auto end_label = get_and_end_label();
    auto dst = make_temporary_variable(get_type(node), table);

    auto v1 = process_expression(node->left, instructions, table);
    instructions.emplace_back(jump_if_zero_statement{ v1, false_end_label });
    auto v2 = process_expression(node->right, instructions, table);
    instructions.emplace_back(jump_if_zero_statement{ v2, false_end_label });
    instructions.emplace_back(copy_statement{ int_constant{ 1 }, dst });
    instructions.emplace_back(jump_statement{ end_label });
    instructions.emplace_back(label_statement{ false_end_label });
    instructions.emplace_back(copy_statement{ int_constant{ 0 }, dst });
    instructions.emplace_back(label_statement{ end_label });
    return dst;
}

val process_binary_or(const std::unique_ptr<parser::binary_node> &node,
                      std::vector<instruction> &instructions,
                      symbol_table::symbol_table &table)
{
    auto false_end_label = get_or_false_label();
    auto end_label = get_or_end_label();
    auto dst = make_temporary_variable(get_type(node), table);

    auto v1 = process_expression(node->left, instructions, table);
    instructions.emplace_back(jump_if_not_zero_statement{ v1, false_end_label });
    auto v2 = process_expression(node->right, instructions, table);
    instructions.emplace_back(jump_if_not_zero_statement{ v2, false_end_label });
    instructions.emplace_back(copy_statement{ int_constant{ 0 }, dst });
    instructions.emplace_back(jump_statement{ end_label });
    instructions.emplace_back(label_statement{ false_end_label });
    instructions.emplace_back(copy_statement{ int_constant{ 1 }, dst });
    instructions.emplace_back(label_statement{ end_label });
    return dst;
}

val process_binary_node(const std::unique_ptr<parser::binary_node> &node,
                        std::vector<instruction> &instructions,
                        symbol_table::symbol_table &table)
{
    if (std::holds_alternative<logical_and_operator>(node->op))
    {
        return process_binary_and(node, instructions, table);
    }
    if (std::holds_alternative<logical_or_operator>(node->op))
    {
        return process_binary_or(node, instructions, table);
    }

    auto v1 = process_expression(node->left, instructions, table);
    auto v2 = process_expression(node->right, instructions, table);
    auto dst = make_temporary_variable(get_type(node), table);
    auto op = process_binary_operator(node->op);
    instructions.emplace_back(binary_statement{ op, v1, v2, dst });
    return dst;
}

val process_expression(const wccff::parser::expression &exp,
                       std::vector<instruction> &instructions,
                       symbol_table::symbol_table &table)
{
    return std::visit(visitor{
                        [](const constant &c) -> val { return process_constant(c); },
                        [](const parser::var &c) -> val { return var{ c.name }; },
                        [&instructions, &table](const std::unique_ptr<parser::unary_node> &n) -> val {
                            return process_unary_node(n, instructions, table);
                        },
                        [&instructions, &table](const std::unique_ptr<parser::binary_node> &n) -> val {
                            return process_binary_node(n, instructions, table);
                        },
                        [&instructions, &table](const std::unique_ptr<parser::cast_expression> &n) -> val {
                            return process_cast_expression(n, instructions, table);
                        },
                        [&instructions, &table](const std::unique_ptr<parser::assignment_node> &n) -> val {
                            return process_assignment_node(n, instructions, table);
                        },
                        [&instructions, &table](const std::unique_ptr<parser::conditional_node> &n) -> val {
                            return process_conditional_node(n, instructions, table);
                        },
                        [&instructions, &table](const std::unique_ptr<parser::function_call> &n) -> val {
                            return process_function_call(n, instructions, table);
                        },
                      },
                      exp);
}

void process_return_node(const wccff::parser::return_node &stmt,
                         std::vector<instruction> &instructions,
                         symbol_table::symbol_table &table)
{
    auto node = return_statement{ process_expression(stmt.e, instructions, table) };
    instructions.emplace_back(return_statement{ node });
}

void process_statement(const wccff::parser::statement &s,
                       std::vector<instruction> &instructions,
                       symbol_table::symbol_table &table)
{
    std::visit(
      visitor{
        [&instructions, &table](const parser::return_node &n) { process_return_node(n, instructions, table); },
        [&instructions, &table](const parser::expression &n) { process_expression(n, instructions, table); },
        [&](const std::unique_ptr<parser::if_node> &n) { process_if(n, instructions, table); },
        [&](const std::unique_ptr<parser::compound_statement> &n) {
            process_compound_statement(n, instructions, table);
        },
        [&](const parser::break_statement &n) { process_break_statement(n, instructions); },
        [&](const parser::continue_statement &n) { process_continue_statement(n, instructions); },
        [&](const parser::goto_statement &n) { process_goto_statement(n, instructions); },
        [&](const std::unique_ptr<parser::while_statement> &n) { process_while_statement(n, instructions, table); },
        [&](const std::unique_ptr<parser::do_while_statement> &n) {
            process_do_while_statement(n, instructions, table);
        },
        [&](const std::unique_ptr<parser::for_statement> &n) { process_for_statement(n, instructions, table); },
        [&](const std::unique_ptr<parser::labelled_statement> &n) {
            return process_labeled_statement(n, instructions, table);
        },
        [](const std::monostate) {},
      },
      s);
}

void process_block_item(const parser::block_item &s,
                        std::vector<instruction> &instructions,
                        symbol_table::symbol_table &table)
{
    std::visit(visitor{
                 [&instructions, &table](const parser::declaration &n) { process_declaration(n, instructions, table); },
                 [&instructions, &table](const parser::statement &n) { process_statement(n, instructions, table); },
                 [](const std::monostate n) {},
               },
               s);
}

void process_break_statement(const parser::break_statement &node, std::vector<instruction> &instructions)
{
    identifier label = get_loop_break_label(node.label);
    instructions.emplace_back(jump_statement{ label });
}

val process_cast_expression(const std::unique_ptr<parser::cast_expression> &node,
                            std::vector<instruction> &instructions,
                            symbol_table::symbol_table &table)
{
    auto src = process_expression(node->exp, instructions, table);
    if (node->target == get_type(node->exp))
    {
        return src;
    }

    auto dst = make_temporary_variable(node->target, table);

    if (std::holds_alternative<double_type>(node->target))
    {
        if (is_signed_type(get_type(node->exp)))
        {
            instructions.emplace_back(int_to_double{ src, dst });
        }
        else
        {
            instructions.emplace_back(uint_to_double{ src, dst });
        }
    }
    else if (std::holds_alternative<double_type>(get_type(node->exp)))
    {
        if (is_signed_type(node->target))
        {
            instructions.emplace_back(double_to_int{ src, dst });
        }
        else
        {
            instructions.emplace_back(double_to_uint{ src, dst });
        }
    }
    else if (get_type_size(node->target) == get_type_size(get_type(node->exp)))
    {
        instructions.emplace_back(copy_statement{ src, dst });
    }
    else if (get_type_size(node->target) < get_type_size(get_type(node->exp)))
    {
        instructions.emplace_back(truncate{ src, dst });
    }
    else if (is_signed_type(get_type(node->exp)))
    {
        instructions.emplace_back(sing_extend{ src, dst });
    }
    else
    {
        instructions.emplace_back(zero_extend{ src, dst });
    }

    return dst;
}

void process_compound_statement(const std::unique_ptr<parser::compound_statement> &node,
                                std::vector<instruction> &instructions,
                                symbol_table::symbol_table &table)
{
    process_block(node->block, instructions, table);
}

void process_continue_statement(const parser::continue_statement &node, std::vector<instruction> &instructions)
{
    identifier label = get_loop_continue_label(node.label);
    instructions.emplace_back(jump_statement{ label });
}

void process_declaration(const parser::declaration &node,
                         std::vector<instruction> &instructions,
                         symbol_table::symbol_table &table)
{
    std::visit(visitor{
                 [&table](const parser::function_declaration &node) {
                     if (process_function_definition(node, table).has_value())
                     {
                         throw std::runtime_error("Found unexpected function definition");
                     }
                 },
                 [&instructions, &table](const parser::variable_declaration &node) {
                     process_variable_declaration(node, instructions, table);
                 },
               },
               node);
}

void process_do_while_statement(const std::unique_ptr<parser::do_while_statement> &node,
                                std::vector<instruction> &instructions,
                                symbol_table::symbol_table &table)
{
    auto start_label = identifier{ fmt::format("start_{}", node->label.name) };
    auto continue_label = get_loop_continue_label(node->label);
    auto break_label = get_loop_break_label(node->label);

    instructions.emplace_back(label_statement{ start_label });
    process_statement(node->body, instructions, table);
    instructions.emplace_back(label_statement{ continue_label });
    auto result = process_expression(node->condition, instructions, table);
    instructions.emplace_back(jump_if_not_zero_statement{ result, start_label });
    instructions.emplace_back(label_statement{ break_label });
}

void process_for_init(const parser::for_init &node,
                      std::vector<instruction> &instructions,
                      symbol_table::symbol_table &table)
{
    std::visit(visitor{
                 [&instructions, &table](const parser::init_declaration &n) {
                     process_variable_declaration(n.decl, instructions, table);
                 },
                 [&instructions, &table](const parser::init_expression &n) {
                     if (n.expression.has_value())
                     {
                         process_expression(n.expression.value(), instructions, table);
                     }
                 },
               },
               node);
}

void process_for_statement(const std::unique_ptr<parser::for_statement> &node,
                           std::vector<instruction> &instructions,
                           symbol_table::symbol_table &table)
{
    auto start_label = identifier{ fmt::format("start_{}", node->label.name) };
    auto continue_label = get_loop_continue_label(node->label);
    auto break_label = get_loop_break_label(node->label);

    process_for_init(node->init, instructions, table);
    instructions.emplace_back(label_statement{ start_label });
    if (node->condition.has_value())
    {
        auto result = process_expression(node->condition.value(), instructions, table);
        instructions.emplace_back(jump_if_zero_statement{ result, break_label });
    }

    process_statement(node->body, instructions, table);

    instructions.emplace_back(label_statement{ continue_label });
    if (node->post.has_value())
    {
        process_expression(node->post.value(), instructions, table);
    }
    instructions.emplace_back(jump_statement{ start_label });
    instructions.emplace_back(label_statement{ break_label });
}

val process_function_call(const std::unique_ptr<parser::function_call> &f,
                          std::vector<instruction> &instructions,
                          symbol_table::symbol_table &table)
{
    std::vector<val> arguments;
    arguments.reserve(f->arguments.size());

    for (const auto &arg : f->arguments)
    {
        auto r = process_expression(arg, instructions, table);
        arguments.emplace_back(r);
    }

    auto dst = make_temporary_variable(get_type(f), table);
    instructions.emplace_back(fun_call{ f->name, std::move(arguments), dst });

    return dst;
}
std::optional<function_definition> process_function_definition(const parser::function_declaration &f,
                                                               symbol_table::symbol_table &table)
{
    if (f.body.has_value())
    {
        std::vector<instruction> instructions;
        std::vector<identifier> params;
        params.reserve(f.arguments.size());

        for (const auto &p : f.arguments)
        {
            params.emplace_back(p);
        }

        process_block(f.body.value(), instructions, table);
        instructions.emplace_back(return_statement{ int_constant{ 0 } });

        auto attrs = std::get<symbol_table::func_attributes>(table.get(f.name)->attrs);
        return function_definition{ f.name, attrs.is_global, std::move(params), std::move(instructions) };
    }

    return std::nullopt;
}

void process_goto_statement(const parser::goto_statement &node, std::vector<instruction> &instructions)
{
    instructions.emplace_back(jump_statement{ node.label });
}

void process_labeled_statement(const std::unique_ptr<parser::labelled_statement> &id,
                               std::vector<instruction> &instructions,
                               symbol_table::symbol_table &table)
{
    auto start_label = identifier{ fmt::format("{}", id->label.name) };
    instructions.emplace_back(label_statement{ start_label });
    process_statement(id->body, instructions, table);
}

program process(const parser::program &input, symbol_table::symbol_table &table)
{
    std::vector<top_level> functions;
    for (const auto &f : input.f)
    {
        if (std::holds_alternative<parser::function_declaration>(f))
        {
            if (auto tmp = process_function_definition(std::get<parser::function_declaration>(f), table);
                tmp.has_value())
            {
                functions.emplace_back(tmp.value());
            }
        }
    }

    std::vector<top_level> tacky_definition;
    for (const auto &[_, symbol] : table)
    {
        if (std::holds_alternative<symbol_table::static_attributes>(symbol.attrs))
        {
            auto attrs = std::get<symbol_table::static_attributes>(symbol.attrs);
            if (std::holds_alternative<initial>(attrs.init))
            {
                auto initial = std::get<wccff::initial>(attrs.init);
                tacky_definition.push_back(
                  static_variable{ symbol.name, attrs.is_global, copy_type(symbol.type), initial });
            }
            else if (std::holds_alternative<symbol_table::tentative>(attrs.init))
            {

                tacky_definition.push_back(static_variable{ symbol.name,
                                                            attrs.is_global,
                                                            copy_type(symbol.type),
                                                            get_default_initial(symbol.type) });
            }
        }
    }

    tacky_definition.insert(tacky_definition.end(),
                            std::make_move_iterator(functions.begin()),
                            std::make_move_iterator(functions.end()));

    return { std::move(tacky_definition) };
}

void process_variable_declaration(const wccff::parser::variable_declaration &s,
                                  std::vector<instruction> &instructions,
                                  symbol_table::symbol_table &table)
{
    if (s.init.has_value() == false)
    {
        return;
    }

    if (std::holds_alternative<symbol_table::static_attributes>(table.get(s.name)->attrs))
    {
        return;
    }

    auto src = process_expression(s.init.value(), instructions, table);
    auto dst = var{ s.name };
    instructions.emplace_back(copy_statement{ src, dst });
}

void process_while_statement(const std::unique_ptr<parser::while_statement> &node,
                             std::vector<instruction> &instructions,
                             symbol_table::symbol_table &table)
{
    auto continue_label = get_loop_continue_label(node->label);
    auto break_label = get_loop_break_label(node->label);

    instructions.emplace_back(label_statement{ continue_label });
    auto result = process_expression(node->condition, instructions, table);
    instructions.emplace_back(jump_if_zero_statement{ result, break_label });
    process_statement(node->body, instructions, table);
    instructions.emplace_back(jump_statement{ continue_label });
    instructions.emplace_back(label_statement{ break_label });
}

std::string pretty_print(const constant &val, int32_t ident)
{
    return std::visit(
      visitor{
        [ident](const double_constant &val) { return wccff::format_indented(ident, "DoubleConstant({})", val.value); },
        [ident](const int_constant &val) { return wccff::format_indented(ident, "IntConstant({})", val.value); },
        [ident](const long_constant &val) { return wccff::format_indented(ident, "LongConstant({})", val.value); },
        [ident](const unsigned_int_constant &val) {
            return wccff::format_indented(ident, "UIntConstant({})", val.value);
        },
        [ident](const unsigned_long_constant &val) {
            return wccff::format_indented(ident, "ULongConstant({})", val.value);
        },
        [](const auto &) -> std::string { throw std::runtime_error("Not implemented"); },
      },
      val);
}
std::string pretty_print(const var &var, int32_t ident)
{
    return wccff::format_indented(ident, "Var({})", var.id.name);
}
std::string pretty_print(const val &val, int32_t ident)
{
    return std::visit(visitor{ [ident](const auto &n) { return pretty_print(n, ident); } }, val);
}
std::string pretty_print(const return_statement &instruction, int32_t ident)
{
    return wccff::format_indented(ident, "Return({})\n", pretty_print(instruction.val, 0));
}
std::string pretty_print(const sing_extend &node, int32_t ident)
{
    return wccff::format_indented(ident,
                                  "SignExtend(src={}, dst={})\n",
                                  pretty_print(node.src, 0),
                                  pretty_print(node.dst, 0));
}
std::string pretty_print(const static_variable &top, int32_t ident)
{
    return wccff::format_indented(ident,
                                  "StaticVar(name={}, global={}, init={})\n",
                                  top.name.name,
                                  top.global,
                                  wccff::pretty_print(top.init));
}
std::string pretty_print(const top_level &top, int32_t ident)
{
    return std::visit(visitor{ [ident](const function_definition &n) { return pretty_print(n, ident); },
                               [ident](const static_variable &n) { return pretty_print(n, ident); } },
                      top);
}

std::string pretty_print(const truncate &node, int32_t ident)
{
    return wccff::format_indented(ident,
                                  "Truncate(src={}, dst={})\n",
                                  pretty_print(node.src, 0),
                                  pretty_print(node.dst, 0));
}

std::string pretty_print(const unary_statement &i, int32_t ident)
{
    return wccff::format_indented(ident,
                                  "Unary({}, {}, {})\n",
                                  pretty_print(i.op, 0),
                                  pretty_print(i.src, 0),
                                  pretty_print(i.dst, 0));
}
std::string pretty_print(const binary_statement &i, int32_t ident)
{
    return wccff::format_indented(ident,
                                  "Binary({}, {}, {}, {})\n",
                                  pretty_print(i.op, 0),
                                  pretty_print(i.src1, 0),
                                  pretty_print(i.src2, 0),
                                  pretty_print(i.dst, 0));
}

std::string pretty_print(const copy_statement &i, int32_t ident)
{
    return wccff::format_indented(ident, "Copy({}, {})\n", pretty_print(i.src, 0), pretty_print(i.dst, 0));
}

std::string pretty_print(const double_to_int &i, int32_t ident)
{
    return wccff::format_indented(ident, "Double2Int({}, {})\n", pretty_print(i.src, 0), pretty_print(i.dst, 0));
}

std::string pretty_print(const double_to_uint &i, int32_t ident)
{
    return wccff::format_indented(ident, "Double2UInt({}, {})\n", pretty_print(i.src, 0), pretty_print(i.dst, 0));
}

std::string pretty_print(const initial &node, int32_t ident)
{
    return std::visit(
      visitor{
        [ident](const double_initial &val) { return wccff::format_indented(ident, "DoubleInitial({})", val.value); },
        [ident](const int_initial &val) { return wccff::format_indented(ident, "IntInitial({})", val.value); },
        [ident](const long_initial &val) { return wccff::format_indented(ident, "LongInitial({})", val.value); },
        [ident](const unsigned_int_initial &val) {
            return wccff::format_indented(ident, "UIntInitial({})", val.value);
        },
        [ident](const unsigned_long_initial &val) {
            return wccff::format_indented(ident, "ULongInitial({})", val.value);
        },
      },
      node);
}

std::string pretty_print(const fun_call &f, int32_t ident)
{
    std::string params;
    for (const auto &arg : f.args)
    {
        params += pretty_print(arg, 0);
    }

    return wccff::format_indented(ident, "FUNCALL({}, ({}), {})\n", f.fun_name.name, params, pretty_print(f.dst));
}

std::string pretty_print(const int_to_double &i, int32_t ident)
{
    return wccff::format_indented(ident, "Int2Double({}, {})\n", pretty_print(i.src, 0), pretty_print(i.dst, 0));
}

std::string pretty_print(const jump_statement &i, int32_t ident)
{
    return wccff::format_indented(ident, "Jump({})\n", i.target.name);
}
std::string pretty_print(const jump_if_zero_statement &i, int32_t ident)
{
    return wccff::format_indented(ident, "JumpIfZero({}, {})\n", pretty_print(i.condition, 0), i.target.name);
}
std::string pretty_print(const jump_if_not_zero_statement &i, int32_t ident)
{
    return wccff::format_indented(ident, "JumpIfNotZero({}, {})\n", pretty_print(i.condition, 0), i.target.name);
}
std::string pretty_print(const label_statement &i, int32_t ident)
{
    return wccff::format_indented(ident, "Label({})\n", i.target.name);
}

std::string pretty_print(const instruction &instruction, int32_t ident)
{
    return std::visit(
      visitor{
        [ident](const return_statement &n) { return pretty_print(n, ident); },
        [ident](const unary_statement &n) { return pretty_print(n, ident); },
        [ident](const binary_statement &n) { return pretty_print(n, ident); },
        [ident](const copy_statement &n) { return pretty_print(n, ident); },
        [ident](const double_to_int &n) { return pretty_print(n, ident); },
        [ident](const double_to_uint &n) { return pretty_print(n, ident); },
        [ident](const int_to_double &n) { return pretty_print(n, ident); },
        [ident](const jump_statement &n) { return pretty_print(n, ident); },
        [ident](const jump_if_zero_statement &n) { return pretty_print(n, ident); },
        [ident](const jump_if_not_zero_statement &n) { return pretty_print(n, ident); },
        [ident](const label_statement &n) { return pretty_print(n, ident); },
        [ident](const fun_call &n) { return pretty_print(n, ident); },
        [ident](const sing_extend &n) { return pretty_print(n, ident); },
        [ident](const truncate &n) { return pretty_print(n, ident); },
        [ident](const uint_to_double &n) { return pretty_print(n, ident); },
        [ident](const zero_extend &n) { return pretty_print(n, ident); },
        [](const auto &) -> std::string { throw std::logic_error(wccff::get_not_implemented_message()); },
      },
      instruction);
}

std::string pretty_print(const std::vector<instruction> &instructions, int32_t ident)
{
    std::string output;
    for (const auto &i : instructions)
    {
        output += pretty_print(i, ident);
    }

    return output;
}

std::string pretty_print(const function_definition &f, int ident)
{
    return wccff::format_indented(ident,
                                  "Function({}, global={})\n{}",
                                  f.name.name,
                                  f.global,
                                  pretty_print(f.instructions, ident + 4));
}

std::string pretty_print(const program &p, int ident)
{
    std::string output;
    for (const auto &f : p.function)
    {
        output += pretty_print(f, ident);
    }
    return output;
}

std::string pretty_print(const uint_to_double &i, int32_t ident)
{
    return wccff::format_indented(ident, "UInt2Double({}, {})\n", pretty_print(i.src, 0), pretty_print(i.dst, 0));
}

std::string pretty_print(const zero_extend &node, int32_t ident)
{
    return wccff::format_indented(ident,
                                  "ZeroExtend(src={}, dst={})\n",
                                  pretty_print(node.src, 0),
                                  pretty_print(node.dst, 0));
}
} // namespace wccff::tacky
