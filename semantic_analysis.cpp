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

#include "semantic_analysis.h"

#include "visitor.h"
namespace wccff::sema {

static std::unexpected<semantic_error> generate_unknown_variable(parser::identifier var_name)
{
    auto msg = fmt::format("Unknown variable '{}'", var_name.name);
    return std::unexpected<semantic_error>(msg);
}

void variable_map::add(const parser::identifier &name, const parser::identifier &unique_name)
{
    m_map.insert({ name.name, unique_name.name });
}
bool variable_map::contains(const parser::identifier &name) const
{
    return m_map.contains(name.name);
}
parser::identifier variable_map::get_unique_name(const parser::identifier &name) const
{
    return { m_map.at(name.name) };
}
parser::identifier variable_map::generate_unique_name(const parser::identifier &name)
{
    return { fmt::format("var-{}-{}", name.name, m_counter++) };
}

std::expected<parser::program, semantic_error> analyse(const parser::program &input)
{
    variable_map variable_map;
    return variable_resolution(input, variable_map);
}

bool is_lvalue(const std::unique_ptr<parser::binary_node> &e)
{
    return false;
}
bool is_lvalue(const std::unique_ptr<parser::unary_node> &e)
{
    return is_lvalue(e->exp);
}
bool is_lvalue(const parser::expression &e)
{
    return std::visit(visitor{
                        [&](const std::unique_ptr<parser::assignment_node> &n) { return false; },
                        [&](const std::unique_ptr<parser::conditional_node> &n) { return false; },
                        [&](const std::unique_ptr<parser::binary_node> &n) { return is_lvalue(n); },
                        [&](const std::unique_ptr<parser::unary_node> &n) { return is_lvalue(n); },
                        [&](const parser::var &n) { return true; },
                        [&](const parser::int_constant &n) { return false; },
                      },
                      e);
}
std::expected<std::unique_ptr<parser::assignment_node>, semantic_error> resolve_assignment_node(
  const std::unique_ptr<parser::assignment_node> &node,
  variable_map &variable_map)
{
    if (std::holds_alternative<parser::var>(node->lhs) == false)
    {
        auto msg = fmt::format("Trying to assign a value to a non variable");
        return std::unexpected{ semantic_error{ msg } };
    }

    auto left = resolve_expression(node->lhs, variable_map);
    if (left.has_value() == false)
    {
        return std::unexpected{ left.error() };
    }
    auto right = resolve_expression(node->rhs, variable_map);
    if (right.has_value() == false)
    {
        return std::unexpected{ right.error() };
    }
    return std::make_unique<parser::assignment_node>(node->op, std::move(left.value()), std::move(right.value()));
}

std::expected<parser::block_item, semantic_error> resolve_block_item(const parser::block_item &input,
                                                                     variable_map &variable_map)
{
    return std::visit(
      visitor{
        [&variable_map](const parser::declaration &n) mutable -> std::expected<parser::block_item, semantic_error> {
            auto d = resolve_declaration(n, variable_map);
            if (d.has_value() == false)
            {
                return std::unexpected{ d.error() };
            }
            return parser::block_item{ std::move(d.value()) };
        },
        [&variable_map](const parser::statement &n) mutable -> std::expected<parser::block_item, semantic_error> {
            auto stmt = resolve_statement(n, variable_map);
            if (stmt.has_value() == false)
            {
                return std::unexpected{ stmt.error() };
            }
            return parser::block_item{ std::move(stmt.value()) };
        },
        [](std::monostate &) -> std::expected<parser::block_item, semantic_error> {
            return parser::block_item{ std::monostate{} };
        },
      },
      input);
}

std::expected<std::unique_ptr<parser::conditional_node>, semantic_error> resolve_conditional_node(
  const std::unique_ptr<parser::conditional_node> &node,
  variable_map &variable_map)
{
    auto cond = resolve_expression(node->condition, variable_map);
    if (cond.has_value() == false)
    {
        return std::unexpected{ cond.error() };
    }
    auto e1 = resolve_expression(node->e1, variable_map);
    if (e1.has_value() == false)
    {
        return std::unexpected{ e1.error() };
    }

    auto e2 = resolve_expression(node->e2, variable_map);
    if (e2.has_value() == false)
    {
        return std::unexpected{ e2.error() };
    }

    return std::make_unique<parser::conditional_node>(std::move(cond.value()),
                                                      std::move(e1.value()),
                                                      std::move(e2.value()));
}
std::expected<parser::declaration, semantic_error> resolve_declaration(const parser::declaration &input,
                                                                       variable_map &variable_map)
{
    if (variable_map.contains(input.name))
    {
        auto msg = fmt::format("Duplicate definition for variable {}", input.name.name);
        return std::unexpected{ semantic_error{ msg } };
    }

    parser::identifier unique_name = variable_map.generate_unique_name(input.name);
    variable_map.add(input.name, unique_name);

    std::optional<parser::expression> init;
    if (input.init.has_value())
    {
        auto a = resolve_expression(input.init.value(), variable_map);
        if (a.has_value() == false)
        {
            return std::unexpected{ a.error() };
        }
        init = std::move(a.value());
    }

    return parser::declaration{ unique_name, std::move(init) };
}

std::expected<parser::expression, semantic_error> resolve_expression(const parser::expression &input,
                                                                     variable_map &variable_map)
{
    return std::visit(
      visitor{
        [&](const std::unique_ptr<parser::assignment_node> &n) -> std::expected<parser::expression, semantic_error> {
            return resolve_assignment_node(n, variable_map);
        },
        [&](const std::unique_ptr<parser::conditional_node> &n) -> std::expected<parser::expression, semantic_error> {
            return resolve_conditional_node(n, variable_map);
        },
        [&](const std::unique_ptr<parser::binary_node> &n) -> std::expected<parser::expression, semantic_error> {
            return resolve_binary_node(n, variable_map);
        },
        [&](const std::unique_ptr<parser::unary_node> &n) -> std::expected<parser::expression, semantic_error> {
            return resolve_unary_node(n, variable_map);
        },
        [&](const parser::var &n) -> std::expected<parser::expression, semantic_error> {
            return resolve_var(n, variable_map);
        },
        [&](const parser::int_constant &n) -> std::expected<parser::expression, semantic_error> { return n; },
      },
      input);
}

std::expected<parser::function, semantic_error> resolve_function(const parser::function &input,
                                                                 variable_map &variable_map)
{
    std::vector<parser::block_item> items;
    for (const auto &b : input.body)
    {
        auto new_block = resolve_block_item(b, variable_map);
        if (new_block.has_value() == false)
        {
            return std::unexpected{ new_block.error() };
        }
        items.push_back(std::move(new_block.value()));
    }

    return parser::function{ input.function_name, std::move(items) };
}

std::expected<std::unique_ptr<parser::if_node>, semantic_error> resolve_if_node(
  const std::unique_ptr<parser::if_node> &node,
  variable_map &variable_map)
{
    auto op = resolve_expression(node->op, variable_map);
    if (op.has_value() == false)
    {
        return std::unexpected{ op.error() };
    }
    auto then_stmt = resolve_statement(node->then_stmt, variable_map);
    if (then_stmt.has_value() == false)
    {
        return std::unexpected{ then_stmt.error() };
    }
    if (node->else_stmt.has_value())
    {
        auto else_stmt = resolve_statement(node->else_stmt.value(), variable_map);
        if (else_stmt.has_value() == false)
        {
            return std::unexpected{ else_stmt.error() };
        }
        return std::make_unique<parser::if_node>(std::move(op.value()),
                                                 std::move(then_stmt.value()),
                                                 std::move(else_stmt.value()));
    }
    return std::make_unique<parser::if_node>(std::move(op.value()), std::move(then_stmt.value()), std::nullopt);
}

std::expected<parser::return_node, semantic_error> resolve_return_node(const parser::return_node &input,
                                                                       variable_map &variable_map)
{
    auto e = resolve_expression(input.e, variable_map);
    if (e.has_value() == false)
    {
        return std::unexpected{ e.error() };
    }
    return parser::return_node{ std::move(e.value()) };
}

std::expected<std::unique_ptr<parser::binary_node>, semantic_error> resolve_binary_node(
  const std::unique_ptr<parser::binary_node> &node,
  variable_map &variable_map)
{
    auto left = resolve_expression(node->left, variable_map);
    if (left.has_value() == false)
    {
        return std::unexpected{ left.error() };
    }

    auto right = resolve_expression(node->right, variable_map);
    if (right.has_value() == false)
    {
        return std::unexpected{ right.error() };
    }
    return std::make_unique<parser::binary_node>(node->op, std::move(left.value()), std::move(right.value()));
}

std::expected<std::unique_ptr<parser::unary_node>, semantic_error> resolve_unary_node(
  const std::unique_ptr<parser::unary_node> &node,
  variable_map &variable_map)
{
    if (std::holds_alternative<parser::prefix_decrement_operator>(node->op) ||
        std::holds_alternative<parser::prefix_increment_operator>(node->op) ||
        std::holds_alternative<parser::postfix_decrement_operator>(node->op) ||
        std::holds_alternative<parser::postfix_increment_operator>(node->op))
    {
        if (is_lvalue(node->exp) == false)
        {
            return std::unexpected{ semantic_error{
              "Not a lvalue expression on prefix or postfix increment/decrement" } };
        }
    }

    auto exp = resolve_expression(node->exp, variable_map);
    if (exp.has_value() == false)
    {
        return std::unexpected{ exp.error() };
    }

    return std::make_unique<parser::unary_node>(node->op, std::move(exp.value()));
}

std::expected<parser::var, semantic_error> resolve_var(const parser::var &node, variable_map &variable_map)
{
    if (variable_map.contains(node.name) == false)
    {
        return std::unexpected<semantic_error>{ fmt::format("Variable {} not defined", node.name.name) };
    }

    return parser::var{ variable_map.get_unique_name(node.name) };
}

std::expected<parser::statement, semantic_error> resolve_statement(const parser::statement &input,
                                                                   variable_map &variable_map)
{
    return std::visit(
      visitor{
        [&](const parser::return_node &n) -> std::expected<parser::statement, semantic_error> {
            return resolve_return_node(n, variable_map);
        },
        [&](const parser::expression &n) -> std::expected<parser::statement, semantic_error> {
            return resolve_expression(n, variable_map);
        },
        [&](const std::unique_ptr<parser::if_node> &n) -> std::expected<parser::statement, semantic_error> {
            return resolve_if_node(n, variable_map);
        },
        [&](const std::monostate &n) -> std::expected<parser::statement, semantic_error> { return std::monostate{}; },
      },
      input);
}
std::expected<parser::program, semantic_error> variable_resolution(const parser::program &input,
                                                                   variable_map &variable_map)
{

    auto f = resolve_function(input.f, variable_map);
    if (f.has_value() == false)
    {
        return std::unexpected{ f.error() };
    }
    return parser::program{ std::move(f.value()) };
}
}