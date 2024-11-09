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

/*

auto v1 = process_expression(node->left, instructions);
auto v2 = process_expression(node->right, instructions);
auto dst = var{ get_temporary_name() };
auto op = process_binary_operator(node->op);
instructions.emplace_back(binary_statement{ op, v1, v2, dst });
return dst;*/

val process_assignment_node(const std::unique_ptr<parser::assignment_node> &node,
                            std::vector<instruction> &instructions)
{
    if (std::holds_alternative<parser::assignment_operator>(node->op))
    {
        auto right = process_expression(node->rhs, instructions);
        // The previous pass ensures that the left side of an assignment is a var.
        // If it's not, then just terminate.
        auto left = std::get<parser::var>(node->lhs);
        auto dst = var{ process_identifier(left.name) };
        instructions.emplace_back(copy_statement{ right, dst });
        return dst;
    }

    auto right = process_expression(node->rhs, instructions);
    // The previous pass ensures that the left side of an assignment is a var.
    // If it's not, then just terminate.
    auto left = std::get<parser::var>(node->lhs);
    auto dst = var{ process_identifier(left.name) };
    auto tmp = var{ get_temporary_name() };
    auto op = process_binary_operator(node->op);
    instructions.emplace_back(binary_statement{ op, dst, right, tmp });
    instructions.emplace_back(copy_statement{ tmp, dst });
    return dst;

    // Process right side.
    // Process Left side
    // Ren Binary Operation, of left and right save in tmp
    // copy tmp to left
}

void process_block(const parser::block &node, std::vector<instruction> &instructions)
{
    for (const auto &child : node.items)
    {
        process_block_item(child, instructions);
    }
}

val process_conditional_node(const std::unique_ptr<parser::conditional_node> &node,
                             std::vector<instruction> &instructions)
{
    auto e2_label = identifier{ get_conditional_e2_label() };
    auto end_label = identifier{ get_conditional_end_label() };
    auto result = var{ get_temporary_name() };
    auto c = process_expression(node->condition, instructions);

    instructions.emplace_back(jump_if_zero_statement{ c, e2_label });
    auto r_e1 = process_expression(node->e1, instructions);
    instructions.emplace_back(copy_statement{ r_e1, result });
    instructions.emplace_back(jump_statement{ end_label });
    instructions.emplace_back(label_statement{ e2_label });
    auto r_e2 = process_expression(node->e2, instructions);
    instructions.emplace_back(copy_statement{ r_e2, result });
    instructions.emplace_back(label_statement{ end_label });

    return result;
}

identifier process_identifier(const parser::identifier &id)
{
    return { id.name };
}

void process_if(const std::unique_ptr<parser::if_node> &node, std::vector<instruction> &instructions)
{
    // Create both else and end label, so that the numbering don't get out of sync.
    // i.e. both labels will have the same number no matter if the 'if' doesn't have an 'else'
    auto else_label = identifier{ get_if_else_label() };
    auto end_label = identifier{ get_if_end_label() };
    auto result = process_expression(node->op, instructions);

    if (node->else_stmt.has_value())
    {
        instructions.emplace_back(jump_if_zero_statement{ result, else_label });
        process_statement(node->then_stmt, instructions);
        instructions.emplace_back(jump_statement{ end_label });
        instructions.emplace_back(label_statement{ else_label });
        process_statement(node->else_stmt.value(), instructions);
        instructions.emplace_back(label_statement{ end_label });
    }
    else
    {
        instructions.emplace_back(jump_if_zero_statement{ result, end_label });
        process_statement(node->then_stmt, instructions);
        instructions.emplace_back(label_statement{ end_label });
    }
}

constant process_int_constant(const parser::int_constant &int_con)
{
    return { int_con.value };
}

