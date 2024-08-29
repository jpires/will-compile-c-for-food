#include "tacky.h"
#include "visitor.h"
#include <fmt/format.h>

namespace wccff::tacky {

std::string get_temporary_name()
{
    static int counter = 0;
    return fmt::format("tacky-{}", ++counter);
}

identifier process_identifier(const parser::identifier &id)
{
    return { id.name };
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
      },
      op);
}

binary_operator process_binary_operator(const parser::binary_operator &op)
{
    return std::visit(visitor{
                        [](const parser::plus_operator &) -> binary_operator { return plus_operator{}; },
                        [](const parser::subtract_operator &) -> binary_operator { return subtract_operator{}; },
                        [](const parser::multiply_operator &) -> binary_operator { return multiply_operator{}; },
                        [](const parser::divide_operator &) -> binary_operator { return divide_operator{}; },
                        [](const parser::remainder_operator &) -> binary_operator { return remainder_operator{}; },
                      },
                      op);
}

val process_unary_node(const std::unique_ptr<parser::unary_node> &node, std::vector<instruction> &instructions)
{
    auto src = process_expression(node->exp, instructions);
    auto dst = var{ get_temporary_name() };
    auto op = process_unary_operator(node->op);
    instructions.emplace_back(unary_statement{ op, src, dst });
    return dst;
}

val process_binary_node(const std::unique_ptr<parser::binary_node> &node, std::vector<instruction> &instructions)
{
    auto v1 = process_expression(node->left, instructions);
    auto v2 = process_expression(node->right, instructions);
    auto dst = var{ get_temporary_name() };
    auto op = process_binary_operator(node->op);
    instructions.emplace_back(binary_statement{ op, v1, v2, dst });
    return dst;
}

val process_expression(const wccff::parser::expression &exp, std::vector<instruction> &instructions)
{
    return std::visit(visitor{ [](const parser::int_constant &c) -> val { return process_int_constant(c); },
                               [&instructions](const std::unique_ptr<parser::unary_node> &n) -> val {
                                   return process_unary_node(n, instructions);
                               },
                               [&instructions](const std::unique_ptr<parser::binary_node> &n) -> val {
                                   return process_binary_node(n, instructions);
                               } },
                      exp);
}

std::vector<instruction> process_return_node(const wccff::parser::return_node &stmt)
{
    std::vector<instruction> instructions;
    auto node = return_statement{ process_expression(stmt.e, instructions) };
    instructions.emplace_back(return_statement{ node });
    return instructions;
}

std::vector<instruction> process_statement(const wccff::parser::statement &s)
{
    return process_return_node(std::get<wccff::parser::return_node>(s));
}
function_definition process_function_definition(const parser::function &f)
{
    return { process_identifier(f.function_name), process_statement(f.body) };
}

program process(const parser::program &input)
{
    return { process_function_definition(input.f) };
}

std::string pretty_print(const unary_operator &op, int32_t ident)
{
    return std::visit(visitor{ [](const binary_complement_operator &) { return fmt::format("Complement"); },
                               [ident](const negate_operator &) { return fmt::format("Negate"); } },
                      op);
}
std::string pretty_print(const binary_operator &op, int32_t ident)
{
    return std::visit(visitor{
                        [ident](const plus_operator &) { return fmt::format("Plus"); },
                        [ident](const subtract_operator &) { return fmt::format("Subtract"); },
                        [ident](const multiply_operator &) { return fmt::format("Multiply"); },
                        [ident](const divide_operator &) { return fmt::format("Divide"); },
                        [ident](const remainder_operator &) { return fmt::format("Remainder"); },

                      },
                      op);
}
std::string pretty_print(const constant &val, int32_t ident)
{
    return fmt::format("Constant({})", val.value);
}
std::string pretty_print(const var &var, int32_t ident)
{
    return fmt::format("Var({})", var.id.name);
}
std::string pretty_print(const val &val, int32_t ident)
{
    return std::visit(visitor{ [ident](const auto &n) { return pretty_print(n, ident); } }, val);
}
std::string pretty_print(const return_statement &instruction, int32_t ident)
{
    return fmt::format("Return({})\n", pretty_print(instruction.val, ident));
}
std::string pretty_print(const unary_statement &i, int32_t ident)
{
    return fmt::format("Unary({}, {}, {})\n",
                       pretty_print(i.op, ident),
                       pretty_print(i.src, ident),
                       pretty_print(i.dst, ident));
}
std::string pretty_print(const binary_statement &i, int32_t ident)
{
    return fmt::format("Binary({}, {}, {}, {})\n",
                       pretty_print(i.op, ident),
                       pretty_print(i.src1, ident),
                       pretty_print(i.src2, ident),
                       pretty_print(i.dst, ident));
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
    return fmt::format("Function({})\n{}", f.name.name, pretty_print(f.instructions, ident));
}

std::string pretty_print(const program &p, int ident)
{
    return pretty_print(p.function, ident);
}
} // namespace wccff::tacky
