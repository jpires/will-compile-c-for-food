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
#include "symbol_table.h"
#include "visitor.h"
#include <algorithm>
#include <ranges>

namespace wccff::assembly_generation {

assembly_type get_assembly_type(const wccff::constant &v)
{
    return std::visit(visitor{
                        [](const int_constant &) -> assembly_type { return long_word{}; },
                        [](const long_constant &) -> assembly_type { return quad_word{}; },
                        [](const unsigned_int_constant &) -> assembly_type { return long_word{}; },
                        [](const unsigned_long_constant &) -> assembly_type { return quad_word{}; },
                        [](const auto &) -> assembly_type { throw std::runtime_error("Not Implemented"); },
                      },
                      v);
}
assembly_type get_assembly_type(const tacky::val &v, const wccff::symbol_table::symbol_table &t)
{
    return std::visit(visitor{
                        [](const constant &v) -> assembly_type { return get_assembly_type(v); },
                        [&t](const tacky::var &v) -> assembly_type { return get_assembly_type(v, t); },
                      },
                      v);
}
assembly_type get_assembly_type(const tacky::var &v, const wccff::symbol_table::symbol_table &t)
{
    auto s = t.get(parser::identifier{ v.id.name });
    if (s.has_value() == false)
    {
        throw std::logic_error("Variable not found");
    }

    return std::visit(
      visitor{
        [](const wccff::double_type &) -> assembly_type { throw std::logic_error("Not implemented"); },
        [](const wccff::int_type &) -> assembly_type { return long_word{}; },
        [](const wccff::long_type &) -> assembly_type { return quad_word{}; },
        [](const wccff::unsigned_int_type &) -> assembly_type { return long_word{}; },
        [](const wccff::unsigned_long_type &) -> assembly_type { return quad_word{}; },
        [](const wccff::void_type &) -> assembly_type { throw std::logic_error("Not implemented"); },
        [](const std::unique_ptr<fun_type> &) -> assembly_type { throw std::logic_error("Not implemented"); },
      },
      s.value().type);
}

type get_type(const wccff::constant &v)
{
    return std::visit(visitor{
                        [](const int_constant &) -> type { return int_type{}; },
                        [](const long_constant &) -> type { return long_type{}; },
                        [](const unsigned_int_constant &) -> type { return unsigned_int_type{}; },
                        [](const unsigned_long_constant &) -> type { return unsigned_long_type{}; },
                        [](const auto &) -> type { throw std::runtime_error("Not Implemented"); },
                      },
                      v);
}
type get_type(const tacky::val &v, const wccff::symbol_table::symbol_table &table)
{
    return std::visit(visitor{
                        [](const constant &v) -> type { return get_type(v); },
                        [&table](const tacky::var &v) -> type { return get_type(v, table); },
                      },
                      v);
}
type get_type(const tacky::var &v, const wccff::symbol_table::symbol_table &table)
{
    auto s = table.get(parser::identifier{ v.id.name });
    if (s.has_value() == false)
    {
        throw std::logic_error("Variable not found");
    }

    return copy_type(s.value().type);
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

binary_operator process_binary_operator(const wccff::tacky::binary_operator &op, const type &t)
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
        [&t](const tacky::left_shift_operator &) -> binary_operator {
            return is_signed_type(t) ? binary_operator{ left_shift{} } : binary_operator{ left_shift_aritmetic{} };
        },
        [&t](const tacky::right_shift_operator &) -> binary_operator {
            return is_signed_type(t) ? binary_operator{ right_shift{} } : binary_operator{ right_shift_aritmetic{} };
        },
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

operand convert_pseudo(pseudo &r, wccff::symbol_table::backend_symbol_table &t)
{
    auto s = t.get(parser::identifier{ r.name.name });
    if (s.has_value() == false)
    {
        throw std::logic_error(fmt::format("Pseudo {} doesn't have an entry on the backend_symbol_table", r.name.name));
    }

    if (std::get<wccff::symbol_table::obj_entry>(s.value()).is_static)
    {
        return data{ r.name };
    }

    return stack{ t.get_symbol_offset(parser::identifier{ r.name.name }) };
}

void replace_pseudo_registers(mov_instruction &i, wccff::symbol_table::backend_symbol_table &t)
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

void replace_pseudo_registers(movx &i, wccff::symbol_table::backend_symbol_table &t)
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

void replace_pseudo_registers(mov_zero_extend &i, wccff::symbol_table::backend_symbol_table &t)
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

void replace_pseudo_registers(unary &i, wccff::symbol_table::backend_symbol_table &t)
{
    if (std::holds_alternative<pseudo>(i.dst))
    {
        auto r = std::get<pseudo>(i.dst);
        i.dst = convert_pseudo(r, t);
    }
}

void replace_pseudo_registers(binary &i, wccff::symbol_table::backend_symbol_table &t)
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
void replace_pseudo_registers(cmp &i, wccff::symbol_table::backend_symbol_table &t)
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
void replace_pseudo_registers(idiv &i, wccff::symbol_table::backend_symbol_table &t)
{
    if (std::holds_alternative<pseudo>(i.src))
    {
        auto r = std::get<pseudo>(i.src);
        i.src = convert_pseudo(r, t);
    }
}

void replace_pseudo_registers(div &i, wccff::symbol_table::backend_symbol_table &t)
{
    if (std::holds_alternative<pseudo>(i.src))
    {
        auto r = std::get<pseudo>(i.src);
        i.src = convert_pseudo(r, t);
    }
}

void replace_pseudo_registers(setcc &i, wccff::symbol_table::backend_symbol_table &t)
{
    if (std::holds_alternative<pseudo>(i.dst))
    {
        auto r = std::get<pseudo>(i.dst);
        i.dst = convert_pseudo(r, t);
    }
}

void replace_pseudo_registers(push &i, wccff::symbol_table::backend_symbol_table &t)
{
    if (std::holds_alternative<pseudo>(i.src))
    {
        auto r = std::get<pseudo>(i.src);
        i.src = convert_pseudo(r, t);
    }
}

void replace_pseudo_registers(function &f, wccff::symbol_table::backend_symbol_table &t)
{
    t.enter_function();
    for (auto &i : f.instructions)
    {
        std::visit(visitor{
                     [&t](mov_instruction &inst) { replace_pseudo_registers(inst, t); },
                     [&t](movx &inst) { return replace_pseudo_registers(inst, t); },
                     [&t](mov_zero_extend &inst) { return replace_pseudo_registers(inst, t); },
                     [&t](unary &inst) { replace_pseudo_registers(inst, t); },
                     [&t](binary &inst) { replace_pseudo_registers(inst, t); },
                     [&t](cmp &inst) { replace_pseudo_registers(inst, t); },
                     [&t](idiv &inst) { replace_pseudo_registers(inst, t); },
                     [&t](div &inst) { replace_pseudo_registers(inst, t); },
                     [](cdq &) { /*Nothing to do */ },
                     [](jmp &) { /*Nothing to do */ },
                     [](jmpcc &) { /*Nothing to do */ },
                     [&t](setcc &inst) { replace_pseudo_registers(inst, t); },
                     [](label &) { /*Nothing to do */ },
                     [&t](push &inst) { replace_pseudo_registers(inst, t); },
                     [](call &) { /*Nothing to do */ },
                     [](ret_instruction &) { /*Nothing to do */ },
                   },
                   i);
    }
    f.stack_size = std::abs(t.get_current_offset());
}

void replace_pseudo_registers(top_level &program, wccff::symbol_table::backend_symbol_table &t)
{
    std::visit(visitor{
                 [&t](function &f) { replace_pseudo_registers(f, t); },
                 [](static_variable &) { /*Nothing to do here*/ },
               },
               program);
}
void replace_pseudo_registers(program &program, wccff::symbol_table::backend_symbol_table &t)
{
    for (auto &i : program.functions)
    {
        replace_pseudo_registers(i, t);
    }
}

bool is_larger_immediate(const operand &op)
{
    if (std::holds_alternative<immediate>(op))
    {
        auto im = std::get<immediate>(op);
        if (im.value > std::numeric_limits<int32_t>::max() || im.value < std::numeric_limits<int32_t>::min())
        {
            return true;
        }
    }

    return false;
};

bool is_memory_operand(const operand &o)
{
    return std::holds_alternative<stack>(o) || std::holds_alternative<data>(o);
}

void assembly_generation::process(const tacky::binary_statement &stmt)
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

    auto convert_tacky_op = [](tacky::binary_operator op, const wccff::type &t) {
        if (is_signed_type(t))
        {
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
        }
        else
        {
            return std::visit(visitor{
                                [](tacky::equal_operator) -> cond_code { return E{}; },
                                [](tacky::not_equal_operator) -> cond_code { return NE{}; },
                                [](tacky::less_than_operator) -> cond_code { return B{}; },
                                [](tacky::less_than_or_equal_operator) -> cond_code { return BE{}; },
                                [](tacky::greater_than_operator) -> cond_code { return A{}; },
                                [](tacky::greater_than_or_equal_operator) -> cond_code { return AE{}; },
                                [](auto) -> cond_code {
                                    throw std::logic_error("Binary operator is not converted into a binary operator");
                                },
                              },
                              op);
        }
    };

    auto src1_type = get_assembly_type(stmt.src1, m_table);
    if (is_relational_operator(stmt.op))
    {
        auto op_type = get_type(stmt.src1, m_table);

        auto dst_type = get_assembly_type(stmt.dst, m_table);
        m_instructions.emplace_back(cmp{ process(stmt.src2), process(stmt.src1), src1_type });
        m_instructions.emplace_back(mov_instruction{ immediate{ 0 }, process(stmt.dst), dst_type });
        m_instructions.emplace_back(setcc{ convert_tacky_op(stmt.op, op_type), process(stmt.dst) });

        return;
    }

    if (std::holds_alternative<wccff::tacky::divide_operator>(stmt.op))
    {
        auto op_type = get_type(stmt.src1, m_table);
        if (is_signed_type(op_type))
        {
            m_instructions.emplace_back(mov_instruction{ process(stmt.src1), ax{}, src1_type });
            m_instructions.emplace_back(cdq{ src1_type });
            m_instructions.emplace_back(idiv{ process(stmt.src2), src1_type });
            m_instructions.emplace_back(mov_instruction{ ax{}, process(stmt.dst), src1_type });
            return;
        }
        else
        {
            m_instructions.emplace_back(mov_instruction{ process(stmt.src1), ax{}, src1_type });
            m_instructions.emplace_back(mov_instruction{ immediate{ 0 }, dx{}, src1_type });
            m_instructions.emplace_back(div{ process(stmt.src2), src1_type });
            m_instructions.emplace_back(mov_instruction{ ax{}, process(stmt.dst), src1_type });
            return;
        }
    }

    if (std::holds_alternative<wccff::tacky::remainder_operator>(stmt.op))
    {
        auto op_type = get_type(stmt.src1, m_table);

        if (is_signed_type(op_type))
        {
            std::vector<instruction> instructions;
            m_instructions.emplace_back(mov_instruction{ process(stmt.src1), ax{}, src1_type });
            m_instructions.emplace_back(cdq{ src1_type });
            m_instructions.emplace_back(idiv{ process(stmt.src2), src1_type });
            m_instructions.emplace_back(mov_instruction{ dx{}, process(stmt.dst), src1_type });

            return;
        }
        else
        {
            m_instructions.emplace_back(mov_instruction{ process(stmt.src1), ax{}, src1_type });
            m_instructions.emplace_back(mov_instruction{ immediate{ 0 }, dx{}, src1_type });
            m_instructions.emplace_back(idiv{ process(stmt.src2), src1_type });
            m_instructions.emplace_back(mov_instruction{ dx{}, process(stmt.dst), src1_type });
            return;
        }
    }

    m_instructions.emplace_back(mov_instruction{ process(stmt.src1), process(stmt.dst), src1_type });
    m_instructions.emplace_back(binary{ process_binary_operator(stmt.op, get_type(stmt.src1, m_table)),
                                        process(stmt.src2),
                                        process(stmt.dst),
                                        src1_type });
}

operand assembly_generation::process(const constant &n)
{
    return std::visit(
      visitor{
        [](const int_constant &c) -> operand { return immediate{ c.value }; },
        [](const long_constant &c) -> operand { return immediate{ c.value }; },
        [](const unsigned_int_constant &c) -> operand { return immediate{ c.value }; },
        [](const unsigned_long_constant &c) -> operand { return immediate{ static_cast<int64_t>(c.value) }; },
        [](const auto &) -> operand { throw std::runtime_error("INTERNAL ERROR"); },
      },
      n);
}

void assembly_generation::process(const tacky::copy_statement &stmt)
{
    m_instructions.emplace_back(
      mov_instruction{ process(stmt.src), process(stmt.dst), get_assembly_type(stmt.src, m_table) });
}
void assembly_generation::process(const tacky::fun_call &i)
{
    std::array<reg, 6> regs = { di{}, si{}, dx{}, cx{}, R8{}, R9{} };
    int stack_padding = i.args.size() % 2 ? 8 : 0;

    int stack_args = i.args.size() >= 6 ? i.args.size() - 6 : 0;
    if (stack_padding != 0)
    {
        m_instructions.emplace_back(binary{ sub{}, immediate{ stack_padding }, SP{}, quad_word{} });
    }

    std::span args_in_reg(i.args.begin(), std::min(i.args.size(), 6ul));
    int pos = 0;
    for (const auto &arg : args_in_reg)
    {
        auto src = process(arg);
        m_instructions.emplace_back(mov_instruction{ src, regs[pos], get_assembly_type(arg, m_table) });
        pos++;
    }

    if (i.args.size() > 6)
    {
        std::span arg_on_stack(i.args.begin() + 6, i.args.size() - 6);
        for (const auto &arg : arg_on_stack | std::views::reverse)
        {
            auto src = process(arg);
            if (std::holds_alternative<immediate>(src) || std::holds_alternative<reg>(src) ||
                std::holds_alternative<quad_word>(get_assembly_type(arg, m_table)))
            {
                m_instructions.emplace_back(push{ src });
            }
            else
            {
                m_instructions.emplace_back(mov_instruction{ src, ax{}, long_word{} });
                m_instructions.emplace_back(push{ ax{} });
            }
        }
    }

    m_instructions.emplace_back(call{ process(i.fun_name) });

    if (int to_remove = (stack_args * 8) + stack_padding; to_remove != 0)
    {
        m_instructions.emplace_back(binary{ add{}, immediate{ to_remove }, SP{}, quad_word{} });
    }

    m_instructions.emplace_back(mov_instruction{ ax{}, process(i.dst), get_assembly_type(i.dst, m_table) });

    return;
}
function assembly_generation::process(const tacky::function_definition &f)
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