unary_operator process_unary_operator(const parser::unary_operator &op)
{
    return std::visit(
      visitor{
        [](const parser::bitwise_complement_operator &) -> unary_operator { return binary_complement_operator{}; },
        [](const parser::negate_operator &) -> unary_operator { return negate_operator{}; },
        [](const parser::logical_not_operator &) -> unary_operator { return not_operator{}; },
        [](const parser::postfix_decrement_operator &) -> unary_operator {
            throw std::logic_error("logical_not_operator");
        },
        [](const parser::postfix_increment_operator &) -> unary_operator {
            throw std::logic_error("logical_not_operator");
        },
        [](const parser::prefix_decrement_operator &) -> unary_operator {
            throw std::logic_error("logical_not_operator");
        },
        [](const parser::prefix_increment_operator &) -> unary_operator {
            throw std::logic_error("logical_not_operator");
        },
      },
      op);
}

binary_operator process_binary_operator(const parser::binary_operator &op)
{
    return std::visit(
      visitor{
        [](const parser::plus_operator &) -> binary_operator { return plus_operator{}; },
        [](const parser::subtract_operator &) -> binary_operator { return subtract_operator{}; },
        [](const parser::multiply_operator &) -> binary_operator { return multiply_operator{}; },
        [](const parser::divide_operator &) -> binary_operator { return divide_operator{}; },
        [](const parser::remainder_operator &) -> binary_operator { return remainder_operator{}; },
        [](const parser::bitwise_and_operator &) -> binary_operator { return binary_and_operator{}; },
        [](const parser::bitwise_or_operator &) -> binary_operator { return binary_or_operator{}; },
        [](const parser::bitwise_xor_operator &) -> binary_operator { return binary_xor_operator{}; },
        [](const parser::left_shift_operator &) -> binary_operator { return left_shift_operator{}; },
        [](const parser::right_shift_operator &) -> binary_operator { return right_shift_operator{}; },
        [](const parser::logical_and_operator &) -> binary_operator {
            throw std::logic_error("logical and operator Not implemented");
        },
        [](const parser::logical_or_operator &) -> binary_operator {
            throw std::logic_error("logical or operator Not implemented");
        },
        [](const parser::equals_operator &) -> binary_operator { return equal_operator{}; },
        [](const parser::not_equals_operator &) -> binary_operator { return not_equal_operator{}; },
        [](const parser::less_than_operator &) -> binary_operator { return less_than_operator{}; },
        [](const parser::less_than_or_equal_operator &) -> binary_operator { return less_than_or_equal_operator{}; },
        [](const parser::greater_than_operator &) -> binary_operator { return greater_than_operator{}; },
        [](const parser::greater_than_or_equal_operator &) -> binary_operator {
            return greater_than_or_equal_operator{};
        },
        [](const parser::assignment_operator &) -> binary_operator {
            throw std::logic_error("Assignment operator not converted");
        },
        [](const parser::compound_plus_operator &) -> binary_operator { return plus_operator{}; },
        [](const parser::compound_subtract_operator &) -> binary_operator { return subtract_operator{}; },
        [](const parser::compound_multiply_operator &) -> binary_operator { return multiply_operator{}; },
        [](const parser::compound_divide_operator &) -> binary_operator { return divide_operator{}; },
        [](const parser::compound_remainder_operator &) -> binary_operator { return remainder_operator{}; },
        [](const parser::compound_bitwise_and_operator &) -> binary_operator { return binary_and_operator{}; },
        [](const parser::compound_bitwise_or_operator &) -> binary_operator { return binary_or_operator{}; },
        [](const parser::compound_bitwise_xor_operator &) -> binary_operator { return binary_xor_operator{}; },
        [](const parser::compound_left_shift_operator &) -> binary_operator { return left_shift_operator{}; },
        [](const parser::compound_right_shift_operator &) -> binary_operator { return right_shift_operator{}; },
      },
      op);
}

val process_prefix_unary(const std::unique_ptr<parser::unary_node> &node, std::vector<instruction> &instructions)
{
    binary_operator op;
    if (std::holds_alternative<parser::prefix_increment_operator>(node->op))
    {
        op = plus_operator{};
    }
    else
    {
        op = subtract_operator{};
    }
    auto src = process_expression(node->exp, instructions);
    auto dst = var{ get_temporary_name() };
    instructions.emplace_back(binary_statement{ op, src, constant{ 1 }, dst });
    instructions.emplace_back(copy_statement(dst, src));
    return dst;
}

