#include "assembly_generation.h"
#include "visitor.h"

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
std::vector<instruction> process_statement(const wccff::tacky::return_statement &stmt)
{
    mov_instruction mov{ process_val(stmt.val), ax{} };
    ret_instruction ret{};

    return { mov, ret };
}

unary_operator process_unary_operator(const wccff::tacky::unary_operator &op)
{
    return std::visit(visitor{ [](const tacky::binary_complement_operator &) -> unary_operator { return not_op{}; },
                               [](const tacky::negate_operator &) -> unary_operator { return neg_op{}; } },
                      op);
}

std::vector<instruction> process_statement(const wccff::tacky::unary_statement &stmt)
{
    mov_instruction mov{ process_val(stmt.src), process_val(stmt.dst) };
    unary ret{ process_unary_operator(stmt.op), process_val(stmt.dst) };

    return { mov, ret };
}

std::vector<instruction> process_statement(const tacky::instruction &i)
{
    return std::visit(visitor{ [](const tacky::return_statement &n) { return process_statement(n); },
                               [](const tacky::unary_statement &n) { return process_statement(n); },
                               [](const tacky::binary_statement &n) { return process_statement(n); } },
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

function process_function(const wccff::tacky::function_definition &f)
{
    function asm_f;
    asm_f.name = process_identifier(f.name);
    asm_f.instructions = process_statement(f.instructions);
    return asm_f;
}

program process(const wccff::tacky::program &program)
{
    return { process_function(program.function) };
}

void replace_pseudo_registers_q(mov_instruction &i)
{
    if (std::holds_alternative<pseudo>(i.src))
    {
        auto r = std::get<pseudo>(i.src);
        i.src = stack{ table.get_address(r.name) };
    }
    if (std::holds_alternative<pseudo>(i.dst))
    {
        auto r = std::get<pseudo>(i.dst);
        i.dst = stack{ table.get_address(r.name) };
    }
}
void replace_pseudo_registers_q(unary &i)
{
    if (std::holds_alternative<pseudo>(i.dst))
    {
        auto r = std::get<pseudo>(i.dst);
        i.dst = stack{ table.get_address(r.name) };
    }
}
void replace_pseudo_registers_q(allocate_stack &i) {}
void replace_pseudo_registers_q(ret_instruction &i) {}

void replace_pseudo_registers(program &program)
{
    for (auto &i : program.function.instructions)
    {
        std::visit(visitor{ [](auto &inst) { replace_pseudo_registers_q(inst); } }, i);
    }
}

std::optional<std::vector<instruction>> fixing_up_instructions11(const mov_instruction &n)
{
    if (std::holds_alternative<stack>(n.src) && std::holds_alternative<stack>(n.dst))
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
std::optional<std::vector<instruction>> fixing_up_instructions1(const instruction &node)
{
    return std::visit(
      visitor{
        [](const mov_instruction &n) -> std::optional<std::vector<instruction>> { return fixing_up_instructions11(n); },
        [](const unary &) -> std::optional<std::vector<instruction>> { return std::nullopt; },
        [](const allocate_stack &) -> std::optional<std::vector<instruction>> { return std::nullopt; },
        [](const ret_instruction &) -> std::optional<std::vector<instruction>> { return std::nullopt; } },
      node);
}
void fixing_up_instructions(std::vector<instruction> &node)
{
    std::vector<instruction> tmp;

    auto stact_size = table.get_last_address();
    tmp.emplace_back(allocate_stack{ stact_size });

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
    fixing_up_instructions(node.instructions);
}

void fixing_up_instructions(program &node)
{
    fixing_up_instructions(node.function);
}

std::string pretty_print(const identifier &node)
{
    return fmt::format("{}", node.name);
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
    return std::visit(visitor{ [](const ax &) { return "ax"; }, [](const R10 &) { return "R10d"; } }, node);
}

std::string pretty_print(const pseudo &node)
{
    return fmt::format("Pseudo({})", pretty_print(node.name));
}

std::string pretty_print(const stack &node)
{
    return fmt::format("Stack({})", pretty_print(node.value));
}

std::string pretty_print(const operand &node)
{
    return std::visit(visitor{ [](const immediate &n) { return pretty_print(n); },
                               [](const reg &n) { return pretty_print(n); },
                               [](const pseudo &n) { return pretty_print(n); },
                               [](const stack &n) { return pretty_print(n); } },
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

std::string pretty_print(const allocate_stack &node)
{
    return fmt::format("Stack({})", pretty_print(node.size));
}

std::string pretty_print(const ret_instruction &node)
{
    return fmt::format("Ret");
}

std::string pretty_print(const instruction &node)
{
    return std::visit(visitor{ [](const mov_instruction &n) { return pretty_print(n); },
                               [](const unary &n) { return pretty_print(n); },
                               [](const allocate_stack &n) { return pretty_print(n); },
                               [](const ret_instruction &n) { return pretty_print(n); } },
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

std::string pretty_print(const program &node)
{
    return pretty_print(node.function);
}

} // namespace wccff::assembly_generation