    int pos = 0;
    for (const auto &p : f.params)
    {
        m_instructions.emplace_back(
          mov_instruction{ p_source(pos), pseudo{ process(p) }, get_assembly_type(tacky::var{ p.name }, m_table) });
        pos++;
    }

    process(f.instructions);
    return function{ process(f.name), std::move(m_instructions), .is_global = f.global };
}

identifier assembly_generation::process(const wccff::tacky::identifier &id)
{
    return { id.name };
}

void assembly_generation::process(const tacky::instruction &i)
{
    std::visit(visitor{
                 [&](const tacky::binary_statement &n) { process(n); },
                 [&](const tacky::copy_statement &n) { process(n); },
                 [&](const tacky::fun_call &n) { process(n); },
                 [&](const tacky::jump_if_not_zero_statement &n) { process(n); },
                 [&](const tacky::jump_if_zero_statement &n) { process(n); },
                 [&](const tacky::jump_statement &n) { process(n); },
                 [&](const tacky::label_statement &n) { process(n); },
                 [&](const tacky::return_statement &n) { process(n); },
                 [&](const tacky::sing_extend &n) { process(n); },
                 [&](const tacky::truncate &n) { process(n); },
                 [&](const tacky::unary_statement &n) { process(n); },
                 [&](const tacky::zero_extend &n) { process(n); },
                 [](const auto &) { throw std::runtime_error("NOT IMPLEMENTED"); },
               },
               i);
}
void assembly_generation::process(const tacky::jump_if_not_zero_statement &stmt)
{
    m_instructions.emplace_back(
      cmp{ immediate{ 0 }, process(stmt.condition), get_assembly_type(stmt.condition, m_table) });
    m_instructions.emplace_back(jmpcc{ NE{}, process(stmt.target) });
}
void assembly_generation::process(const tacky::jump_if_zero_statement &stmt)
{
    m_instructions.emplace_back(
      cmp{ immediate{ 0 }, process(stmt.condition), get_assembly_type(stmt.condition, m_table) });
    m_instructions.emplace_back(jmpcc{ E{}, process(stmt.target) });
}
void assembly_generation::process(const tacky::jump_statement &stmt)
{
    m_instructions.emplace_back(jmp{ process(stmt.target) });
}
void assembly_generation::process(const tacky::label_statement &stmt)
{
    m_instructions.emplace_back(label{ process(stmt.target) });
}
program assembly_generation::process(const tacky::program &program)
{
    std::vector<top_level> top_levels;
    top_levels.reserve(program.function.size());
    for (const auto &f : program.function)
    {
        top_levels.push_back(process(f));
    }
    return { std::move(top_levels) };
}
void assembly_generation::process(const tacky::return_statement &stmt)
{
    m_instructions.emplace_back(mov_instruction{ process(stmt.val), ax{}, get_assembly_type(stmt.val, m_table) });
    m_instructions.emplace_back(ret_instruction{});
}
void assembly_generation::process(const tacky::sing_extend &i)
{
    m_instructions.emplace_back(movx{ process(i.src), process(i.dst) });
}
static_variable assembly_generation::process(const tacky::static_variable &f)
{
    auto align = std::visit(visitor{
                              [](const wccff::int_type &) { return 4; },
                              [](const wccff::long_type &) { return 8; },
                              [](const wccff::unsigned_int_type &) { return 4; },
                              [](const wccff::unsigned_long_type &) { return 8; },
                              [](const auto &) -> int32_t { throw std::logic_error("Not implemented"); },
                            },
                            m_table.get(parser::identifier{ f.name.name }).value().type);
    return { process(f.name), f.global, align, f.init };
}
top_level assembly_generation::process(const tacky::top_level &f)
{
    return std::visit(visitor{ [&](const tacky::function_definition &node) -> top_level { return process(node); },
                               [&](const tacky::static_variable &node) -> top_level { return process(node); } },
                      f);
}
void assembly_generation::process(const tacky::truncate &i)
{
    m_instructions.emplace_back(mov_instruction{ process(i.src), process(i.dst), long_word{} });
}
void assembly_generation::process(const tacky::unary_statement &stmt)
{
    if (std::holds_alternative<tacky::not_operator>(stmt.op))
    {
        m_instructions.emplace_back(
          cmp{ operand{ immediate{ 0 } }, process(stmt.src), get_assembly_type(stmt.src, m_table) });
        m_instructions.emplace_back(
          mov_instruction{ immediate{ 0 }, process(stmt.dst), get_assembly_type(stmt.dst, m_table) });
        m_instructions.emplace_back(setcc{ E{}, process(stmt.dst) });
        return;
    }

    m_instructions.emplace_back(
      mov_instruction{ process(stmt.src), process(stmt.dst), get_assembly_type(stmt.src, m_table) });
    m_instructions.emplace_back(
      unary{ process_unary_operator(stmt.op), process(stmt.dst), get_assembly_type(stmt.src, m_table) });
}