val process_postfix_unary(const std::unique_ptr<parser::unary_node> &node, std::vector<instruction> &instructions)
{
    binary_operator op;
    if (std::holds_alternative<parser::postfix_increment_operator>(node->op))
    {
        op = plus_operator{};
    }
    else
    {
        op = subtract_operator{};
    }
    auto src = process_expression(node->exp, instructions);
    auto dst = var{ get_temporary_name() };
    auto tmp = var{ get_temporary_name() };
    instructions.emplace_back(copy_statement(src, dst));
    instructions.emplace_back(binary_statement{ op, src, constant{ 1 }, tmp });
    instructions.emplace_back(copy_statement(tmp, src));
    return dst;
}

val process_unary_node(const std::unique_ptr<parser::unary_node> &node, std::vector<instruction> &instructions)
{
    if (std::holds_alternative<parser::prefix_increment_operator>(node->op) ||
        std::holds_alternative<parser::prefix_decrement_operator>(node->op))
    {
        return process_prefix_unary(node, instructions);
    }

    if (std::holds_alternative<parser::postfix_increment_operator>(node->op) ||
        std::holds_alternative<parser::postfix_decrement_operator>(node->op))
    {
        return process_postfix_unary(node, instructions);
    }

    auto src = process_expression(node->exp, instructions);
    auto dst = var{ get_temporary_name() };
    auto op = process_unary_operator(node->op);
    instructions.emplace_back(unary_statement{ op, src, dst });
    return dst;
}

val process_binary_and(const std::unique_ptr<parser::binary_node> &node, std::vector<instruction> &instructions)
{
    auto false_end_label = get_and_false_label();
    auto end_label = get_and_end_label();
    auto dst = var{ get_temporary_name() };

    auto v1 = process_expression(node->left, instructions);
    instructions.emplace_back(jump_if_zero_statement{ v1, false_end_label });
    auto v2 = process_expression(node->right, instructions);
    instructions.emplace_back(jump_if_zero_statement{ v2, false_end_label });
    instructions.emplace_back(copy_statement{ constant{ 1 }, dst });
    instructions.emplace_back(jump_statement{ end_label });
    instructions.emplace_back(label_statement{ false_end_label });
    instructions.emplace_back(copy_statement{ constant{ 0 }, dst });
    instructions.emplace_back(label_statement{ end_label });
    return dst;
}

val process_binary_or(const std::unique_ptr<parser::binary_node> &node, std::vector<instruction> &instructions)
{
    auto false_end_label = get_or_false_label();
    auto end_label = get_or_end_label();
    auto dst = var{ get_temporary_name() };

    auto v1 = process_expression(node->left, instructions);
    instructions.emplace_back(jump_if_not_zero_statement{ v1, false_end_label });
    auto v2 = process_expression(node->right, instructions);
    instructions.emplace_back(jump_if_not_zero_statement{ v2, false_end_label });
    instructions.emplace_back(copy_statement{ constant{ 0 }, dst });
    instructions.emplace_back(jump_statement{ end_label });
    instructions.emplace_back(label_statement{ false_end_label });
    instructions.emplace_back(copy_statement{ constant{ 1 }, dst });
    instructions.emplace_back(label_statement{ end_label });
    return dst;
}

val process_binary_node(const std::unique_ptr<parser::binary_node> &node, std::vector<instruction> &instructions)
{
    if (std::holds_alternative<parser::logical_and_operator>(node->op))
    {
        return process_binary_and(node, instructions);
    }
    if (std::holds_alternative<parser::logical_or_operator>(node->op))
    {
        return process_binary_or(node, instructions);
    }

    auto v1 = process_expression(node->left, instructions);
    auto v2 = process_expression(node->right, instructions);
    auto dst = var{ get_temporary_name() };
    auto op = process_binary_operator(node->op);
    instructions.emplace_back(binary_statement{ op, v1, v2, dst });
    return dst;
}

