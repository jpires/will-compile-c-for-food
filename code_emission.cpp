#include "code_emission.h"

#include <fstream>

namespace wccff::code_emission {

template<class... Ts>
struct visitor : Ts...
{
    using Ts::operator()...;
};
template<class... Ts>
visitor(Ts...) -> visitor<Ts...>;

std::string process_identifier(const assembly_generation::identifier &identifier)
{
    return fmt::format("_{}", identifier.name);
}

std::string process_immediate(const assembly_generation::immediate &immediate)
{
    return fmt::format("${}", immediate.value);
}

std::string process_register(const assembly_generation::reg &node)
{
    return std::visit(visitor{ [](const assembly_generation::ax &) { return "%eax"; },
                               [](const assembly_generation::R10 &) { return "%r10d"; } },
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

std::string process_operand(const assembly_generation::operand &operand)
{
    return std::visit(
      visitor{ [](const assembly_generation::immediate &immediate) { return process_immediate(immediate); },
               [](const assembly_generation::reg &reg) { return process_register(reg); },
               [](const assembly_generation::pseudo &reg) { return process_pseudo(reg); },
               [](const assembly_generation::stack &reg) { return process_stack(reg); } },
      operand);
}

std::string process_mov_instruction(const assembly_generation::mov_instruction &mov)
{
    return fmt::format("movl {}, {}", process_operand(mov.src), process_operand(mov.dst));
}
std::string process_ret_instruction(const assembly_generation::ret_instruction &)
{
    return fmt::format("movq %rbp, %rsp\npopq %rbp\nret");
}

std::string process_unary_operator(const assembly_generation::unary_operator &node)
{
    return std::visit(visitor{ [](const assembly_generation::neg_op &) { return "negl"; },
                               [](const assembly_generation::not_op &) { return "notl"; } },
                      node);
}
std::string process_unary(const assembly_generation::unary &node)
{
    return fmt::format("{} {}", process_unary_operator(node.op), process_operand(node.dst));
}

std::string process_allocate_stack(const assembly_generation::allocate_stack &node)
{
    return fmt::format("subq ${}, %rsp", -node.size.value);
}

std::string process_instruction(const assembly_generation::instruction &instruction)
{
    return std::visit(visitor{
                        [](const assembly_generation::mov_instruction &mov) { return process_mov_instruction(mov); },
                        [](const assembly_generation::unary &node) { return process_unary(node); },
                        [](const assembly_generation::allocate_stack &node) { return process_allocate_stack(node); },
                        [](const assembly_generation::ret_instruction &ret) { return process_ret_instruction(ret); },
                      },
                      instruction);
}

std::string process_function(const assembly_generation::function &f)
{
    auto function_name = process_identifier(f.name);
    auto result = fmt::format(".globl {}\n{}:\n", function_name, function_name);
    result += fmt::format("pushq %rbp\nmovq %rsp, %rbp\n");
    for (const auto &i : f.instructions)
    {
        result += fmt::format("{}\n", process_instruction(i));
    }
    return result;
}
std::string process_program(const assembly_generation::program &p)
{
    return process_function(p.function);
}

void process(const std::filesystem::path &output_file, const assembly_generation::program &p)
{
    auto asm_listing = process_program(p);
    fmt::print("ASM: {}", asm_listing);

    std::ofstream out(output_file);
    out << asm_listing << std::endl;
}
} // namespace wccff::code_emission