operand assembly_generation::process(const wccff::tacky::val &v)
{
    return std::visit(visitor{
                        [&](const constant &n) -> operand { return process(n); },
                        [&](const tacky::var &n) -> operand { return pseudo{ process(n.id) }; },
                      },
                      v);
}

void assembly_generation::process(const std::vector<tacky::instruction> &s)
{
    for (const auto &i : s)
    {
        process(i);
    }
}
void assembly_generation::process(const tacky::zero_extend &i)
{
    m_instructions.emplace_back(mov_zero_extend{ process(i.src), process(i.dst) });
}

std::optional<std::vector<instruction>> fixing_up_instructions11(const mov_instruction &n)
{
    if (is_memory_operand(n.src) && is_memory_operand(n.dst))
    {
        std::vector<instruction> ret_insts;
        mov_instruction m1{ n.src, R10{}, n.type };
        mov_instruction m2{ R10{}, n.dst, n.type };
        ret_insts.emplace_back(m1);
        ret_insts.emplace_back(m2);
        return ret_insts;
    }

    if (is_larger_immediate(n.src) && is_memory_operand(n.dst))
    {
        std::vector<instruction> ret_insts;
        mov_instruction m1{ n.src, R10{}, quad_word{} };
        mov_instruction m2{ R10{}, n.dst, n.type };
        ret_insts.emplace_back(m1);
        ret_insts.emplace_back(m2);
        return ret_insts;
    }

    // This doesn't stricly need to happen, but it makes the assembly code better
    // a movl, will not be able to carry an immediate bigger than 32 bits.
    // The assembler will handle that correctly. But this makes the code more easier to follow.
    if (is_larger_immediate(n.src) && std::holds_alternative<long_word>(n.type))
    {
        auto i = std::get<immediate>(n.src);
        auto new_value = static_cast<int32_t>(i.value);

        std::vector<instruction> ret_insts;
        mov_instruction m1{ immediate{ new_value }, n.dst, n.type };
        ret_insts.emplace_back(m1);
        return ret_insts;
    }

    return std::nullopt;
}