val process_expression(const wccff::parser::expression &exp, std::vector<instruction> &instructions)
{
    return std::visit(visitor{
                        [](const parser::int_constant &c) -> val { return process_int_constant(c); },
                        [](const parser::var &c) -> val { return var{ process_identifier(c.name) }; },
                        [&instructions](const std::unique_ptr<parser::unary_node> &n) -> val {
                            return process_unary_node(n, instructions);
                        },
                        [&instructions](const std::unique_ptr<parser::binary_node> &n) -> val {
                            return process_binary_node(n, instructions);
                        },
                        [&instructions](const std::unique_ptr<parser::assignment_node> &n) -> val {
                            return process_assignment_node(n, instructions);
                        },
                        [&instructions](const std::unique_ptr<parser::conditional_node> &n) -> val {
                            return process_conditional_node(n, instructions);
                        },
                        [&instructions](const std::unique_ptr<parser::function_call> &n) -> val {
                            return process_function_call(n, instructions);
                        },
                      },
                      exp);
}

void process_return_node(const wccff::parser::return_node &stmt, std::vector<instruction> &instructions)
{
    auto node = return_statement{ process_expression(stmt.e, instructions) };
    instructions.emplace_back(return_statement{ node });
}

void process_statement(const wccff::parser::statement &s, std::vector<instruction> &instructions)
{
    std::visit(
      visitor{
        [&instructions](const parser::return_node &n) { process_return_node(n, instructions); },
        [&instructions](const parser::expression &n) { process_expression(n, instructions); },
        [&](const std::unique_ptr<parser::if_node> &n) { process_if(n, instructions); },
        [&](const std::unique_ptr<parser::compound_statement> &n) { process_compound_statement(n, instructions); },
        [&](const parser::break_statement &n) { process_break_statement(n, instructions); },
        [&](const parser::continue_statement &n) { process_continue_statement(n, instructions); },
        [&](const parser::goto_statement &n) { process_goto_statement(n, instructions); },
        [&](const std::unique_ptr<parser::while_statement> &n) { process_while_statement(n, instructions); },
        [&](const std::unique_ptr<parser::do_while_statement> &n) { process_do_while_statement(n, instructions); },
        [&](const std::unique_ptr<parser::for_statement> &n) { process_for_statement(n, instructions); },
        [&](const std::unique_ptr<parser::labelled_statement> &n) {
            return process_labeled_statement(n, instructions);
        },
        [](const std::monostate) {},
      },
      s);
}

void process_block_item(const parser::block_item &s, std::vector<instruction> &instructions)
{
    std::visit(visitor{
                 [&instructions](const parser::declaration &n) { process_declaration(n, instructions); },
                 [&instructions](const parser::statement &n) { process_statement(n, instructions); },
                 [](const std::monostate n) {},
               },
               s);
}

void process_break_statement(const parser::break_statement &node, std::vector<instruction> &instructions)
{
    identifier label = get_loop_break_label(process_identifier(node.label));
    instructions.emplace_back(jump_statement{ label });
}
void process_compound_statement(const std::unique_ptr<parser::compound_statement> &node,
                                std::vector<instruction> &instructions)
{
    process_block(node->block, instructions);
}

void process_continue_statement(const parser::continue_statement &node, std::vector<instruction> &instructions)
{
    identifier label = get_loop_continue_label(process_identifier(node.label));
    instructions.emplace_back(jump_statement{ label });
}

void process_declaration(const parser::declaration &node, std::vector<instruction> &instructions)
{
    std::visit(
      visitor{
        [](const parser::function_declaration &node) {
            if (process_function_definition(node).has_value())
            {
                throw std::runtime_error("Found unexpected function definition");
            }
        },
        [&instructions](const parser::variable_declaration &node) { process_variable_declaration(node, instructions); },
      },
      node);
}

void process_do_while_statement(const std::unique_ptr<parser::do_while_statement> &node,
                                std::vector<instruction> &instructions)
{
    auto start_label = identifier{ fmt::format("start_{}", node->label.name) };
    auto continue_label = get_loop_continue_label(process_identifier(node->label));
    auto break_label = get_loop_break_label(process_identifier(node->label));

    instructions.emplace_back(label_statement{ start_label });
    process_statement(node->body, instructions);
    instructions.emplace_back(label_statement{ continue_label });
    auto result = process_expression(node->condition, instructions);
    instructions.emplace_back(jump_if_not_zero_statement{ result, start_label });
    instructions.emplace_back(label_statement{ break_label });
}

