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

#include "loop_labelling.h"

#include <visitor.h>

namespace wccff::sema::loop_labelling {

static parser::identifier generate_loop_label()
{
    static int32_t counter = 0;
    return parser::identifier(fmt::format("loop_{}", counter++));
}

auto process_block(const parser::block &node, const std::optional<parser::identifier> &label)
  -> std::expected<parser::block, semantic_error>
{
    std::vector<parser::block_item> items;
    for (const auto &i : node.items)
    {
        auto result = process_block_item(i, label);
        if (result.has_value() == false)
        {
            return std::unexpected{ result.error() };
        }
        items.emplace_back(std::move(result.value()));
    }

    return parser::block{ std::move(items) };
}

auto process_block_item(const parser::block_item &node, const std::optional<parser::identifier> &label)
  -> std::expected<parser::block_item, semantic_error>
{
    return std::visit(visitor{
                        [label](const parser::declaration &n) -> std::expected<parser::block_item, semantic_error> {
                            return process_declaration(n);
                        },
                        [label](const parser::statement &n) -> std::expected<parser::block_item, semantic_error> {
                            return process_statement(n, label);
                        },
                        [](const std::monostate &) -> std::expected<parser::block_item, semantic_error> {
                            return parser::block_item{ std::monostate{} };
                        },
                      },
                      node);
}

auto process_break_statement(const std::optional<parser::identifier> &label)
  -> std::expected<parser::break_statement, semantic_error>
{
    if (label.has_value() == false)
    {
        return std::unexpected{ semantic_error{ fmt::format("Break statement outside a loop") } };
    }
    return parser::break_statement{ label.value() };
}

auto process_compound_statement(const std::unique_ptr<parser::compound_statement> &node,
                                const std::optional<parser::identifier> &label)
  -> std::expected<std::unique_ptr<parser::compound_statement>, semantic_error>
{
    auto tmp = process_block(node->block, label);
    if (tmp.has_value() == false)
    {
        return std::unexpected{ tmp.error() };
    }
    return std::make_unique<parser::compound_statement>(std::move(tmp.value()));
}

auto process_continue_statement(const std::optional<parser::identifier> &label)
  -> std::expected<parser::continue_statement, semantic_error>
{
    if (label.has_value() == false)
    {
        return std::unexpected{ semantic_error{ fmt::format("Continue statement outside a loop") } };
    }
    return parser::continue_statement{ label.value() };
}

auto process_declaration(const parser::declaration &node) -> std::expected<parser::declaration, semantic_error>
{
    return std::visit(
      visitor{
        [](const parser::function_declaration &n) -> std::expected<parser::declaration, semantic_error> {
            return process_function_declaration(n);
        },
        [](const parser::variable_declaration &n) -> std::expected<parser::declaration, semantic_error> {
            return parser::copy_declaration(n);
        },
      },
      node);
}

auto process_do_while_statement(const std::unique_ptr<parser::do_while_statement> &node,
                                const std::optional<parser::identifier> &label)
  -> std::expected<std::unique_ptr<parser::do_while_statement>, semantic_error>
{
    parser::identifier new_label = generate_loop_label();
    auto body = process_statement(node->body, new_label);
    if (body.has_value() == false)
    {
        return std::unexpected{ body.error() };
    }
    auto condition = parser::copy_expression(node->condition);

    return std::make_unique<parser::do_while_statement>(std::move(body.value()), std::move(condition), new_label);
}

auto process_for_statement(const std::unique_ptr<parser::for_statement> &node,
                           const std::optional<parser::identifier> &label)
  -> std::expected<std::unique_ptr<parser::for_statement>, semantic_error>
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

    auto body = process_statement(node->body, new_label);

    return std::make_unique<parser::for_statement>(std::move(init),
                                                   std::move(cond),
                                                   std::move(post),
                                                   std::move(body.value()),
                                                   new_label);
}