std::optional<std::vector<instruction>> fixing_up_instructions11(const mov_zero_extend &n)
{
    if (std::holds_alternative<reg>(n.dst))
    {
        std::vector<instruction> ret_insts;
        ret_insts.emplace_back(mov_instruction{ n.src, n.dst, long_word{} });
        return ret_insts;
    }

    if (is_memory_operand(n.dst))
    {
        std::vector<instruction> ret_insts;
        ret_insts.emplace_back(mov_instruction{ n.src, R11{}, long_word{} });
        ret_insts.emplace_back(mov_instruction{ R11{}, n.dst, quad_word{} });
        return ret_insts;
    }

    return std::nullopt;
}

std::optional<std::vector<instruction>> fixing_up_instructions11(const movx &n)
{
    if (is_memory_operand(n.dst) && std::holds_alternative<immediate>(n.src))
    {
        std::vector<instruction> ret_insts;
        mov_instruction m1{ .src = n.src, .dst = R10{}, .type = long_word{} };
        movx m2{ .src = R10{}, .dst = R11{} };
        mov_instruction m3{ .src = R11{}, .dst = n.dst, .type = quad_word{} };

        ret_insts.emplace_back(m1);
        ret_insts.emplace_back(m2);
        ret_insts.emplace_back(m3);
        return ret_insts;
    }

    if (is_memory_operand(n.dst))
    {
        std::vector<instruction> ret_insts;
        movx m1{ .src = n.src, .dst = R10{} };
        mov_instruction m2{ .src = R10{}, .dst = n.dst, .type = quad_word{} };
        ret_insts.emplace_back(m1);
        ret_insts.emplace_back(m2);
        return ret_insts;
    }

    if (std::holds_alternative<immediate>(n.src))
    {
        std::vector<instruction> ret_insts;
        mov_instruction m1{ .src = n.src, .dst = R10{}, .type = long_word{} };
        movx m2{ .src = R10{}, .dst = n.dst };

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
        mov_instruction m1{ n.lhs, R10{}, n.type };
        cmp b1{ R10{}, n.rhs, n.type };
        ret_insts.emplace_back(m1);
        ret_insts.emplace_back(b1);
        return ret_insts;
    }

    if (is_larger_immediate(n.lhs) && std::holds_alternative<immediate>(n.rhs))
    {
        std::vector<instruction> ret_insts;
        mov_instruction m1{ n.lhs, R10{}, quad_word{} };
        mov_instruction m2{ n.rhs, R11{}, n.type };
        cmp b1{ R10{}, R11{}, n.type };
        ret_insts.emplace_back(m1);
        ret_insts.emplace_back(m2);
        ret_insts.emplace_back(b1);
        return ret_insts;
    }

    if (is_larger_immediate(n.lhs))
    {
        std::vector<instruction> ret_insts;
        mov_instruction m1{ n.lhs, R10{}, quad_word{} };
        cmp b1{ R10{}, n.rhs, n.type };
        ret_insts.emplace_back(m1);
        ret_insts.emplace_back(b1);
        return ret_insts;
    }

    if (std::holds_alternative<immediate>(n.rhs))
    {
        std::vector<instruction> ret_insts;
        mov_instruction m1{ n.rhs, R11{}, n.type };
        cmp b1{ n.lhs, R11{}, n.type };
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
            mov_instruction m1{ n.src, R10{}, n.type };
            binary b1{ n.op, R10{}, n.dst, n.type };
            ret_insts.emplace_back(m1);
            ret_insts.emplace_back(b1);
            return ret_insts;
        }

        if (is_larger_immediate(n.src))
        {
            std::vector<instruction> ret_insts;
            mov_instruction m1{ n.src, R10{}, quad_word{} };
            binary b1{ n.op, R10{}, n.dst, n.type };
            ret_insts.emplace_back(m1);
            ret_insts.emplace_back(b1);
            return ret_insts;
        }
    }

    if (std::holds_alternative<left_shift>(n.op) || std::holds_alternative<right_shift>(n.op) ||
        std::holds_alternative<left_shift_aritmetic>(n.op) || std::holds_alternative<right_shift_aritmetic>(n.op))
    {
        std::vector<instruction> ret_insts;
        mov_instruction m1{ n.src, cx{}, n.type };
        binary b1{ n.op, cx{}, n.dst, n.type };
        ret_insts.emplace_back(m1);
        ret_insts.emplace_back(b1);
        return ret_insts;
    }

    if (std::holds_alternative<mul>(n.op))
    {
        if (is_larger_immediate(n.src) && is_memory_operand(n.dst))
        {
            std::vector<instruction> ret_insts;
            mov_instruction m1{ n.src, R10{}, quad_word{} };
            mov_instruction m2{ n.dst, R11{}, .type = n.type };
            binary b1{ n.op, R10{}, R11{}, .type = n.type };
            mov_instruction m3{ R11{}, n.dst, .type = n.type };
            ret_insts.emplace_back(m1);
            ret_insts.emplace_back(m2);
            ret_insts.emplace_back(b1);
            ret_insts.emplace_back(m3);
            return ret_insts;
        }

        if (is_memory_operand(n.dst))
        {
            std::vector<instruction> ret_insts;
            mov_instruction m1{ n.dst, R11{}, .type = n.type };
            binary b1{ n.op, n.src, R11{}, .type = n.type };
            mov_instruction m2{ R11{}, n.dst, .type = n.type };
            ret_insts.emplace_back(m1);
            ret_insts.emplace_back(b1);
            ret_insts.emplace_back(m2);
            return ret_insts;
        }
        if (is_larger_immediate(n.src))
        {
            std::vector<instruction> ret_insts;
            mov_instruction m1{ n.src, R10{}, quad_word{} };
            binary b1{ n.op, R10{}, n.dst, .type = n.type };
            ret_insts.emplace_back(m1);
            ret_insts.emplace_back(b1);
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
        mov_instruction m1{ n.src, R10{}, n.type };
        idiv m2{ R10{}, n.type };
        ret_insts.emplace_back(m1);
        ret_insts.emplace_back(m2);
        return ret_insts;
    }

    return std::nullopt;
}