void process_for_init(const parser::for_init &node, std::vector<instruction> &instructions)
{
    std::visit(
      visitor{
        [&instructions](const parser::init_declaration &n) { process_variable_declaration(n.decl, instructions); },
        [&instructions](const parser::init_expression &n) {
            if (n.expression.has_value())
            {
                process_expression(n.expression.value(), instructions);
            }
        },
      },
      node);
}

void process_for_statement(const std::unique_ptr<parser::for_statement> &node, std::vector<instruction> &instructions)
{
    auto start_label = identifier{ fmt::format("start_{}", node->label.name) };
    auto continue_label = get_loop_continue_label(process_identifier(node->label));
    auto break_label = get_loop_break_label(process_identifier(node->label));

    process_for_init(node->init, instructions);
    instructions.emplace_back(label_statement{ start_label });
    if (node->condition.has_value())
    {
        auto result = process_expression(node->condition.value(), instructions);
        instructions.emplace_back(jump_if_zero_statement{ result, break_label });
    }

    process_statement(node->body, instructions);

    instructions.emplace_back(label_statement{ continue_label });
    if (node->post.has_value())
    {
        process_expression(node->post.value(), instructions);
    }
    instructions.emplace_back(jump_statement{ start_label });
    instructions.emplace_back(label_statement{ break_label });
}

val process_function_call(const std::unique_ptr<parser::function_call> &f, std::vector<instruction> &instructions)
{
    std::vector<val> arguments;
    arguments.reserve(f->arguments.size());

    for (const auto &arg : f->arguments)
    {
        auto r = process_expression(arg, instructions);
        arguments.emplace_back(r);
    }

    auto dst = var{ get_temporary_name() };
    instructions.emplace_back(fun_call{ process_identifier(f->name), std::move(arguments), dst });

    return dst;
}
std::optional<function_definition> process_function_definition(const parser::function_declaration &f)
{
    if (f.body.has_value())
    {
        std::vector<instruction> instructions;
        std::vector<identifier> params;
        params.reserve(f.arguments.size());

        for (const auto &p : f.arguments)
        {
            params.emplace_back(process_identifier(p));
        }

        process_block(f.body.value(), instructions);
        instructions.emplace_back(return_statement{ constant{ 0 } });

        return function_definition{ process_identifier(f.name), std::move(params), std::move(instructions) };
    }

    return std::nullopt;
}

void process_goto_statement(const parser::goto_statement &node, std::vector<instruction> &instructions)
{
    instructions.emplace_back(jump_statement{ process_identifier(node.label) });
}

void process_labeled_statement(const std::unique_ptr<parser::labelled_statement> &id,
                               std::vector<instruction> &instructions)
{
    auto start_label = identifier{ fmt::format("{}", id->label.name) };
    instructions.emplace_back(label_statement{ start_label });
    process_statement(id->body, instructions);
}

program process(const parser::program &input)
{
    std::vector<function_definition> functions;
    for (const auto &f : input.f)
    {
        if (auto tmp = process_function_definition(f); tmp.has_value())
        {
            functions.push_back(tmp.value());
        }
    }
    return { functions };
}

void process_variable_declaration(const wccff::parser::variable_declaration &s, std::vector<instruction> &instructions)
{
    if (s.init.has_value() == false)
    {
        return;
    }

    auto src = process_expression(s.init.value(), instructions);
    auto dst = var{ process_identifier(s.name) };
    instructions.emplace_back(copy_statement{ src, dst });
}

void process_while_statement(const std::unique_ptr<parser::while_statement> &node,
                             std::vector<instruction> &instructions)
{
    auto continue_label = get_loop_continue_label(process_identifier(node->label));
    auto break_label = get_loop_break_label(process_identifier(node->label));

    instructions.emplace_back(label_statement{ continue_label });
    auto result = process_expression(node->condition, instructions);
    instructions.emplace_back(jump_if_zero_statement{ result, break_label });
    process_statement(node->body, instructions);
    instructions.emplace_back(jump_statement{ continue_label });
    instructions.emplace_back(label_statement{ break_label });
}

