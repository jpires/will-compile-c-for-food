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

#include "assembly_generation.h"
#include "visitor.h"
#include <algorithm>
#include <ranges>

namespace wccff::assembly_generation {

struct symbol_table
{
    int32_t get_address(const identifier &id)
    {
        auto it = std::find_if(symbols.begin(), symbols.end(), [id](const auto &s) { return s.first == id; });
        if (it == symbols.end())
        {
            // The symbol doesn't exist
            // Create a new one and return the new address
            auto last_address = get_last_address();
            symbols.emplace_back(id, last_address - 4);
            return last_address - 4;
        }
        return it->second;
    }

    int32_t get_last_address()
    {
        if (symbols.empty())
        {
            return 0;
        }
        return symbols.back().second;
    }

    bool has_symbol(const identifier &id)
    {
        return std::any_of(symbols.begin(), symbols.end(), [id](const auto &s) { return s.first == id; });
    }

    std::vector<std::pair<identifier, int32_t>> symbols{};
};

symbol_table table;

identifier process_identifier(const wccff::tacky::identifier &id)
{
    return { id.name };
}

operand process_val(const wccff::tacky::val &v)
{
    return std::visit(visitor{
                        [](const tacky::constant &n) -> operand { return immediate{ n.value }; },
                        [](const tacky::var &n) -> operand { return pseudo{ process_identifier(n.id) }; },
                      },
                      v);
}

std::vector<instruction> process_statement(const wccff::tacky::copy_statement &stmt)
{
    return { mov_instruction{ process_val(stmt.src), process_val(stmt.dst) } };
}
std::vector<instruction> process_statement(const wccff::tacky::jump_statement &stmt)
{
    return { jmp{ process_identifier(stmt.target) } };
}
std::vector<instruction> process_statement(const wccff::tacky::jump_if_zero_statement &stmt)
{
    return { cmp{ immediate{ 0 }, process_val(stmt.condition) }, jmpcc{ E{}, process_identifier(stmt.target) } };
}
std::vector<instruction> process_statement(const wccff::tacky::jump_if_not_zero_statement &stmt)
{
    return { cmp{ immediate{ 0 }, process_val(stmt.condition) }, jmpcc{ NE{}, process_identifier(stmt.target) } };
}
std::vector<instruction> process_statement(const wccff::tacky::label_statement &stmt)
{
    return { label{ process_identifier(stmt.target) } };
}
std::vector<instruction> process_statement(const wccff::tacky::return_statement &stmt)
{
    mov_instruction mov{ process_val(stmt.val), ax{} };
    ret_instruction ret{};

    return { mov, ret };
}

unary_operator process_unary_operator(const wccff::tacky::unary_operator &op)
{
    return std::visit(visitor{
                        [](const tacky::binary_complement_operator &) -> unary_operator { return not_op{}; },
                        [](const tacky::negate_operator &) -> unary_operator { return neg_op{}; },
                        [](const tacky::not_operator &) -> unary_operator {
                            throw std::logic_error("logical not operator Not implemented");
                        },
                      },
                      op);
}

binary_operator process_binary_operator(const wccff::tacky::binary_operator &op)
{
    return std::visit(
      visitor{
        [](const tacky::plus_operator &) -> binary_operator { return add{}; },
        [](const tacky::subtract_operator &) -> binary_operator { return sub{}; },
        [](const tacky::multiply_operator &) -> binary_operator { return mul{}; },
        [](const tacky::divide_operator &) -> binary_operator { return sub{}; },
        [](const tacky::remainder_operator &) -> binary_operator { return mul{}; },
        [](const tacky::binary_and_operator &) -> binary_operator { return binary_and{}; },
        [](const tacky::binary_or_operator &) -> binary_operator { return binary_or{}; },
        [](const tacky::binary_xor_operator &) -> binary_operator { return binary_xor{}; },
        [](const tacky::left_shift_operator &) -> binary_operator { return left_shift{}; },
        [](const tacky::right_shift_operator &) -> binary_operator { return right_shift{}; },
        [](const tacky::equal_operator &) -> binary_operator {
            throw std::logic_error("Equal operator is not converted into a binary operator");
        },
        [](const tacky::not_equal_operator &) -> binary_operator {
            throw std::logic_error("Not Equal operator is not converted into a binary operator");
        },
        [](const tacky::less_than_operator &) -> binary_operator {
            throw std::logic_error("Less Than operator is not converted into a binary operator");
        },
        [](const tacky::less_than_or_equal_operator &) -> binary_operator {
            throw std::logic_error("Less Than or Equal operator is not converted into a binary operator");
        },
        [](const tacky::greater_than_operator &) -> binary_operator {
            throw std::logic_error("Greater Than operator is not converted into a binary operator");
        },
        [](const tacky::greater_than_or_equal_operator &) -> binary_operator {
            throw std::logic_error("Greater Than or Equal operator is not converted into a binary operator");
        },
        [](const tacky::assignment_operator &) -> binary_operator {
            throw std::logic_error("Assignment operator is not converted into a binary operator");
        },
        [](const tacky::compound_plus_operator &) -> binary_operator {
            throw std::logic_error("Compound Plus operator is not converted into a binary operator");
        },
        [](const tacky::compound_minus_operator &) -> binary_operator {
            throw std::logic_error("Compound Minus operator is not converted into a binary operator");
        },
        [](const tacky::compound_multiplication_operator &) -> binary_operator {
            throw std::logic_error("Compound Multiplication operator is not converted into a binary operator");
        },
        [](const tacky::compound_division_operator &) -> binary_operator {
            throw std::logic_error("Compound Division operator is not converted into a binary operator");
        },
        [](const tacky::compound_remainder_operator &) -> binary_operator {
            throw std::logic_error("Compound Remainder operator is not converted into a binary operator");
        },
        [](const tacky::compound_bitwise_and_operator &) -> binary_operator {
            throw std::logic_error("Compound Bitwise And operator is not converted into a binary operator");
        },
        [](const tacky::compound_bitwise_or_operator &) -> binary_operator {
            throw std::logic_error("Compound Bitwise Or operator is not converted into a binary operator");
        },
        [](const tacky::compound_bitwise_xor_operator &) -> binary_operator {
            throw std::logic_error("Compound Bitwise Xor operator is not converted into a binary operator");
        },
        [](const tacky::compound_left_shift_operator &) -> binary_operator {
            throw std::logic_error("Compound Left Shift operator is not converted into a binary operator");
        },
        [](const tacky::compound_right_shift_operator &) -> binary_operator {
            throw std::logic_error("Compound Right Shift operator is not converted into a binary operator");
        },

      },
      op);
}

std::vector<instruction> process_statement(const wccff::tacky::unary_statement &stmt)
{
    std::vector<instruction> instructions;
    if (std::holds_alternative<tacky::not_operator>(stmt.op))
    {
        instructions.emplace_back(cmp{ operand{ immediate{ 0 } }, process_val(stmt.src) });
        instructions.emplace_back(mov_instruction{ immediate{ 0 }, process_val(stmt.dst) });
        instructions.emplace_back(setcc{ E{}, process_val(stmt.dst) });
        return instructions;
    }

    mov_instruction mov{ process_val(stmt.src), process_val(stmt.dst) };
    unary ret{ process_unary_operator(stmt.op), process_val(stmt.dst) };

    return { mov, ret };
}

std::vector<instruction> process_statement(const wccff::tacky::binary_statement &stmt)
{
    auto is_relational_operator = [](tacky::binary_operator op) {
        return std::visit(visitor{
                            [](tacky::equal_operator) { return true; },
                            [](tacky::not_equal_operator) { return true; },
                            [](tacky::less_than_operator) { return true; },
                            [](tacky::less_than_or_equal_operator) { return true; },
                            [](tacky::greater_than_operator) { return true; },
                            [](tacky::greater_than_or_equal_operator) { return true; },
                            [](auto) { return false; },
                          },
                          op);
    };

    auto convert_tacky_op = [](tacky::binary_operator op) {
        return std::visit(visitor{
                            [](tacky::equal_operator) -> cond_code { return E{}; },
                            [](tacky::not_equal_operator) -> cond_code { return NE{}; },
                            [](tacky::less_than_operator) -> cond_code { return L{}; },
                            [](tacky::less_than_or_equal_operator) -> cond_code { return LE{}; },
                            [](tacky::greater_than_operator) -> cond_code { return G{}; },
                            [](tacky::greater_than_or_equal_operator) -> cond_code { return GE{}; },
                            [](auto) -> cond_code {
                                throw std::logic_error("Binary operator is not converted into a binary operator");
                            },
                          },
                          op);
    };

    if (is_relational_operator(stmt.op))
    {
        std::vector<instruction> instructions;
        instructions.emplace_back(cmp{ process_val(stmt.src2), process_val(stmt.src1) });
        instructions.emplace_back(mov_instruction{ immediate{ 0 }, process_val(stmt.dst) });
        instructions.emplace_back(setcc{ convert_tacky_op(stmt.op), process_val(stmt.dst) });

        return instructions;
    }

    if (std::holds_alternative<wccff::tacky::divide_operator>(stmt.op))
    {
        mov_instruction mov1{ process_val(stmt.src1), ax{} };
        idiv div{ process_val(stmt.src2) };
        mov_instruction mov2{ ax{}, process_val(stmt.dst) };

        return { mov1, cdq{}, div, mov2 };
    }

    if (std::holds_alternative<wccff::tacky::remainder_operator>(stmt.op))
    {
        mov_instruction mov1{ process_val(stmt.src1), ax{} };
        idiv div{ process_val(stmt.src2) };
        mov_instruction mov2{ dx{}, process_val(stmt.dst) };

        return { mov1, cdq{}, div, mov2 };
    }

    mov_instruction mov{ process_val(stmt.src1), process_val(stmt.dst) };
    binary ret{ process_binary_operator(stmt.op), process_val(stmt.src2), process_val(stmt.dst) };

    return { mov, ret };
}

std::vector<instruction> process_statement(const tacky::instruction &i)
{
    return std::visit(visitor{
                        [](const tacky::return_statement &n) { return process_statement(n); },
                        [](const tacky::unary_statement &n) { return process_statement(n); },
                        [](const tacky::binary_statement &n) { return process_statement(n); },
                        [](const tacky::copy_statement &n) { return process_statement(n); },
                        [](const tacky::jump_statement &n) { return process_statement(n); },
                        [](const tacky::jump_if_zero_statement &n) { return process_statement(n); },
                        [](const tacky::jump_if_not_zero_statement &n) { return process_statement(n); },
                        [](const tacky::label_statement &n) { return process_statement(n); },
                        [](const tacky::fun_call &n) -> std::vector<instruction> { return fun_call(n); },
                      },
                      i);
}

std::vector<instruction> process_statement(const std::vector<tacky::instruction> &s)
{
    std::vector<instruction> ret_insts;
    for (const auto &i : s)
    {
        ret_insts.append_range(process_statement(i));
    }
    return ret_insts;
}

std::vector<instruction> fun_call(const tacky::fun_call &i)
{
    std::array<reg, 6> regs = { di{}, si{}, dx{}, cx{}, R8{}, R9{} };
    std::vector<instruction> instructions;
    int stack_padding = i.args.size() % 2 ? 8 : 0;

    int stack_args = i.args.size() >= 6 ? i.args.size() - 6 : 0;
    if (stack_padding != 0)
    {
        instructions.emplace_back(allocate_stack{ stack_padding });
    }

    std::span args_in_reg(i.args.begin(), std::min(i.args.size(), 6ul));
    int pos = 0;
    for (const auto &arg : args_in_reg)
    {
        auto src = process_val(arg);
        instructions.emplace_back(mov_instruction{ src, regs[pos] });
        pos++;
    }

    if (i.args.size() > 6)
    {
        std::span arg_on_stack(i.args.begin() + 6, i.args.size() - 6);
        for (const auto &arg : arg_on_stack | std::views::reverse)
        {
            auto src = process_val(arg);
            if (std::holds_alternative<immediate>(src) || std::holds_alternative<reg>(src))
            {
                instructions.emplace_back(push{ src });
            }
            else
            {
                instructions.emplace_back(mov_instruction{ src, ax{} });
                instructions.emplace_back(push{ ax{} });
            }
        }
    }

    instructions.emplace_back(call{ process_identifier(i.fun_name) });

    if (int to_remove = (stack_args * 8) + stack_padding; to_remove != 0)
    {
        instructions.emplace_back(deallocate_stack{ to_remove });
    }

    instructions.emplace_back(mov_instruction{ ax{}, process_val(i.dst) });

    return instructions;
}

function process_function(const wccff::tacky::function_definition &f)
{
    auto p_source = [](int pos) -> operand {
        switch (pos)
        {
            case 0:
                return reg{ di{} };
            case 1:
                return reg{ si{} };
            case 2:
                return reg{ dx{} };
            case 3:
                return reg{ cx{} };
            case 4:
                return reg{ R8{} };
            case 5:
                return reg{ R9{} };
            default:
                return stack{ 16 + (pos - 6) * 8 };
        }
    };
    std::vector<instruction> instructions;

    int pos = 0;
    for (const auto &p : f.params)
    {
        instructions.emplace_back(mov_instruction{ p_source(pos), pseudo{ process_identifier(p) } });
        pos++;
    }

    instructions.append_range(process_statement(f.instructions));
    return function{ process_identifier(f.name), std::move(instructions), .is_global = f.global };
}

top_level process_top_level(const wccff::tacky::top_level &f)
{
    return std::visit(
      visitor{ [](const tacky::function_definition &node) -> top_level { return process_function(node); },
               [](const tacky::static_variable &node) -> top_level { return process_static_variable(node); } },
      f);
}

static_variable process_static_variable(const wccff::tacky::static_variable &f)
{
    return { process_identifier(f.name), f.global, f.init };
}

program process(const wccff::tacky::program &program)
{
    std::vector<top_level> top_levels;
    top_levels.reserve(program.function.size());
    for (const auto &f : program.function)
    {
        top_levels.push_back(process_top_level(f));
    }
    return { std::move(top_levels) };
}

operand convert_pseudo(pseudo &r, const wccff::symbol_table::symbol_table &t)
{
    if (table.has_symbol(r.name))
    {
        return stack{ table.get_address(r.name) };
    }

    if (auto s = t.get(parser::identifier{ r.name.name });
        s.has_value() && std::holds_alternative<wccff::symbol_table::static_attributes>(s.value().attrs))
    {
        return data{ r.name };
    }

    return stack{ table.get_address(r.name) };
}

void replace_pseudo_registers(mov_instruction &i, const wccff::symbol_table::symbol_table &t)
{
    if (std::holds_alternative<pseudo>(i.src))
    {
        auto r = std::get<pseudo>(i.src);
        i.src = convert_pseudo(r, t);
    }
    if (std::holds_alternative<pseudo>(i.dst))
    {
        auto r = std::get<pseudo>(i.dst);
        i.dst = convert_pseudo(r, t);
    }
}
void replace_pseudo_registers(unary &i, const wccff::symbol_table::symbol_table &t)
{
    if (std::holds_alternative<pseudo>(i.dst))
    {
        auto r = std::get<pseudo>(i.dst);
        i.dst = convert_pseudo(r, t);
    }
}

void replace_pseudo_registers(binary &i, const wccff::symbol_table::symbol_table &t)
{
    if (std::holds_alternative<pseudo>(i.src))
    {
        auto r = std::get<pseudo>(i.src);
        i.src = convert_pseudo(r, t);
    }

    if (std::holds_alternative<pseudo>(i.dst))
    {
        auto r = std::get<pseudo>(i.dst);
        i.dst = convert_pseudo(r, t);
    }
}
void replace_pseudo_registers(cmp &i, const wccff::symbol_table::symbol_table &t)
{
    if (std::holds_alternative<pseudo>(i.lhs))
    {
        auto r = std::get<pseudo>(i.lhs);
        i.lhs = convert_pseudo(r, t);
    }

    if (std::holds_alternative<pseudo>(i.rhs))
    {
        auto r = std::get<pseudo>(i.rhs);
        i.rhs = convert_pseudo(r, t);
    }
}
void replace_pseudo_registers(idiv &i, const wccff::symbol_table::symbol_table &t)
{
    if (std::holds_alternative<pseudo>(i.src))
    {
        auto r = std::get<pseudo>(i.src);
        i.src = convert_pseudo(r, t);
    }
}

void replace_pseudo_registers(setcc &i, const wccff::symbol_table::symbol_table &t)
{
    if (std::holds_alternative<pseudo>(i.dst))
    {
        auto r = std::get<pseudo>(i.dst);
        i.dst = convert_pseudo(r, t);
    }
}

void replace_pseudo_registers(push &i, const wccff::symbol_table::symbol_table &t)
{
    if (std::holds_alternative<pseudo>(i.src))
    {
        auto r = std::get<pseudo>(i.src);
        i.src = convert_pseudo(r, t);
    }
}

void replace_pseudo_registers(function &f, const wccff::symbol_table::symbol_table &t)
{
    table.symbols.clear();
    for (auto &i : f.instructions)
    {
        std::visit(visitor{
                     [&t](mov_instruction &inst) { replace_pseudo_registers(inst, t); },
                     [&t](unary &inst) { replace_pseudo_registers(inst, t); },
                     [&t](binary &inst) { replace_pseudo_registers(inst, t); },
                     [&t](cmp &inst) { replace_pseudo_registers(inst, t); },
                     [&t](idiv &inst) { replace_pseudo_registers(inst, t); },
                     [](cdq &) { /*Nothing to do */ },
                     [](jmp &) { /*Nothing to do */ },
                     [](jmpcc &) { /*Nothing to do */ },
                     [&t](setcc &inst) { replace_pseudo_registers(inst, t); },
                     [](label &) { /*Nothing to do */ },
                     [](allocate_stack &) { /*Nothing to do */ },
                     [](deallocate_stack &) { /*Nothing to do */ },
                     [&t](push &inst) { replace_pseudo_registers(inst, t); },
                     [](call &) { /*Nothing to do */ },
                     [](ret_instruction &) { /*Nothing to do */ },
                   },
                   i);
    }
    f.stack_size = std::abs(table.get_last_address());
}

void replace_pseudo_registers(top_level &program, const wccff::symbol_table::symbol_table &t)
{
    std::visit(visitor{
                 [&t](function &f) { replace_pseudo_registers(f, t); },
                 [](static_variable &) { /*Nothing to do here*/ },
               },
               program);
}
void replace_pseudo_registers(program &program, const wccff::symbol_table::symbol_table &t)
{
    for (auto &i : program.functions)
    {
        replace_pseudo_registers(i, t);
    }
}

bool is_memory_operand(const operand &o)
{
    return std::holds_alternative<stack>(o) || std::holds_alternative<data>(o);
}

std::optional<std::vector<instruction>> fixing_up_instructions11(const mov_instruction &n)
{
    if (is_memory_operand(n.src) && is_memory_operand(n.dst))
    {
        std::vector<instruction> ret_insts;
        mov_instruction m1{ n.src, R10{} };
        mov_instruction m2{ R10{}, n.dst };
        ret_insts.emplace_back(m1);
        ret_insts.emplace_back(m2);
        return ret_insts;
    }

    return std::nullopt;
}

std::optional<std::vector<instruction>> fixing_up_instructions11(const cmp &n)
{
    if (is_memory_operand(n.lhs) && is_memory_operand(n.rhs))
    {
        std::vector<instruction> ret_insts;
        mov_instruction m1{ n.lhs, R10{} };
        cmp b1{ R10{}, n.rhs };
        ret_insts.emplace_back(m1);
        ret_insts.emplace_back(b1);
        return ret_insts;
    }

    if (std::holds_alternative<immediate>(n.rhs))
    {
        std::vector<instruction> ret_insts;
        mov_instruction m1{ n.rhs, R11{} };
        cmp b1{ n.lhs, R11{} };
        ret_insts.emplace_back(m1);
        ret_insts.emplace_back(b1);
        return ret_insts;
    }

    return std::nullopt;
}

std::optional<std::vector<instruction>> fixing_up_instructions_binary(const binary &n)
{
    if (std::holds_alternative<add>(n.op) || std::holds_alternative<sub>(n.op) ||
        std::holds_alternative<binary_and>(n.op) || std::holds_alternative<binary_or>(n.op) ||
        std::holds_alternative<binary_xor>(n.op))
    {
        if (is_memory_operand(n.src) && is_memory_operand(n.dst))
        {
            std::vector<instruction> ret_insts;
            mov_instruction m1{ n.src, R10{} };
            binary b1{ n.op, R10{}, n.dst };
            ret_insts.emplace_back(m1);
            ret_insts.emplace_back(b1);
            return ret_insts;
        }
    }

    if (std::holds_alternative<left_shift>(n.op) || std::holds_alternative<right_shift>(n.op))
    {
        std::vector<instruction> ret_insts;
        mov_instruction m1{ n.src, cx{} };
        binary b1{ n.op, cx{}, n.dst };
        ret_insts.emplace_back(m1);
        ret_insts.emplace_back(b1);
        return ret_insts;
    }

    if (std::holds_alternative<mul>(n.op))
    {
        if (is_memory_operand(n.dst))
        {
            std::vector<instruction> ret_insts;
            mov_instruction m1{ n.dst, R11{} };
            binary b1{ n.op, n.src, R11{} };
            mov_instruction m2{ R11{}, n.dst };
            ret_insts.emplace_back(m1);
            ret_insts.emplace_back(b1);
            ret_insts.emplace_back(m2);
            return ret_insts;
        }
    }

    return std::nullopt;
}

std::optional<std::vector<instruction>> fixing_up_instructions_idiv(const idiv &n)
{
    if (std::holds_alternative<immediate>(n.src))
    {
        std::vector<instruction> ret_insts;
        mov_instruction m1{ n.src, R10{} };
        idiv m2{ R10{} };
        ret_insts.emplace_back(m1);
        ret_insts.emplace_back(m2);
        return ret_insts;
    }

    return std::nullopt;
}
std::optional<std::vector<instruction>> fixing_up_instructions1(const instruction &node)
{
    return std::visit(
      visitor{
        [](const mov_instruction &n) -> std::optional<std::vector<instruction>> { return fixing_up_instructions11(n); },
        [](const unary &) -> std::optional<std::vector<instruction>> { return std::nullopt; },
        [](const binary &i) -> std::optional<std::vector<instruction>> { return fixing_up_instructions_binary(i); },
        [](const cmp &i) -> std::optional<std::vector<instruction>> { return fixing_up_instructions11(i); },
        [](const idiv &i) -> std::optional<std::vector<instruction>> { return fixing_up_instructions_idiv(i); },
        [](const cdq &) -> std::optional<std::vector<instruction>> { return std::nullopt; },
        [](const jmp &) -> std::optional<std::vector<instruction>> { return std::nullopt; },
        [](const jmpcc &) -> std::optional<std::vector<instruction>> { return std::nullopt; },
        [](const setcc &) -> std::optional<std::vector<instruction>> { return std::nullopt; },
        [](const label &) -> std::optional<std::vector<instruction>> { return std::nullopt; },
        [](const allocate_stack &) -> std::optional<std::vector<instruction>> { return std::nullopt; },
        [](const deallocate_stack &) -> std::optional<std::vector<instruction>> { return std::nullopt; },
        [](const push &) -> std::optional<std::vector<instruction>> { return std::nullopt; },
        [](const call &) -> std::optional<std::vector<instruction>> { return std::nullopt; },
        [](const ret_instruction &) -> std::optional<std::vector<instruction>> { return std::nullopt; } },
      node);
}
void fixing_up_instructions(std::vector<instruction> &node)
{
    std::vector<instruction> tmp;

    for (const auto &i : node)
    {
        auto r = fixing_up_instructions1(i);
        if (r.has_value() == false)
        {
            tmp.push_back(i);
        }
        else
        {
            tmp.append_range(r.value());
        }
    }
    node.swap(tmp);
}

void fixing_up_instructions(function &node)
{
    auto round_up = [](int num) {
        int remainder = num % 16;
        if (remainder == 0)
            return num;

        return num + 16 - remainder;
    };
    auto stack_size = round_up(node.stack_size);
    node.instructions.insert(node.instructions.begin(), allocate_stack{ stack_size });

    fixing_up_instructions(node.instructions);
}

void fixing_up_instructions(top_level &node)
{
    if (std::holds_alternative<function>(node))
    {
        fixing_up_instructions(std::get<function>(node));
    }
}

void fixing_up_instructions(program &node)
{
    for (auto &f : node.functions)
    {
        fixing_up_instructions(f);
    }
}

std::string pretty_print(const cmp &node)
{
    return fmt::format("Cmp({}, {})", pretty_print(node.lhs), pretty_print(node.lhs));
}

std::string pretty_print(const cond_code &node)
{
    return std::visit(wccff::visitor{
                        [](E) { return "E"; },
                        [](NE) { return "NE"; },
                        [](L) { return "L"; },
                        [](LE) { return "LE"; },
                        [](G) { return "G"; },
                        [](GE) { return "GE"; },
                      },
                      node);
}
std::string pretty_print(const jmp &node)
{
    return fmt::format("Jmp({})", pretty_print(node.name));
}
std::string pretty_print(const jmpcc &node)
{
    return fmt::format("JmpCC({}, {})", pretty_print(node.cond), pretty_print(node.name));
}
std::string pretty_print(const label &node)
{
    return fmt::format("Label({})", pretty_print(node.name));
}
std::string pretty_print(const setcc &node)
{
    return fmt::format("SetCC({}, {})", pretty_print(node.cond), pretty_print(node.dst));
}

std::string pretty_print(const call &node)
{
    return fmt::format("Call {}", node.fun_name.name);
}

std::string pretty_print(const identifier &node)
{
    return fmt::format("{}", node.name);
}

std::string pretty_print(const binary_operator &node)
{
    return std::visit(visitor{
                        [](const add &) { return "Add"; },
                        [](const sub &) { return "Sub"; },
                        [](const mul &) { return "Mul"; },
                        [](const binary_and &) { return "Binary And"; },
                        [](const binary_or &) { return "Binary Or"; },
                        [](const binary_xor &) { return "Binary Xor"; },
                        [](const left_shift &) { return "Left Shift"; },
                        [](const right_shift &) { return "Right Shift"; },
                      },
                      node);
}
std::string pretty_print(const unary_operator &node)
{
    return std::visit(visitor{ [](const not_op &) { return "Complement"; }, [](const neg_op &) { return "Negate"; } },
                      node);
}

std::string pretty_print(const immediate &node)
{
    return fmt::format("Imm({})", node.value);
}

std::string pretty_print(const reg &node)
{
    return std::visit(visitor{
                        [](const ax &) { return "ax"; },
                        [](const cx &) { return "cx"; },
                        [](const dx &) { return "dx"; },
                        [](const di &) { return "di"; },
                        [](const si &) { return "si"; },
                        [](const R8 &) { return "R8d"; },
                        [](const R9 &) { return "R9d"; },
                        [](const R10 &) { return "R10d"; },
                        [](const R11 &) { return "R11d"; },
                      },
                      node);
}

std::string pretty_print(const pseudo &node)
{
    return fmt::format("Pseudo({})", pretty_print(node.name));
}

std::string pretty_print(const stack &node)
{
    return fmt::format("Stack({})", pretty_print(node.value));
}
std::string pretty_print(const data &node)
{
    return fmt::format("Data({})", pretty_print(node.name));
}
std::string pretty_print(const operand &node)
{
    return std::visit(visitor{
                        [](const immediate &n) { return pretty_print(n); },
                        [](const reg &n) { return pretty_print(n); },
                        [](const pseudo &n) { return pretty_print(n); },
                        [](const stack &n) { return pretty_print(n); },
                        [](const data &n) { return pretty_print(n); },
                      },
                      node);
}

std::string pretty_print(const mov_instruction &node)
{
    return fmt::format("Mov(src({}), dst({}))", pretty_print(node.src), pretty_print(node.dst));
}

std::string pretty_print(const unary &node)
{
    return fmt::format("Unary(op({}), dst({}))", pretty_print(node.op), pretty_print(node.dst));
}

std::string pretty_print(const binary &node)
{
    return fmt::format("Binary(op({}), src({}), dst({}))",
                       pretty_print(node.op),
                       pretty_print(node.src),
                       pretty_print(node.dst));
}
std::string pretty_print(const idiv &node)
{
    return fmt::format("iDiv");
}
std::string pretty_print(const cdq &node)
{
    return fmt::format("CDQ");
}

std::string pretty_print(const allocate_stack &node)
{
    return fmt::format("Stack({})", pretty_print(node.size));
}

std::string pretty_print(const deallocate_stack &node)
{
    return fmt::format("Stack({})", pretty_print(node.size));
}

std::string pretty_print(const ret_instruction &node)
{
    return fmt::format("Ret");
}

std::string pretty_print(const instruction &node)
{
    return std::visit(visitor{
                        [](const mov_instruction &n) { return pretty_print(n); },
                        [](const unary &n) { return pretty_print(n); },
                        [](const binary &n) { return pretty_print(n); },
                        [](const cmp &n) { return pretty_print(n); },
                        [](const idiv &n) { return pretty_print(n); },
                        [](const cdq &n) { return pretty_print(n); },
                        [](const jmp &n) { return pretty_print(n); },
                        [](const jmpcc &n) { return pretty_print(n); },
                        [](const setcc &n) { return pretty_print(n); },
                        [](const label &n) { return pretty_print(n); },
                        [](const allocate_stack &n) { return pretty_print(n); },
                        [](const deallocate_stack &n) { return pretty_print(n); },
                        [](const push &n) { return pretty_print(n); },
                        [](const call &n) { return pretty_print(n); },
                        [](const ret_instruction &n) { return pretty_print(n); },
                      },
                      node);
}

std::string pretty_print(const std::vector<instruction> &node)
{
    std::string pretty;
    for (const auto &i : node)
    {
        pretty += fmt::format("{}\n", pretty_print(i));
    }
    return pretty;
}

std::string pretty_print(const function &node)
{
    return fmt::format("Function(name: {}\ninsts: {}", pretty_print(node.name), pretty_print(node.instructions));
}
std::string pretty_print(const static_variable &node)
{
    return fmt::format("StaticVariable(name: {})", pretty_print(node.name));
}

std::string pretty_print(const top_level &node)
{
    return std::visit(visitor{ [](const function &n) { return pretty_print(n); },
                               [](const static_variable &n) { return pretty_print(n); } },
                      node);
}

std::string pretty_print(const program &node)
{
    std::string output;
    for (const auto &f : node.functions)
    {
        output += pretty_print(f) + "\n";
    }
    return output;
}

std::string pretty_print(const push &node)
{
    return fmt::format("Push({})", pretty_print(node.src));
}

} // namespace wccff::assembly_generation
