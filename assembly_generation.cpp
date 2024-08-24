#include "assembly_generation.h"

namespace wccff::assembly_generation {

identifier process_identifier(const wccff::parser::identifier &id)
{
    return { id.name };
}

immediate process_int_constant(const wccff::parser::int_constant &c)
{
    return { c.value };
}

operand process_expression(const wccff::parser::expression &e)
{
    return process_int_constant(std::get<parser::int_constant>(e));
}

std::vector<instruction> process_return_node(const wccff::parser::return_node &return_)
{
    mov_instruction mov{ process_expression(return_.e), register_node{} };
    ret_instruction ret{};

    return { mov, ret };
}

std::vector<instruction> process_statement(const wccff::parser::statement &s)
{
    return process_return_node(std::get<wccff::parser::return_node>(s));
}

function process_function(const wccff::parser::function &f)
{
    function asm_f;
    asm_f.name = process_identifier(f.function_name);
    asm_f.instructions = process_statement(f.body);
    return asm_f;
}

program process(const wccff::parser::program &program)
{
    return { process_function(program.f) };
}

} // namespace wccff::assembly_generation