std::string pretty_print(const unary_operator &op, int32_t ident)
{
    return std::visit(
      visitor{
        [ident](const binary_complement_operator &) { return wccff::format_indented(ident, "Complement"); },
        [ident](const negate_operator &) { return wccff::format_indented(ident, "Negate"); },
        [ident](const not_operator &) { return wccff::format_indented(ident, "Not"); },
      },
      op);
}
std::string pretty_print(const binary_operator &op, int32_t ident)
{
    return std::visit(
      visitor{
        [ident](const plus_operator &) { return wccff::format_indented(ident, "Plus"); },
        [ident](const subtract_operator &) { return wccff::format_indented(ident, "Subtract"); },
        [ident](const multiply_operator &) { return wccff::format_indented(ident, "Multiply"); },
        [ident](const divide_operator &) { return wccff::format_indented(ident, "Divide"); },
        [ident](const remainder_operator &) { return wccff::format_indented(ident, "Remainder"); },
        [ident](const binary_and_operator &) { return wccff::format_indented(ident, "Bitwise And"); },
        [ident](const binary_or_operator &) { return wccff::format_indented(ident, "Bitwise Or"); },
        [ident](const binary_xor_operator &) { return wccff::format_indented(ident, "Bitwise Xor"); },
        [ident](const left_shift_operator &) { return wccff::format_indented(ident, "Left Shift"); },
        [ident](const right_shift_operator &) { return wccff::format_indented(ident, "Right Shift"); },
        [ident](const equal_operator &) { return wccff::format_indented(ident, "Equal"); },
        [ident](const not_equal_operator &) { return wccff::format_indented(ident, "Not Equal"); },
        [ident](const less_than_operator &) { return wccff::format_indented(ident, "Less Than"); },
        [ident](const less_than_or_equal_operator &) { return wccff::format_indented(ident, "Less That or Equal"); },
        [ident](const greater_than_operator &) { return wccff::format_indented(ident, "Greater Than"); },
        [ident](const greater_than_or_equal_operator &) {
            return wccff::format_indented(ident, "Greater That or Equal");
        },
        [ident](const assignment_operator &) { return wccff::format_indented(ident, "Greater Than"); },
        [ident](const compound_plus_operator &) { return wccff::format_indented(ident, "Greater Than"); },
        [ident](const compound_minus_operator &) { return wccff::format_indented(ident, "Greater Than"); },
        [ident](const compound_multiplication_operator &) { return wccff::format_indented(ident, "Greater Than"); },
        [ident](const compound_division_operator &) { return wccff::format_indented(ident, "Greater Than"); },
        [ident](const compound_remainder_operator &) { return wccff::format_indented(ident, "Greater Than"); },
        [ident](const compound_bitwise_and_operator &) { return wccff::format_indented(ident, "Greater Than"); },
        [ident](const compound_bitwise_or_operator &) { return wccff::format_indented(ident, "Greater Than"); },
        [ident](const compound_bitwise_xor_operator &) { return wccff::format_indented(ident, "Greater Than"); },
        [ident](const compound_left_shift_operator &) { return wccff::format_indented(ident, "Greater Than"); },
        [ident](const compound_right_shift_operator &) { return wccff::format_indented(ident, "Greater Than"); },

      },
      op);
}
std::string pretty_print(const constant &val, int32_t ident)
{
    return wccff::format_indented(ident, "Constant({})", val.value);
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

std::string pretty_print(const fun_call &f, int32_t ident)
{
    std::string params;
    for (const auto &arg : f.args)
    {
        params += pretty_print(arg, 0);
    }

    return wccff::format_indented(ident, "FUNCALL({}, ({}), {})\n", f.fun_name.name, params, pretty_print(f.dst));
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
    return std::visit(visitor{ [ident](const auto &n) { return pretty_print(n, ident); } }, instruction);
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
    return wccff::format_indented(ident, "Function({})\n{}", f.name.name, pretty_print(f.instructions, ident + 4));
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
} // namespace wccff::tacky