auto process_function_declaration(const parser::function_declaration &node)
  -> std::expected<parser::function_declaration, semantic_error>
{
    std::optional<parser::block> block;
    if (node.body.has_value())
    {
        auto tmp = process_block(node.body.value(), std::nullopt);
        if (tmp.has_value() == false)
        {
            return std::unexpected{ tmp.error() };
        }
        block = std::move(tmp.value());
    }

    return parser::function_declaration{ node.name, node.arguments, std::move(block) };
}

auto process_if_node(const std::unique_ptr<parser::if_node> &node, const std::optional<parser::identifier> &label)
  -> std::expected<std::unique_ptr<parser::if_node>, semantic_error>
{
    auto then_stmt = process_statement(node->then_stmt, label);
    if (then_stmt.has_value() == false)
    {
        return std::unexpected{ then_stmt.error() };
    }

    std::optional<parser::statement> else_stmt;
    if (node->else_stmt.has_value())
    {
        auto tmp = process_statement(node->else_stmt.value(), label);
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
auto process_labelled_statement(const std::unique_ptr<parser::labelled_statement> &node,
                                const std::optional<parser::identifier> &label)
  -> std::expected<std::unique_ptr<parser::labelled_statement>, semantic_error>
{
    auto body = process_statement(node->body, label);
    if (body.has_value() == false)
    {
        return std::unexpected{ body.error() };
    }

    return std::make_unique<parser::labelled_statement>(node->label, std::move(body.value()));
}

auto process_program(const parser::program &node) -> std::expected<parser::program, semantic_error>
{
    std::vector<parser::declaration> functions;
    for (const auto &f : node.f)
    {
        auto tmp = process_declaration(f);
        if (tmp.has_value() == false)
        {
            return std::unexpected{ tmp.error() };
        }
        functions.push_back(std::move(tmp.value()));
    }

    return parser::program{ std::move(functions) };
}

auto process_statement(const parser::statement &node, const std::optional<parser::identifier> &label)
  -> std::expected<parser::statement, semantic_error>
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
            return process_if_node(n, label);
        },
        [label](const std::unique_ptr<parser::compound_statement> &n)
          -> std::expected<parser::statement, semantic_error> { return process_compound_statement(n, label); },
        [label](const parser::break_statement &) -> std::expected<parser::statement, semantic_error> {
            return process_break_statement(label);
        },
        [label](const parser::continue_statement &) -> std::expected<parser::statement, semantic_error> {
            return process_continue_statement(label);
        },
        [](const parser::goto_statement &n) -> std::expected<parser::statement, semantic_error> { return n; },

        [label](const std::unique_ptr<parser::while_statement> &n) -> std::expected<parser::statement, semantic_error> {
            return process_while_statement(n, label);
        },
        [label](const std::unique_ptr<parser::do_while_statement> &n)
          -> std::expected<parser::statement, semantic_error> { return process_do_while_statement(n, label); },
        [label](const std::unique_ptr<parser::for_statement> &n) -> std::expected<parser::statement, semantic_error> {
            return process_for_statement(n, label);
        },
        [](const std::monostate &) -> std::expected<parser::statement, semantic_error> { return std::monostate{}; },
        [label](const std::unique_ptr<parser::labelled_statement> &n)
          -> std::expected<parser::statement, semantic_error> { return process_labelled_statement(n, label); },

      },
      node);
}

auto process_while_statement(const std::unique_ptr<parser::while_statement> &node,
                             const std::optional<parser::identifier> &label)
  -> std::expected<std::unique_ptr<parser::while_statement>, semantic_error>
{
    parser::identifier new_label = generate_loop_label();
    auto condition = parser::copy_expression(node->condition);

    auto body = process_statement(node->body, new_label);
    if (body.has_value() == false)
    {
        return std::unexpected{ body.error() };
    }

    return std::make_unique<parser::while_statement>(std::move(condition), std::move(body.value()), new_label);
}

} // namespace wccff::sema::loop_labelling
