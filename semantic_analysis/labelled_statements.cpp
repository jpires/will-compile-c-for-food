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

#include "labelled_statements.h"
#include <visitor.h>

namespace wccff::sema::labelled_statements {

auto process_block(const parser::block &node, labelled_statement_map &map)
  -> std::expected<parser::block, semantic_error>
{
    std::vector<parser::block_item> items;

    for (const auto &i : node.items)
    {
        auto tmp = process_block_item(i, map);
        if (tmp.has_value() == false)
        {
            return std::unexpected(tmp.error());
        }
        items.push_back(std::move(tmp.value()));
    }

    return parser::block{ std::move(items) };
}

auto process_block_item(const parser::block_item &node, labelled_statement_map &map)
  -> std::expected<parser::block_item, semantic_error>
{
    return std::visit(visitor{
                        [&map](const parser::declaration &n) -> std::expected<parser::block_item, semantic_error> {
                            return process_declaration(n);
                        },
                        [&map](const parser::statement &n) -> std::expected<parser::block_item, semantic_error> {
                            return process_statement(n, map);
                        },
                        [&map](const std::monostate &n) -> std::expected<parser::block_item, semantic_error> {
                            return std::monostate{};
                        },
                      },
                      node);
}

auto process_compound_statement(const std::unique_ptr<parser::compound_statement> &node, labelled_statement_map &map)
  -> std::expected<std::unique_ptr<parser::compound_statement>, semantic_error>
{
    auto tmp = process_block(node->block, map);
    if (tmp.has_value() == false)
    {
        return std::unexpected{ tmp.error() };
    }
    return std::make_unique<parser::compound_statement>(std::move(tmp.value()));
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

auto process_do_while_statement(const std::unique_ptr<parser::do_while_statement> &node, labelled_statement_map &map)
  -> std::expected<std::unique_ptr<parser::do_while_statement>, semantic_error>
{
    auto body = process_statement(node->body, map);
    if (body.has_value() == false)
    {
        return std::unexpected{ body.error() };
    }
    auto condition = parser::copy_expression(node->condition);

    return std::make_unique<parser::do_while_statement>(std::move(body.value()), std::move(condition), node->label);
}

auto process_for_statement(const std::unique_ptr<parser::for_statement> &node, labelled_statement_map &map)
  -> std::expected<std::unique_ptr<parser::for_statement>, semantic_error>
{
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

    auto body = process_statement(node->body, map);

    return std::make_unique<parser::for_statement>(std::move(init),
                                                   std::move(cond),
                                                   std::move(post),
                                                   std::move(body.value()),
                                                   node->label);
}

auto process_goto_statement(const parser::goto_statement &node, labelled_statement_map &map)
  -> std::expected<parser::goto_statement, semantic_error>
{
    auto new_label = map.add_goto(node.label);

    return parser::goto_statement{ new_label };
}

auto process_function(const parser::function &node) -> std::expected<parser::function, semantic_error>
{
    labelled_statement_map map(node.function_name);

    auto result = process_block(node.body, map);
    if (result.has_value() == false)
    {
        return std::unexpected{ result.error() };
    }

    if (map.validate() == false)
    {
        auto msg = fmt::format("Missing target labelled statement in {}", node.function_name.name);
        return std::unexpected{ semantic_error{ msg } };
    }

    return parser::function{ node.function_name, std::move(result.value()) };
}

auto process_function_declaration(const parser::function_declaration &node)
  -> std::expected<parser::function_declaration, semantic_error>
{
    labelled_statement_map map(node.name);

    std::optional<parser::block> body;
    if (node.body.has_value())
    {
        auto result = process_block(node.body.value(), map);
        if (result.has_value() == false)
        {
            return std::unexpected{ result.error() };
        }
        body = std::move(result.value());
    }

    if (map.validate() == false)
    {
        auto msg = fmt::format("Missing target labelled statement in {}", node.name.name);
        return std::unexpected{ semantic_error{ msg } };
    }

    return parser::function_declaration{ node.name, node.arguments, std::move(body) };
}

auto process_if_node(const std::unique_ptr<parser::if_node> &node, labelled_statement_map &map)
  -> std::expected<std::unique_ptr<parser::if_node>, semantic_error>
{
    auto op = parser::copy_expression(node->op);
    auto then_stmt = process_statement(node->then_stmt, map);
    if (then_stmt.has_value() == false)
    {
        return std::unexpected{ then_stmt.error() };
    }
    std::optional<parser::statement> else_stmt;
    if (node->else_stmt.has_value())
    {
        auto tmp = process_statement(node->else_stmt.value(), map);
        if (tmp.has_value() == false)
        {
            return std::unexpected{ tmp.error() };
        }
        else_stmt = std::move(tmp.value());
    }

    return std::make_unique<parser::if_node>(std::move(op), std::move(then_stmt.value()), std::move(else_stmt));
}

auto process_labelled_statement(const std::unique_ptr<parser::labelled_statement> &node, labelled_statement_map &map)
  -> std::expected<std::unique_ptr<parser::labelled_statement>, semantic_error>
{
    auto new_label = map.add_label(node->label);
    if (new_label.has_value() == false)
    {
        return std::unexpected{ new_label.error() };
    }

    auto body = process_statement(node->body, map);
    if (body.has_value() == false)
    {
        return std::unexpected{ body.error() };
    }

    return std::make_unique<parser::labelled_statement>(std::move(new_label.value()), std::move(body.value()));
}

auto process_program(const parser::program &node) -> std::expected<parser::program, semantic_error>
{
    std::vector<parser::function_declaration> functions;
    for (const auto &f : node.f)
    {
        auto func = process_function_declaration(f);
        if (func.has_value() == false)
        {
            return std::unexpected{ func.error() };
        }
        functions.push_back(std::move(func.value()));
    }

    return parser::program{ std::move(functions) };
};

auto process_statement(const parser::statement &node, labelled_statement_map &map)
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
        [&map](const std::unique_ptr<parser::if_node> &n) -> std::expected<parser::statement, semantic_error> {
            return process_if_node(n, map);
        },
        [&map](const std::unique_ptr<parser::compound_statement> &n)
          -> std::expected<parser::statement, semantic_error> { return process_compound_statement(n, map); },
        [](const parser::break_statement &n) -> std::expected<parser::statement, semantic_error> {
            return parser::break_statement{ n.label };
        },
        [](const parser::continue_statement &n) -> std::expected<parser::statement, semantic_error> {
            return parser::continue_statement{ n.label };
        },
        [&map](const parser::goto_statement &n) -> std::expected<parser::statement, semantic_error> {
            return process_goto_statement(n, map);
        },
        [&map](const std::unique_ptr<parser::while_statement> &n) -> std::expected<parser::statement, semantic_error> {
            return process_while_statement(n, map);
        },
        [&map](const std::unique_ptr<parser::do_while_statement> &n)
          -> std::expected<parser::statement, semantic_error> { return process_do_while_statement(n, map); },
        [&map](const std::unique_ptr<parser::for_statement> &n) -> std::expected<parser::statement, semantic_error> {
            return process_for_statement(n, map);
        },
        [](const std::monostate &) -> std::expected<parser::statement, semantic_error> { return std::monostate{}; },
        [&map](const std::unique_ptr<parser::labelled_statement> &n)
          -> std::expected<parser::statement, semantic_error> { return process_labelled_statement(n, map); },

      },
      node);
}

auto process_while_statement(const std::unique_ptr<parser::while_statement> &node, labelled_statement_map &map)
  -> std::expected<std::unique_ptr<parser::while_statement>, semantic_error>
{
    auto condition = parser::copy_expression(node->condition);
    auto body = process_statement(node->body, map);
    if (body.has_value() == false)
    {
        return std::unexpected{ body.error() };
    }

    return std::make_unique<parser::while_statement>(std::move(condition), std::move(body.value()), node->label);
}

} // namespace wccff::sema::labelled_statements