std::optional<std::vector<instruction>> fixing_up_instructions_idiv(const div &n)
{
    if (std::holds_alternative<immediate>(n.src))
    {
        std::vector<instruction> ret_insts;
        mov_instruction m1{ n.src, R10{}, n.type };
        div m2{ R10{}, n.type };
        ret_insts.emplace_back(m1);
        ret_insts.emplace_back(m2);
        return ret_insts;
    }

    return std::nullopt;
}

std::optional<std::vector<instruction>> fixing_up_instructions11(const push &n)
{
    if (is_larger_immediate(n.src))
    {
        std::vector<instruction> ret_insts;
        mov_instruction m1{ n.src, R10{}, quad_word{} };
        push m2{ R10{} };
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
        [](const mov_zero_extend &n) -> std::optional<std::vector<instruction>> { return fixing_up_instructions11(n); },
        [](const movx &n) -> std::optional<std::vector<instruction>> { return fixing_up_instructions11(n); },
        [](const unary &) -> std::optional<std::vector<instruction>> { return std::nullopt; },
        [](const binary &i) -> std::optional<std::vector<instruction>> { return fixing_up_instructions_binary(i); },
        [](const cmp &i) -> std::optional<std::vector<instruction>> { return fixing_up_instructions11(i); },
        [](const idiv &i) -> std::optional<std::vector<instruction>> { return fixing_up_instructions_idiv(i); },
        [](const div &i) -> std::optional<std::vector<instruction>> { return fixing_up_instructions_idiv(i); },
        [](const cdq &) -> std::optional<std::vector<instruction>> { return std::nullopt; },
        [](const jmp &) -> std::optional<std::vector<instruction>> { return std::nullopt; },
        [](const jmpcc &) -> std::optional<std::vector<instruction>> { return std::nullopt; },
        [](const setcc &) -> std::optional<std::vector<instruction>> { return std::nullopt; },
        [](const label &) -> std::optional<std::vector<instruction>> { return std::nullopt; },
        [](const push &i) -> std::optional<std::vector<instruction>> { return fixing_up_instructions11(i); },
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
    node.instructions.insert(node.instructions.begin(), binary{ sub{}, immediate{ stack_size }, SP{}, quad_word{} });

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

std::string pretty_print(const assembly_type &node)
{
    return std::visit(visitor{
                        [](const long_word) { return "long_word"; },
                        [](const quad_word) { return "quad_word"; },
                      },
                      node);
}

std::string pretty_print(const binary &node)
{
    return fmt::format("Binary(type({}), op({}), src({}), dst({}))",
                       pretty_print(node.type),
                       pretty_print(node.op),
                       pretty_print(node.src),
                       pretty_print(node.dst));
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
                        [](const left_shift_aritmetic &) { return "Left Shift Arithmetic"; },
                        [](const right_shift_aritmetic &) { return "Right Shift Arithmetic"; },
                      },
                      node);
}

