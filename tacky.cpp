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
        [](const parser::logical_not_operator &) -> unary_operator { return not_operator{}; },
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
    return pretty_print(p.function, ident);
}
} // namespace wccff::tacky
