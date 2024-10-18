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
#include <algorithm>
#include <ranges>

namespace wccff::sema {

static std::unexpected<semantic_error> generate_unknown_variable(parser::identifier var_name)
{
    auto msg = fmt::format("Unknown variable '{}'", var_name.name);
    return std::unexpected<semantic_error>(msg);
}

static parser::identifier generate_loop_label()
{
    static int32_t counter = 0;
    return parser::identifier(fmt::format("loop_{}", counter++));
}

parser::identifier variable_map::add(const parser::identifier &name)
{
    auto unique_name = generate_unique_name(name);
    symbol s{ name.name, unique_name.name };

    m_map.back().insert({ name.name, s });

    return unique_name;
}
bool variable_map::contains(const parser::identifier &name, scopes on_current_scope) const
{
    // In this situation, we want to know if this variable exists in the current scope.
    // So, we only need to check the latest entry.
    if (on_current_scope == scopes::current_scope)
    {
        return m_map.back().contains(name.name);
    }

    // We want to look at all scopes, going in reverse order as they were declared.
    // i.e. a variable that is declared in two scope, then we want the innermost definition
    // of that variable.
    return std::ranges::any_of(m_map, [&](const auto &pair) { return pair.contains(name.name); });
}
void variable_map::create_scope()
{
    m_map.emplace_back();
    m_scope_counter++;
}
void variable_map::destroy_scope()
{
    m_map.pop_back();
    m_scope_counter--;
}
parser::identifier variable_map::get_unique_name(const parser::identifier &name) const
{
    for (auto const &pair : std::ranges::reverse_view(m_map))
    {
        if (pair.contains(name.name))
        {
            return { pair.at(name.name).unique_name };
        }
    }
    throw std::runtime_error("variable map does not exist");
}
parser::identifier variable_map::generate_unique_name(const parser::identifier &name)
{
    return { fmt::format("var-{}-{}", name.name, m_counter++) };
}

std::expected<parser::program, semantic_error> analyse(const parser::program &input)
{
    variable_map variable_map;
    auto var_result = variable_resolution(input, variable_map);
    if (var_result.has_value() == false)
    {
        return std::unexpected{ var_result.error() };
    }

    return loop_labelling_program(var_result.value());
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

std::expected<parser::block, semantic_error> loop_labelling_block(const parser::block &node,
                                                                  const std::optional<parser::identifier> &label)
{
    std::vector<parser::block_item> items;
    for (const auto &i : node.items)
    {
        auto result = loop_labelling_block_item(i, label);
        if (result.has_value() == false)
        {
            return std::unexpected{ result.error() };
        }
        items.emplace_back(std::move(result.value()));
    }

    return parser::block{ std::move(items) };
}
std::expected<parser::block_item, semantic_error> loop_labelling_block_item(
  const parser::block_item &node,
  const std::optional<parser::identifier> &label)
{
    return std::visit(visitor{
                        [](const parser::declaration &n) -> std::expected<parser::block_item, semantic_error> {
                            return parser::copy_declaration(n);
                        },
                        [label](const parser::statement &n) -> std::expected<parser::block_item, semantic_error> {
                            return loop_labelling_statement(n, label);
                        },
                        [](const std::monostate &) -> std::expected<parser::block_item, semantic_error> {
                            return parser::block_item{ std::monostate{} };
                        },
                      },
                      node);
}

std::expected<parser::break_statement, semantic_error> loop_labelling_break_statement(
  const std::optional<parser::identifier> &label)
{
    if (label.has_value() == false)
    {
        return std::unexpected{ semantic_error{ fmt::format("Break statement outside a loop") } };
    }
    return parser::break_statement{ label.value() };
}

std::expected<std::unique_ptr<parser::compound_statement>, semantic_error> loop_labelling_compound_statement(
  const std::unique_ptr<parser::compound_statement> &node,
  const std::optional<parser::identifier> &label)
{
    auto tmp = loop_labelling_block(node->block, label);
    if (tmp.has_value() == false)
    {
        return std::unexpected{ tmp.error() };
    }
    return std::make_unique<parser::compound_statement>(std::move(tmp.value()));
}

std::expected<parser::continue_statement, semantic_error> loop_labelling_continue_statement(
  const std::optional<parser::identifier> &label)
{
    if (label.has_value() == false)
    {
        return std::unexpected{ semantic_error{ fmt::format("Continue statement outside a loop") } };
    }
    return parser::continue_statement{ label.value() };
}

std::expected<std::unique_ptr<parser::do_while_statement>, semantic_error> loop_labelling_do_while_statement(
  const std::unique_ptr<parser::do_while_statement> &node,
  const std::optional<parser::identifier> &label)
{
    parser::identifier new_label = generate_loop_label();
    auto body = loop_labelling_statement(node->body, new_label);
    if (body.has_value() == false)
    {
        return std::unexpected{ body.error() };
    }
    auto condition = parser::copy_expression(node->condition);

    return std::make_unique<parser::do_while_statement>(std::move(body.value()), std::move(condition), new_label);
}

std::expected<std::unique_ptr<parser::for_statement>, semantic_error> loop_labelling_for_statement(
  const std::unique_ptr<parser::for_statement> &node,
  const std::optional<parser::identifier> &label)
{
    parser::identifier new_label = generate_loop_label();
    parser::for_init init = std::visit(visitor{
                                         [](const parser::init_declaration &n) -> parser::for_init {
                                             return parser::init_declaration{ parser::copy_declaration(n.decl) };
                                         },
                                         [](const parser::init_expression &n) -> parser::for_init {
                                             return n.expression.has_value()
                                                      ? parser::init_expression{ copy_expression(n.expression.value()) }
                                                      : parser::init_expression{ std::nullopt };
                                         },
                                       },
                                       node->init);

    std::optional<parser::expression> cond;
    if (node->condition.has_value())
    {
        cond = parser::copy_expression(node->condition.value());
    }

    std::optional<parser::expression> post;
    if (node->post.has_value())
    {
        post = parser::copy_expression(node->post.value());
    }

    auto body = loop_labelling_statement(node->body, new_label);

    return std::make_unique<parser::for_statement>(std::move(init),
                                                   std::move(cond),
                                                   std::move(post),
                                                   std::move(body.value()),
                                                   new_label);
}

std::expected<parser::function, semantic_error> loop_labelling_function(const parser::function &node)
{
    auto b = loop_labelling_block(node.body, std::nullopt);
    if (b.has_value() == false)
    {
        return std::unexpected{ b.error() };
    }

    return parser::function{ node.function_name, std::move(b.value()) };
}

std::expected<std::unique_ptr<parser::if_node>, semantic_error> loop_labelling_if_node(
  const std::unique_ptr<parser::if_node> &node,
  const std::optional<parser::identifier> &label)
{
    auto then_stmt = loop_labelling_statement(node->then_stmt, label);
    if (then_stmt.has_value() == false)
    {
        return std::unexpected{ then_stmt.error() };
    }

    std::optional<parser::statement> else_stmt;
    if (node->else_stmt.has_value())
    {
        auto tmp = loop_labelling_statement(node->else_stmt.value(), label);
        if (tmp.has_value() == false)
        {
            return std::unexpected{ tmp.error() };
        }
        else_stmt = std::move(tmp.value());
    }

    return std::make_unique<parser::if_node>(parser::copy_expression(node->op),
                                             std::move(then_stmt.value()),
                                             std::move(else_stmt));
}

std::expected<parser::program, semantic_error> loop_labelling_program(const parser::program &node)
{
    auto f = loop_labelling_function(node.f);
    if (f.has_value() == false)
    {
        return std::unexpected{ f.error() };
    }
    return parser::program{ std::move(f.value()) };
}

std::expected<parser::statement, semantic_error> loop_labelling_statement(
  const parser::statement &node,
  const std::optional<parser::identifier> &label)
{
    return std::visit(
      visitor{
        [](const parser::return_node &n) -> std::expected<parser::statement, semantic_error> {
            return parser::return_node{ parser::copy_expression(n.e) };
        },
        [](const parser::expression &n) -> std::expected<parser::statement, semantic_error> {
            return parser::copy_expression(n);
        },
        [label](const std::unique_ptr<parser::if_node> &n) -> std::expected<parser::statement, semantic_error> {
            return loop_labelling_if_node(n, label);
        },
        [label](const std::unique_ptr<parser::compound_statement> &n)
          -> std::expected<parser::statement, semantic_error> { return loop_labelling_compound_statement(n, label); },
        [label](const parser::break_statement &) -> std::expected<parser::statement, semantic_error> {
            return loop_labelling_break_statement(label);
        },
        [label](const parser::continue_statement &) -> std::expected<parser::statement, semantic_error> {
            return loop_labelling_continue_statement(label);
        },
        [label](const std::unique_ptr<parser::while_statement> &n) -> std::expected<parser::statement, semantic_error> {
            return loop_labelling_while_statement(n, label);
        },
        [label](const std::unique_ptr<parser::do_while_statement> &n)
          -> std::expected<parser::statement, semantic_error> { return loop_labelling_do_while_statement(n, label); },
        [label](const std::unique_ptr<parser::for_statement> &n) -> std::expected<parser::statement, semantic_error> {
            return loop_labelling_for_statement(n, label);
        },
        [](const std::monostate &) -> std::expected<parser::statement, semantic_error> { return std::monostate{}; },
      },
      node);
}

std::expected<std::unique_ptr<parser::while_statement>, semantic_error> loop_labelling_while_statement(
  const std::unique_ptr<parser::while_statement> &node,
  const std::optional<parser::identifier> &label)
{
    parser::identifier new_label = generate_loop_label();
    auto condition = parser::copy_expression(node->condition);

    auto body = loop_labelling_statement(node->body, new_label);
    if (body.has_value() == false)
    {
        return std::unexpected{ body.error() };
    }

    return std::make_unique<parser::while_statement>(std::move(condition), std::move(body.value()), new_label);
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

std::expected<parser::block, semantic_error> resolve_block(const parser::block &node, variable_map &variable_map)
{
    parser::block block;
    for (const auto &b : node.items)
    {
        auto new_block = resolve_block_item(b, variable_map);
        if (new_block.has_value() == false)
        {
            return std::unexpected{ new_block.error() };
        }
        block.items.push_back(std::move(new_block.value()));
    }

    return block;
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

std::expected<std::unique_ptr<parser::compound_statement>, semantic_error> resolve_compound_statement(
  const std::unique_ptr<parser::compound_statement> &node,
  variable_map &variable_map)
{
    variable_map.create_scope();
    auto block = resolve_block(node->block, variable_map);
    if (block.has_value() == false)
    {
        return std::unexpected{ block.error() };
    }

    variable_map.destroy_scope();
    return std::make_unique<parser::compound_statement>(std::move(block.value()));
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
    if (variable_map.contains(input.name, variable_map::scopes::current_scope))
    {
        auto msg = fmt::format("Duplicate definition for variable {}", input.name.name);
        return std::unexpected{ semantic_error{ msg } };
    }

    parser::identifier unique_name = variable_map.add(input.name);

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

std::expected<std::unique_ptr<parser::do_while_statement>, semantic_error> resolve_do_while_statement(
  const std::unique_ptr<parser::do_while_statement> &node,
  variable_map &variable_map)
{
    auto body = resolve_statement(node->body, variable_map);
    if (body.has_value() == false)
    {
        return std::unexpected{ body.error() };
    }

    auto cond = resolve_expression(node->condition, variable_map);
    if (cond.has_value() == false)
    {
        return std::unexpected{ cond.error() };
    }

    return std::make_unique<parser::do_while_statement>(std::move(body.value()), std::move(cond.value()), node->label);
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

std::expected<parser::for_init, semantic_error> resolve_for_init(const parser::for_init &node,
                                                                 variable_map &variable_map)
{
    return std::visit(visitor{
                        [&](const parser::init_declaration &n) -> std::expected<parser::for_init, semantic_error> {
                            return resolve_init_declaration(n, variable_map);
                        },
                        [&](const parser::init_expression &n) -> std::expected<parser::for_init, semantic_error> {
                            return resolve_init_expression(n, variable_map);
                        },
                      },
                      node);
}
std::expected<std::unique_ptr<parser::for_statement>, semantic_error> resolve_for_statement(
  const std::unique_ptr<parser::for_statement> &node,
  variable_map &variable_map)
{
    variable_map.create_scope();
    auto init = resolve_for_init(node->init, variable_map);
    if (init.has_value() == false)
    {
        return std::unexpected{ init.error() };
    }
    std::optional<parser::expression> cond;
    if (node->condition.has_value())
    {
        auto tmp = resolve_expression(node->condition.value(), variable_map);
        if (tmp.has_value() == false)
        {
            return std::unexpected{ tmp.error() };
        }
        cond = std::move(tmp.value());
    }

    std::optional<parser::expression> post;
    if (node->post.has_value())
    {
        auto tmp = resolve_expression(node->post.value(), variable_map);
        if (tmp.has_value() == false)
        {
            return std::unexpected{ tmp.error() };
        }
        post = std::move(tmp.value());
    }

    auto body = resolve_statement(node->body, variable_map);
    if (body.has_value() == false)
    {
        return std::unexpected{ body.error() };
    }

    variable_map.destroy_scope();
    return std::make_unique<parser::for_statement>(std::move(init.value()),
                                                   std::move(cond),
                                                   std::move(post),
                                                   std::move(body.value()),
                                                   node->label);
}

std::expected<parser::function, semantic_error> resolve_function(const parser::function &input,
                                                                 variable_map &variable_map)
{
    auto block = resolve_block(input.body, variable_map);
    if (block.has_value() == false)
    {
        return std::unexpected{ block.error() };
    }

    return parser::function{ input.function_name, std::move(block.value()) };
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

std::expected<parser::init_declaration, semantic_error> resolve_init_declaration(const parser::init_declaration &node,
                                                                                 variable_map &variable_map)
{
    auto tmp = resolve_declaration(node.decl, variable_map);
    if (tmp.has_value() == false)
    {
        return std::unexpected{ tmp.error() };
    }
    return parser::init_declaration{ std::move(tmp.value()) };
}
std::expected<parser::init_expression, semantic_error> resolve_init_expression(const parser::init_expression &node,
                                                                               variable_map &variable_map)
{
    if (node.expression.has_value())
    {
        auto tmp = resolve_expression(node.expression.value(), variable_map);
        if (tmp.has_value() == false)
        {
            return std::unexpected{ tmp.error() };
        }
        return parser::init_expression{ std::move(tmp.value()) };
    }
    return parser::init_expression{ std::nullopt };
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
        [&](const std::unique_ptr<parser::compound_statement> &n) -> std::expected<parser::statement, semantic_error> {
            return resolve_compound_statement(n, variable_map);
        },
        [&](const parser::break_statement &n) -> std::expected<parser::statement, semantic_error> { return n; },
        [&](const parser::continue_statement &n) -> std::expected<parser::statement, semantic_error> { return n; },
        [&](const std::unique_ptr<parser::while_statement> &n) -> std::expected<parser::statement, semantic_error> {
            return resolve_while_statement(n, variable_map);
        },
        [&](const std::unique_ptr<parser::do_while_statement> &n) -> std::expected<parser::statement, semantic_error> {
            return resolve_do_while_statement(n, variable_map);
        },
        [&](const std::unique_ptr<parser::for_statement> &n) -> std::expected<parser::statement, semantic_error> {
            return resolve_for_statement(n, variable_map);
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

std::expected<std::unique_ptr<parser::while_statement>, semantic_error> resolve_while_statement(
  const std::unique_ptr<parser::while_statement> &node,
  variable_map &variable_map)
{
    auto cond = resolve_expression(node->condition, variable_map);
    if (cond.has_value() == false)
    {
        return std::unexpected{ cond.error() };
    }
    auto body = resolve_statement(node->body, variable_map);
    if (body.has_value() == false)
    {
        return std::unexpected{ body.error() };
    }

    return std::make_unique<parser::while_statement>(std::move(cond.value()), std::move(body.value()), node->label);
}
}