std::string pretty_print(const call &node)
{
    return fmt::format("Call {}", node.fun_name.name);
}

std::string pretty_print(const cdq &node)
{
    return fmt::format("CDQ(type({})", pretty_print(node.type));
}

std::string pretty_print(const cmp &node)
{
    return fmt::format("Cmp(type({}), lhs({}), rhs({}))",
                       pretty_print(node.type),
                       pretty_print(node.lhs),
                       pretty_print(node.rhs));
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
                        [](A) { return "A"; },
                        [](AE) { return "AE"; },
                        [](B) { return "B"; },
                        [](BE) { return "BE"; },
                      },
                      node);
}

std::string pretty_print(const data &node)
{
    return fmt::format("Data({})", pretty_print(node.name));
}

std::string pretty_print(const div &node)
{
    return fmt::format("Div(type({}))", pretty_print(node.type));
}

std::string pretty_print(const function &node)
{
    return fmt::format("Function(name({}), is_global({}), stack_size({})\n\tinsts:\n{})",
                       pretty_print(node.name),
                       node.is_global,
                       node.stack_size,
                       pretty_print(node.instructions));
}

std::string pretty_print(const identifier &node)
{
    return fmt::format("{}", node.name);
}

std::string pretty_print(const idiv &node)
{
    return fmt::format("iDiv(type({}))", pretty_print(node.type));
}

