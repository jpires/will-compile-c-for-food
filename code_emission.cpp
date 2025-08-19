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

#include "code_emission.h"
#include "visitor.h"
#include <fstream>

namespace wccff::code_emission {

operand_size get_operand_size(const assembly_type &t)
{
    return std::visit(visitor{
                        [](const long_word &) { return operand_size::four_bytes; },
                        [](const quad_word &) { return operand_size::eight_bytes; },
                        [](const double_asm &) { return operand_size::eight_bytes; },
                      },
                      t);
}

std::string process_identifier(const assembly_generation::identifier &identifier)
{
    return fmt::format("_{}", identifier.name);
}

std::string process_immediate(const assembly_generation::immediate &immediate)
{
    return fmt::format("${}", immediate.value);
}

std::string process_register(const assembly_generation::reg &node, operand_size size = operand_size::four_bytes)
{
    if (size == operand_size::four_bytes)
    {
        return std::visit(visitor{
                            [](const assembly_generation::ax &) -> std::string { return "%eax"; },
                            [](const assembly_generation::cx &) -> std::string { return "%ecx"; },
                            [](const assembly_generation::dx &) -> std::string { return "%edx"; },
                            [](const assembly_generation::di &) -> std::string { return "%edi"; },
                            [](const assembly_generation::si &) -> std::string { return "%esi"; },
                            [](const assembly_generation::R8 &) -> std::string { return "%r8d"; },
                            [](const assembly_generation::R9 &) -> std::string { return "%r9d"; },
                            [](const assembly_generation::R10 &) -> std::string { return "%r10d"; },
                            [](const assembly_generation::R11 &) -> std::string { return "%r11d"; },
                            [](const assembly_generation::SP &) -> std::string { return "%esp"; },
                            [](const auto &) -> std::string { throw std::runtime_error("Invalid register"); },
                          },
                          node);
    }
    else if (size == operand_size::eight_bytes)
    {
        return std::visit(visitor{
                            [](const assembly_generation::ax &) -> std::string { return "%rax"; },
                            [](const assembly_generation::cx &) -> std::string { return "%rcx"; },
                            [](const assembly_generation::dx &) -> std::string { return "%rdx"; },
                            [](const assembly_generation::di &) -> std::string { return "%rdi"; },
                            [](const assembly_generation::si &) -> std::string { return "%rsi"; },
                            [](const assembly_generation::R8 &) -> std::string { return "%r8"; },
                            [](const assembly_generation::R9 &) -> std::string { return "%r9"; },
                            [](const assembly_generation::R10 &) -> std::string { return "%r10"; },
                            [](const assembly_generation::R11 &) -> std::string { return "%r11"; },
                            [](const assembly_generation::SP &) -> std::string { return "%rsp"; },
                            [](const assembly_generation::XMM0 &) -> std::string { return "%xmm0"; },
                            [](const assembly_generation::XMM1 &) -> std::string { return "%xmm1"; },
                            [](const assembly_generation::XMM2 &) -> std::string { return "%xmm2"; },
                            [](const assembly_generation::XMM3 &) -> std::string { return "%xmm3"; },
                            [](const assembly_generation::XMM4 &) -> std::string { return "%xmm4"; },
                            [](const assembly_generation::XMM5 &) -> std::string { return "%xmm5"; },
                            [](const assembly_generation::XMM6 &) -> std::string { return "%xmm6"; },
                            [](const assembly_generation::XMM7 &) -> std::string { return "%xmm7"; },
                            [](const assembly_generation::XMM14 &) -> std::string { return "%xmm14"; },
                            [](const assembly_generation::XMM15 &) -> std::string { return "%xmm15"; },
                            [](const auto &) -> std::string { throw std::runtime_error("Invalid register"); },
                          },
                          node);
    }
    return std::visit(visitor{
                        [](const assembly_generation::ax &) -> std::string { return "%al"; },
                        [](const assembly_generation::cx &) -> std::string { return "%cl"; },
                        [](const assembly_generation::dx &) -> std::string { return "%dl"; },
                        [](const assembly_generation::di &) -> std::string { return "%di"; },
                        [](const assembly_generation::si &) -> std::string { return "%si"; },
                        [](const assembly_generation::R8 &) -> std::string { return "%r8b"; },
                        [](const assembly_generation::R9 &) -> std::string { return "%r9b"; },
                        [](const assembly_generation::R10 &) -> std::string { return "%r10b"; },
                        [](const assembly_generation::R11 &) -> std::string { return "%r11b"; },
                        [](const assembly_generation::SP &) -> std::string { return "%sp"; },
                        [](const auto &) -> std::string { throw std::runtime_error("Invalid register"); },
                      },
                      node);
}
std::string process_pseudo(const assembly_generation::pseudo &node)
{
    return "ERROR";
}
std::string process_stack(const assembly_generation::stack &node)
{
    return fmt::format("{}(%rbp)", node.value.value);
}
std::string process_data(const assembly_generation::data &node)
{
    return fmt::format("{}(%rip)", process_identifier(node.name));
}

std::string process_cond_code(assembly_generation::cond_code cond)
{
    return std::visit(visitor{
                        [](const assembly_generation::E &) -> std::string { return "e"; },
                        [](const assembly_generation::NE &) -> std::string { return "ne"; },
                        [](const assembly_generation::L &) -> std::string { return "l"; },
                        [](const assembly_generation::LE &) -> std::string { return "le"; },
                        [](const assembly_generation::G &) -> std::string { return "g"; },
                        [](const assembly_generation::GE &) -> std::string { return "ge"; },
                        [](const assembly_generation::A &) -> std::string { return "a"; },
                        [](const assembly_generation::AE &) -> std::string { return "ae"; },
                        [](const assembly_generation::B &) -> std::string { return "b"; },
                        [](const assembly_generation::BE &) -> std::string { return "be"; },
                      },
                      cond);
}

std::string process_operand(const assembly_generation::operand &operand, operand_size size)
{
    return std::visit(visitor{
                        [](const assembly_generation::immediate &immediate) { return process_immediate(immediate); },
                        [size](const assembly_generation::reg &reg) { return process_register(reg, size); },
                        [](const assembly_generation::pseudo &reg) { return process_pseudo(reg); },
                        [](const assembly_generation::stack &reg) { return process_stack(reg); },
                        [](const assembly_generation::data &reg) { return process_data(reg); },
                      },
                      operand);
}

std::string_view process_assembly_type(const assembly_type &t)
{
    return std::visit(visitor{
                        [](const long_word &) { return "l"; },
                        [](const quad_word &) { return "q"; },
                        [](const double_asm &) { return "sd"; },
                      },
                      t);
}

std::string process_mov_instruction(const assembly_generation::mov_instruction &mov)
{
    if (std::holds_alternative<wccff::double_asm>(mov.type))
    {
        return fmt::format("movsd {}, {}",
                           process_operand(mov.src, operand_size::eight_bytes),
                           process_operand(mov.dst, operand_size::eight_bytes));
    }
    if (std::holds_alternative<wccff::quad_word>(mov.type))
    {
        return fmt::format("movq {}, {}",
                           process_operand(mov.src, operand_size::eight_bytes),
                           process_operand(mov.dst, operand_size::eight_bytes));
    }

    return fmt::format("movl {}, {}",
                       process_operand(mov.src, operand_size::four_bytes),
                       process_operand(mov.dst, operand_size::four_bytes));
}

std::string process_movx(const assembly_generation::movx &mov)
{
    return fmt::format("movslq {}, {}",
                       process_operand(mov.src, operand_size::four_bytes),
                       process_operand(mov.dst, operand_size::eight_bytes));
}

std::string process_ret_instruction(const assembly_generation::ret_instruction &)
{
    return fmt::format("movq %rbp, %rsp\npopq %rbp\nret");
}

std::string process_binary_operator(const assembly_generation::binary_operator &node, const assembly_type &t)
{
    auto sufix = process_assembly_type(t);

    auto operand = std::visit(visitor{
                                [](const assembly_generation::add &) { return "add"; },
                                [](const assembly_generation::sub &) { return "sub"; },
                                [](const assembly_generation::mul &) { return "imul"; },
                                [](const assembly_generation::binary_and &) { return "and"; },
                                [](const assembly_generation::binary_or &) { return "or"; },
                                [](const assembly_generation::binary_xor &) { return "xor"; },
                                [](const assembly_generation::left_shift &) { return "sal"; },
                                [](const assembly_generation::right_shift &) { return "sar"; },
                                [](const assembly_generation::left_shift_aritmetic &) { return "shl"; },
                                [](const assembly_generation::right_shift_aritmetic &) { return "shr"; },
                                [](const assembly_generation::div_double &) { return "div"; },
                                [](const auto &) { return "shr"; },
                              },
                              node);

    return fmt::format("{}{}", operand, sufix);
}

std::string process_unary_operator(const assembly_generation::unary_operator &node, const assembly_type &t)
{
    auto sufix = process_assembly_type(t);
    auto operand = std::visit(visitor{ [](const assembly_generation::neg_op &) { return "neg"; },
                                       [](const assembly_generation::not_op &) { return "not"; } },
                              node);
    return fmt::format("{}{}", operand, sufix);
}
std::string process_unary(const assembly_generation::unary &node)
{
    return fmt::format("{} {}",
                       process_unary_operator(node.op, node.type),
                       process_operand(node.dst, get_operand_size(node.type)),
                       process_operand(node.dst, get_operand_size(node.type)));
}
std::string process_binary(const assembly_generation::binary &node)
{
    if (std::holds_alternative<double_asm>(node.type))
    {
        if (std::holds_alternative<assembly_generation::binary_xor>(node.op))
        {
            return fmt::format("xorpd {}, {}",
                               process_operand(node.src, get_operand_size(node.type)),
                               process_operand(node.dst, get_operand_size(node.type)));
        }

        if (std::holds_alternative<assembly_generation::mul>(node.op))
        {
            return fmt::format("mulsd {}, {}",
                               process_operand(node.src, get_operand_size(node.type)),
                               process_operand(node.dst, get_operand_size(node.type)));
        }
    }

    if (std::holds_alternative<assembly_generation::left_shift>(node.op) ||
        std::holds_alternative<assembly_generation::right_shift>(node.op) ||
        std::holds_alternative<assembly_generation::left_shift_aritmetic>(node.op) ||
        std::holds_alternative<assembly_generation::right_shift_aritmetic>(node.op))
    {
        return fmt::format("{} {}, {}",
                           process_binary_operator(node.op, node.type),
                           process_operand(node.src, operand_size::one_byte),
                           process_operand(node.dst, get_operand_size(node.type)));
    }

    return fmt::format("{} {}, {}",
                       process_binary_operator(node.op, node.type),
                       process_operand(node.src, get_operand_size(node.type)),
                       process_operand(node.dst, get_operand_size(node.type)));
}

std::string process_cmp(const assembly_generation::cmp &node)
{
    if (std::holds_alternative<double_asm>(node.type))
    {
        return fmt::format("comisd {}, {}",
                           process_operand(node.lhs, get_operand_size(node.type)),
                           process_operand(node.rhs, get_operand_size(node.type)));
    }
    return fmt::format("cmp{} {}, {}",
                       process_assembly_type(node.type),
                       process_operand(node.lhs, get_operand_size(node.type)),
                       process_operand(node.rhs, get_operand_size(node.type)));
}

std::string process_cmp(const assembly_generation::cvtsi2sd &node)
{
    return fmt::format("cvtsi2sd{} {}, {}",
                       process_assembly_type(node.src_type),
                       process_operand(node.src, get_operand_size(node.src_type)),
                       process_operand(node.dst, operand_size::eight_bytes));
}

std::string process_cmp(const assembly_generation::cvttsd2si &node)
{
    return fmt::format("cvttsd2si{} {}, {}",
                       process_assembly_type(node.dst_type),
                       process_operand(node.src, operand_size::eight_bytes),
                       process_operand(node.dst, get_operand_size(node.dst_type)));
}

std::string process_div(const assembly_generation::div &node)
{
    return fmt::format("div{} {}",
                       process_assembly_type(node.type),
                       process_operand(node.src, get_operand_size(node.type)));
}

std::string process_idiv(const assembly_generation::idiv &node)
{
    return fmt::format("idiv{} {}",
                       process_assembly_type(node.type),
                       process_operand(node.src, get_operand_size(node.type)));
}
std::string process_cdq(const assembly_generation::cdq &node)
{
    if (std::holds_alternative<quad_word>(node.type))
    {
        return fmt::format("cqo");
    }
    return fmt::format("cdq");
}
std::string process_jmp(const assembly_generation::jmp &node)
{
    return fmt::format("jmp L{}", process_identifier(node.name));
}
std::string process_jmpcc(const assembly_generation::jmpcc &node)
{
    return fmt::format("j{} L{}", process_cond_code(node.cond), process_identifier(node.name));
}
std::string process_setcc(const assembly_generation::setcc &node)
{
    return fmt::format("set{} {}", process_cond_code(node.cond), process_operand(node.dst, operand_size::one_byte));
}
std::string process_label(const assembly_generation::label &node)
{
    return fmt::format("L{}:", process_identifier(node.name));
}
std::string process_push(const assembly_generation::push &node)
{
    return fmt::format("pushq {}", process_operand(node.src, operand_size::eight_bytes));
}
std::string process_call(const assembly_generation::call &node)
{
    return fmt::format("call {}", process_identifier(node.fun_name));
}

std::string process_instruction(const assembly_generation::instruction &instruction)
{
    return std::visit(visitor{
                        [](const assembly_generation::mov_instruction &mov) { return process_mov_instruction(mov); },
                        [](const assembly_generation::movx &mov) { return process_movx(mov); },
                        [](const assembly_generation::unary &node) { return process_unary(node); },
                        [](const assembly_generation::binary &node) { return process_binary(node); },
                        [](const assembly_generation::cmp &node) { return process_cmp(node); },
                        [](const assembly_generation::cvtsi2sd &node) { return process_cmp(node); },
                        [](const assembly_generation::cvttsd2si &node) { return process_cmp(node); },
                        [](const assembly_generation::div &node) { return process_div(node); },
                        [](const assembly_generation::idiv &node) { return process_idiv(node); },
                        [](const assembly_generation::cdq &node) { return process_cdq(node); },
                        [](const assembly_generation::jmp &node) { return process_jmp(node); },
                        [](const assembly_generation::jmpcc &node) { return process_jmpcc(node); },
                        [](const assembly_generation::setcc &node) { return process_setcc(node); },
                        [](const assembly_generation::label &node) { return process_label(node); },
                        [](const assembly_generation::push &node) -> std::string { return process_push(node); },
                        [](const assembly_generation::call &node) -> std::string { return process_call(node); },
                        [](const assembly_generation::ret_instruction &ret) { return process_ret_instruction(ret); },
                        [](const auto &) -> std::string { throw std::runtime_error("Not Implemented"); },
                      },
                      instruction);
}

std::string process_function(const assembly_generation::function &f)
{
    auto function_name = process_identifier(f.name);
    auto globl = f.is_global ? fmt::format(".globl {}\n.text", function_name) : ".text";
    auto result = fmt::format("{}\n{}:\n", globl, function_name);
    result += fmt::format("pushq %rbp\nmovq %rsp, %rbp\n");
    for (const auto &i : f.instructions)
    {
        result += fmt::format("{}\n", process_instruction(i));
    }
    return result;
}

std::string process_static_variable(const assembly_generation::static_variable &f)
{
    auto get_var_section = [](const wccff::initial &init) {
        return std::visit(
          visitor{
            [](const auto &i) { return i.value == 0 ? ".bss" : ".data"; },
          },
          init);
    };
    auto get_init_value = [](const wccff::initial &init) {
        return std::visit(visitor{ [](const int_initial &i) {
                                      if (i.value == 0)
                                      {
                                          return fmt::format(".zero 4");
                                      }
                                      return fmt::format(".long {}", i.value);
                                  },
                                   [](const long_initial &i) {
                                       if (i.value == 0)
                                       {
                                           return fmt::format(".zero 8");
                                       }
                                       return fmt::format(".quad {}", i.value);
                                   },
                                   [](const unsigned_int_initial &i) {
                                       if (i.value == 0)
                                       {
                                           return fmt::format(".zero 4");
                                       }
                                       return fmt::format(".long {}", i.value);
                                   },
                                   [](const unsigned_long_initial &i) {
                                       if (i.value == 0)
                                       {
                                           return fmt::format(".zero 8");
                                       }
                                       return fmt::format(".quad {}", i.value);
                                   },
                                   [](const double_initial &i) { return fmt::format(".double {}", i.value); },
                                   [](const auto &) -> std::string { throw std::runtime_error("Not implemented"); } },
                          init);
    };

    auto function_name = process_identifier(f.name);
    auto globl = f.is_global ? fmt::format(".globl {}", function_name) : "";
    auto section = get_var_section(f.init);
    auto init_value = get_init_value(f.init);

    return fmt::format("\t{}\n\t{}\n\t.balign {}\n{}:\n\t{}\n", globl, section, f.alignment, function_name, init_value);
}

std::string process_static_constant(const assembly_generation::static_constant &f)
{
    auto init = std::get<double_initial>(f.init);
    if (f.alignment == 8)
    {
        return fmt::format("\t.literal8\n\t.balign 8\n_{}:\n\t.double {}\n", f.name.name, init.value);
    }
    else
    {
        return fmt::format("\t.literal16\n\t.balign 16\n_{}:\n\t.double {}\n.quad 0\n", f.name.name, init.value);
    }
}

std::string process_top_level(const assembly_generation::top_level &t)
{
    return std::visit(visitor{
                        [](const assembly_generation::function &f) { return process_function(f); },
                        [](const assembly_generation::static_variable &f) { return process_static_variable(f); },
                        [](const assembly_generation::static_constant &f) { return process_static_constant(f); },
                      },
                      t);
}
std::string process_program(const assembly_generation::program &p)
{
    std::string output;

    for (auto const &f : p.functions)
    {
        output += process_top_level(f);
    }
    return output;
}

void process(const std::filesystem::path &output_file, const assembly_generation::program &p)
{
    auto asm_listing = process_program(p);
    fmt::print("ASM: {}", asm_listing);

    std::ofstream out(output_file);
    out << asm_listing << std::endl;
}
} // namespace wccff::code_emission
