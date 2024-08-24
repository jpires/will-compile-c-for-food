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

std::string process_register(const assembly_generation::register_node &)
{
    return "%eax";
}

std::string process_operand(const assembly_generation::operand &operand)
{
    return std::visit(
      visitor{ [](const assembly_generation::immediate &immediate) { return process_immediate(immediate); },
               [](const assembly_generation::register_node &reg) { return process_register(reg); } },
      operand);
}

std::string process_mov_instruction(const assembly_generation::mov_instruction &mov)
{
    return fmt::format("movl {}, {}", process_operand(mov.src), process_operand(mov.dst));
}
std::string process_ret_instruction(const assembly_generation::ret_instruction &ret)
{
    return "ret";
}

std::string process_instruction(const assembly_generation::instruction &instruction)
{
    return std::visit(visitor{
                        [](const assembly_generation::mov_instruction &mov) { return process_mov_instruction(mov); },
                        [](const assembly_generation::ret_instruction &ret) { return process_ret_instruction(ret); },
                      },
                      instruction);
}

std::string process_function(const assembly_generation::function &f)
{
    auto function_name = process_identifier(f.name);
    auto result = fmt::format(".globl {}\n{}:\n", function_name, function_name);

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