std::string pretty_print(const immediate &node)
{
    return fmt::format("Imm({})", node.value);
}

std::string pretty_print(const instruction &node)
{
    return std::visit(visitor{
                        [](const mov_instruction &n) { return pretty_print(n); },
                        [](const movx &n) { return pretty_print(n); },
                        [](const mov_zero_extend &n) { return pretty_print(n); },
                        [](const unary &n) { return pretty_print(n); },
                        [](const binary &n) { return pretty_print(n); },
                        [](const cmp &n) { return pretty_print(n); },
                        [](const idiv &n) { return pretty_print(n); },
                        [](const div &n) { return pretty_print(n); },
                        [](const cdq &n) { return pretty_print(n); },
                        [](const jmp &n) { return pretty_print(n); },
                        [](const jmpcc &n) { return pretty_print(n); },
                        [](const setcc &n) { return pretty_print(n); },
                        [](const label &n) { return pretty_print(n); },
                        [](const push &n) { return pretty_print(n); },
                        [](const call &n) { return pretty_print(n); },
                        [](const ret_instruction &n) { return pretty_print(n); },
                      },
                      node);
}

std::string pretty_print(const jmp &node)
{
    return fmt::format("Jmp(target({}))", pretty_print(node.name));
}

std::string pretty_print(const jmpcc &node)
{
    return fmt::format("JmpCC(cond({}), target({}))", pretty_print(node.cond), pretty_print(node.name));
}

std::string pretty_print(const label &node)
{
    return fmt::format("Label({})", pretty_print(node.name));
}

std::string pretty_print(const mov_instruction &node)
{
    return fmt::format("Mov(type({}), src({}), dst({}))",
                       pretty_print(node.type),
                       pretty_print(node.src),
                       pretty_print(node.dst));
}

std::string pretty_print(const mov_zero_extend &node)
{
    return fmt::format("MovZeroExtend(src({}), dst({}))", pretty_print(node.src), pretty_print(node.dst));
}

std::string pretty_print(const movx &node)
{
    return fmt::format("Movx(src({}), dst({}))", pretty_print(node.src), pretty_print(node.dst));
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

std::string pretty_print(const program &node)
{
    std::string output;
    for (const auto &f : node.functions)
    {
        output += pretty_print(f) + "\n";
    }
    return output;
}

std::string pretty_print(const pseudo &node)
{
    return fmt::format("Pseudo({})", pretty_print(node.name));
}

std::string pretty_print(const push &node)
{
    return fmt::format("Push({})", pretty_print(node.src));
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
                        [](const SP &) { return "sp"; },
                      },
                      node);
}

std::string pretty_print(const ret_instruction &)
{
    return fmt::format("Ret");
}

std::string pretty_print(const setcc &node)
{
    return fmt::format("SetCC(cond({}), dst({}))", pretty_print(node.cond), pretty_print(node.dst));
}

std::string pretty_print(const top_level &node)
{
    return std::visit(visitor{ [](const function &n) { return pretty_print(n); },
                               [](const static_variable &n) { return pretty_print(n); } },
                      node);
}

std::string pretty_print(const stack &node)
{
    return fmt::format("Stack({})", pretty_print(node.value));
}

std::string pretty_print(const static_variable &node)
{
    return fmt::format("StaticVariable(name({}), global({}), align({}), initial({}))",
                       pretty_print(node.name),
                       node.is_global,
                       node.alignment,
                       pretty_print(node.init));
}

std::string pretty_print(const std::vector<instruction> &node)
{
    std::string pretty;
    for (const auto &i : node)
    {
        pretty += "\t\t";
        pretty += fmt::format("{}\n", pretty_print(i));
    }
    return pretty;
}

std::string pretty_print(const unary &node)
{
    return fmt::format("Unary(type({}), op({}), dst({}))",
                       pretty_print(node.type),
                       pretty_print(node.op),
                       pretty_print(node.dst));
}

std::string pretty_print(const unary_operator &node)
{
    return std::visit(visitor{ [](const not_op &) { return "Complement"; }, [](const neg_op &) { return "Negate"; } },
                      node);
}

} // namespace wccff::assembly_generation
