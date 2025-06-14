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

#include "identifier_resolution.h"
#include <visitor.h>

namespace wccff::sema::variable_resolution {
auto process_assignment_node(const std::unique_ptr<parser::assignment_node> &node, identifier_map &variable_map)
  -> std::expected<std::unique_ptr<parser::assignment_node>, semantic_error>
{
    if (std::holds_alternative<parser::var>(node->lhs) == false)
    {
        auto msg = fmt::format("Trying to assign a value to a non variable");
        return std::unexpected{ semantic_error{ msg } };
    }

    auto left = process_expression(node->lhs, variable_map);
    if (left.has_value() == false)
    {
        return std::unexpected{ left.error() };
    }
    auto right = process_expression(node->rhs, variable_map);
    if (right.has_value() == false)
    {
        return std::unexpected{ right.error() };
    }
    return std::make_unique<parser::assignment_node>(std::move(left.value()), std::move(right.value()));
}

auto process_binary_node(const std::unique_ptr<parser::binary_node> &node, identifier_map &variable_map)
  -> std::expected<std::unique_ptr<parser::binary_node>, semantic_error>
{
    auto left = process_expression(node->left, variable_map);
    if (left.has_value() == false)
    {
        return std::unexpected{ left.error() };
    }

    auto right = process_expression(node->right, variable_map);
    if (right.has_value() == false)
    {
        return std::unexpected{ right.error() };
    }
    return std::make_unique<parser::binary_node>(node->op, std::move(left.value()), std::move(right.value()));
}

auto process_block(const parser::block &node, identifier_map &variable_map)
  -> std::expected<parser::block, semantic_error>
{
    parser::block block;
    for (const auto &b : node.items)
    {
        auto new_block = process_block_item(b, variable_map);
        if (new_block.has_value() == false)
        {
            return std::unexpected{ new_block.error() };
        }
        block.items.push_back(std::move(new_block.value()));
    }

    return block;
}

auto process_block_item(const parser::block_item &node, identifier_map &variable_map)
  -> std::expected<parser::block_item, semantic_error>
{
    return std::visit(
      visitor{
        [&variable_map](const parser::declaration &n) mutable -> std::expected<parser::block_item, semantic_error> {
            auto d = process_declaration(n, variable_map, scope_type::inner);
            if (d.has_value() == false)
            {
                return std::unexpected{ d.error() };
            }
            return parser::block_item{ std::move(d.value()) };
        },
        [&variable_map](const parser::statement &n) mutable -> std::expected<parser::block_item, semantic_error> {
            auto stmt = process_statement(n, variable_map);
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
      node);
}

auto process_cast_expression(const std::unique_ptr<parser::cast_expression> &node, identifier_map &variable_map)
  -> std::expected<std::unique_ptr<parser::cast_expression>, semantic_error>
{
    auto exp = process_expression(node->exp, variable_map);
    if (exp.has_value() == false)
    {
        return std::unexpected{ exp.error() };
    }

    return std::make_unique<parser::cast_expression>(copy_type(node->target), std::move(exp.value()));
}

auto process_compound_statement(const std::unique_ptr<parser::compound_statement> &node, identifier_map &variable_map)
  -> std::expected<std::unique_ptr<parser::compound_statement>, semantic_error>
{
    variable_map.create_scope();
    auto block = process_block(node->block, variable_map);
    if (block.has_value() == false)
    {
        return std::unexpected{ block.error() };
    }

    variable_map.destroy_scope();
    return std::make_unique<parser::compound_statement>(std::move(block.value()));
}

auto process_conditional_node(const std::unique_ptr<parser::conditional_node> &node, identifier_map &variable_map)
  -> std::expected<std::unique_ptr<parser::conditional_node>, semantic_error>
{
    auto cond = process_expression(node->condition, variable_map);
    if (cond.has_value() == false)
    {
        return std::unexpected{ cond.error() };
    }
    auto e1 = process_expression(node->e1, variable_map);
    if (e1.has_value() == false)
    {
        return std::unexpected{ e1.error() };
    }

    auto e2 = process_expression(node->e2, variable_map);
    if (e2.has_value() == false)
    {
        return std::unexpected{ e2.error() };
    }

    return std::make_unique<parser::conditional_node>(std::move(cond.value()),
                                                      std::move(e1.value()),
                                                      std::move(e2.value()));
}

auto process_declaration(const parser::declaration &node, identifier_map &variable_map, scope_type scope)
  -> std::expected<parser::declaration, semantic_error>
{
    return std::visit(
      visitor{
        [&variable_map](
          const parser::function_declaration &node) -> std::expected<parser::declaration, semantic_error> {
            return process_function_declaration(node, variable_map);
        },
        [&variable_map,
         scope](const parser::variable_declaration &node) -> std::expected<parser::declaration, semantic_error> {
            return process_variable_declaration(node, variable_map, scope);
        },
      },
      node);
}

auto process_do_while_statement(const std::unique_ptr<parser::do_while_statement> &node, identifier_map &variable_map)
  -> std::expected<std::unique_ptr<parser::do_while_statement>, semantic_error>
{
    auto body = process_statement(node->body, variable_map);
    if (body.has_value() == false)
    {
        return std::unexpected{ body.error() };
    }

    auto cond = process_expression(node->condition, variable_map);
    if (cond.has_value() == false)
    {
        return std::unexpected{ cond.error() };
    }

    return std::make_unique<parser::do_while_statement>(std::move(body.value()), std::move(cond.value()), node->label);
}

auto process_expression(const parser::expression &node, identifier_map &variable_map)
  -> std::expected<parser::expression, semantic_error>
{
    return std::visit(
      visitor{
        [&](const std::unique_ptr<parser::assignment_node> &n) -> std::expected<parser::expression, semantic_error> {
            return process_assignment_node(n, variable_map);
        },
        [&](const std::unique_ptr<parser::cast_expression> &n) -> std::expected<parser::expression, semantic_error> {
            return process_cast_expression(n, variable_map);
        },
        [&](const std::unique_ptr<parser::conditional_node> &n) -> std::expected<parser::expression, semantic_error> {
            return process_conditional_node(n, variable_map);
        },
        [&](const std::unique_ptr<parser::binary_node> &n) -> std::expected<parser::expression, semantic_error> {
            return process_binary_node(n, variable_map);
        },
        [&](const std::unique_ptr<parser::function_call> &n) -> std::expected<parser::expression, semantic_error> {
            return process_function_call(n, variable_map);
        },
        [&](const std::unique_ptr<parser::unary_node> &n) -> std::expected<parser::expression, semantic_error> {
            return process_unary_node(n, variable_map);
        },
        [&](const parser::var &n) -> std::expected<parser::expression, semantic_error> {
            return process_var(n, variable_map);
        },
        [&](const constant &n) -> std::expected<parser::expression, semantic_error> { return n; },
      },
      node);
}

auto process_for_init(const parser::for_init &node, identifier_map &variable_map)
  -> std::expected<parser::for_init, semantic_error>
{
    return std::visit(visitor{
                        [&](const parser::init_declaration &n) -> std::expected<parser::for_init, semantic_error> {
                            return process_init_declaration(n, variable_map);
                        },
                        [&](const parser::init_expression &n) -> std::expected<parser::for_init, semantic_error> {
                            return process_init_expression(n, variable_map);
                        },
                      },
                      node);
}

auto process_for_statement(const std::unique_ptr<parser::for_statement> &node, identifier_map &variable_map)
  -> std::expected<std::unique_ptr<parser::for_statement>, semantic_error>
{
    variable_map.create_scope();
    auto init = process_for_init(node->init, variable_map);
    if (init.has_value() == false)
    {
        return std::unexpected{ init.error() };
    }
    std::optional<parser::expression> cond;
    if (node->condition.has_value())
    {
        auto tmp = process_expression(node->condition.value(), variable_map);
        if (tmp.has_value() == false)
        {
            return std::unexpected{ tmp.error() };
        }
        cond = std::move(tmp.value());
    }

    std::optional<parser::expression> post;
    if (node->post.has_value())
    {
        auto tmp = process_expression(node->post.value(), variable_map);
        if (tmp.has_value() == false)
        {
            return std::unexpected{ tmp.error() };
        }
        post = std::move(tmp.value());
    }

    auto body = process_statement(node->body, variable_map);
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

auto process_function_call(const std::unique_ptr<parser::function_call> &node, identifier_map &map)
  -> std::expected<std::unique_ptr<parser::function_call>, semantic_error>
{
    auto s = map.find(node->name);
    if (s.has_value() == false)
    {
        auto msg = fmt::format("Function call '{}' not defined", node->name.name);
        return std::unexpected{ semantic_error{ msg } };
    }

    std::vector<parser::expression> args;
    for (const auto &e : node->arguments)
    {
        auto result = process_expression(e, map);
        if (result.has_value() == false)
        {
            return std::unexpected{ result.error() };
        }
        args.push_back(std::move(result.value()));
    }

    return std::make_unique<parser::function_call>(s->unique_name, std::move(args));
}

auto process_function_declaration(const parser::function_declaration &node, identifier_map &variable_map)
  -> std::expected<parser::function_declaration, semantic_error>
{
    if (auto s = variable_map.find(node.name, identifier_map::scopes::current_scope);
        s.has_value() && s->linkage == identifier_map::linkage::internal)
    {
        auto msg = fmt::format("Duplicate Function declaration '{}'", node.name.name);
        return std::unexpected{ semantic_error{ msg } };
    }

    variable_map.add(node.name, identifier_map::linkage::external);

    variable_map.create_scope();

    std::vector<parser::identifier> args;
    for (const auto &a : node.arguments)
    {
        if (variable_map.find(a, identifier_map::scopes::current_scope).has_value())
        {
            auto msg = fmt::format("Duplicate identifier '{}'", a.name);
            return std::unexpected{ semantic_error{ msg } };
        }
        args.push_back(variable_map.add(a));
    }

    std::optional<parser::block> block;
    if (node.body.has_value())
    {
        auto tmp = process_block(node.body.value(), variable_map);
        if (tmp.has_value() == false)
        {
            return std::unexpected{ tmp.error() };
        }
        block = std::move(tmp.value());
    }

    variable_map.destroy_scope();
    return parser::function_declaration{ node.name,
                                         std::move(args),
                                         std::move(block),
                                         copy_type(node.f_type),
                                         node.storage_class };
}

auto process_if_node(const std::unique_ptr<parser::if_node> &node, identifier_map &variable_map)
  -> std::expected<std::unique_ptr<parser::if_node>, semantic_error>
{
    auto op = process_expression(node->op, variable_map);
    if (op.has_value() == false)
    {
        return std::unexpected{ op.error() };
    }
    auto then_stmt = process_statement(node->then_stmt, variable_map);
    if (then_stmt.has_value() == false)
    {
        return std::unexpected{ then_stmt.error() };
    }
    if (node->else_stmt.has_value())
    {
        auto else_stmt = process_statement(node->else_stmt.value(), variable_map);
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

auto process_init_declaration(const parser::init_declaration &node, identifier_map &variable_map)
  -> std::expected<parser::init_declaration, semantic_error>
{
    auto tmp = process_variable_declaration(node.decl, variable_map, scope_type::inner);
    if (tmp.has_value() == false)
    {
        return std::unexpected{ tmp.error() };
    }
    return parser::init_declaration{ std::move(tmp.value()) };
}

auto process_init_expression(const parser::init_expression &node, identifier_map &variable_map)
  -> std::expected<parser::init_expression, semantic_error>
{
    if (node.expression.has_value())
    {
        auto tmp = process_expression(node.expression.value(), variable_map);
        if (tmp.has_value() == false)
        {
            return std::unexpected{ tmp.error() };
        }
        return parser::init_expression{ std::move(tmp.value()) };
    }
    return parser::init_expression{ std::nullopt };
}

auto process_labelled_statement(const std::unique_ptr<parser::labelled_statement> &node, identifier_map &variable_map)
  -> std::expected<std::unique_ptr<parser::labelled_statement>, semantic_error>
{
    auto body = process_statement(node->body, variable_map);
    if (body.has_value() == false)
    {
        return std::unexpected{ body.error() };
    }

    return std::make_unique<parser::labelled_statement>(node->label, std::move(body.value()));
}

auto process_program(const parser::program &node, identifier_map &variable_map)
  -> std::expected<parser::program, semantic_error>
{
    std::vector<parser::declaration> functions;
    for (const auto &f : node.f)
    {
        auto tmp = process_declaration(f, variable_map, scope_type::file);
        if (tmp.has_value() == false)
        {
            return std::unexpected{ tmp.error() };
        }
        functions.push_back(std::move(tmp.value()));
    }

    return parser::program{ std::move(functions) };
}

auto process_return_node(const parser::return_node &node, identifier_map &variable_map)
  -> std::expected<parser::return_node, semantic_error>
{
    auto e = process_expression(node.e, variable_map);
    if (e.has_value() == false)
    {
        return std::unexpected{ e.error() };
    }
    return parser::return_node{ std::move(e.value()) };
}

auto process_statement(const parser::statement &node, identifier_map &variable_map)
  -> std::expected<parser::statement, semantic_error>
{
    return std::visit(
      visitor{
        [&](const parser::return_node &n) -> std::expected<parser::statement, semantic_error> {
            return process_return_node(n, variable_map);
        },
        [&](const parser::expression &n) -> std::expected<parser::statement, semantic_error> {
            return process_expression(n, variable_map);
        },
        [&](const std::unique_ptr<parser::if_node> &n) -> std::expected<parser::statement, semantic_error> {
            return process_if_node(n, variable_map);
        },
        [&](const std::unique_ptr<parser::compound_statement> &n) -> std::expected<parser::statement, semantic_error> {
            return process_compound_statement(n, variable_map);
        },
        [&](const parser::break_statement &n) -> std::expected<parser::statement, semantic_error> { return n; },
        [&](const parser::continue_statement &n) -> std::expected<parser::statement, semantic_error> { return n; },
        [&](const parser::goto_statement &n) -> std::expected<parser::statement, semantic_error> { return n; },
        [&](const std::unique_ptr<parser::while_statement> &n) -> std::expected<parser::statement, semantic_error> {
            return process_while_statement(n, variable_map);
        },
        [&](const std::unique_ptr<parser::do_while_statement> &n) -> std::expected<parser::statement, semantic_error> {
            return process_do_while_statement(n, variable_map);
        },
        [&](const std::unique_ptr<parser::for_statement> &n) -> std::expected<parser::statement, semantic_error> {
            return process_for_statement(n, variable_map);
        },
        [&](const std::monostate &n) -> std::expected<parser::statement, semantic_error> { return std::monostate{}; },
        [&](const std::unique_ptr<parser::labelled_statement> &n) -> std::expected<parser::statement, semantic_error> {
            return process_labelled_statement(n, variable_map);
        },
      },
      node);
}

auto process_unary_node(const std::unique_ptr<parser::unary_node> &node, identifier_map &variable_map)
  -> std::expected<std::unique_ptr<parser::unary_node>, semantic_error>
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

    auto exp = process_expression(node->exp, variable_map);
    if (exp.has_value() == false)
    {
        return std::unexpected{ exp.error() };
    }

    return std::make_unique<parser::unary_node>(node->op, std::move(exp.value()));
}

auto process_var(const parser::var &node, identifier_map &variable_map) -> std::expected<parser::var, semantic_error>
{
    auto s = variable_map.find(node.name);
    if (s.has_value() == false)
    {
        return std::unexpected<semantic_error>{ fmt::format("Variable {} not defined", node.name.name) };
    }

    return parser::var{ s->unique_name };
}

auto process_variable_declaration(const parser::variable_declaration &node,
                                  identifier_map &variable_map,
                                  scope_type scope) -> std::expected<parser::variable_declaration, semantic_error>
{
    auto copy_init = [](const std::optional<parser::expression> &init) {
        return init.has_value() ? std::make_optional(parser::copy_expression(init.value())) : std::nullopt;
    };

    if (scope == scope_type::file)
    {
        variable_map.add(node.name, identifier_map::linkage::external);
        return parser::variable_declaration{ node.name,
                                             copy_init(node.init),
                                             copy_type(node.var_type),
                                             node.storage_class };
    }
    else
    {
        auto s = variable_map.find(node.name, identifier_map::scopes::current_scope);
        if (s.has_value())
        {
            if ((s.value().linkage == identifier_map::linkage::external &&
                 node.storage_class == parser::storage_class::extern_storage) == false)
            {
                auto msg = fmt::format("Conflicting local declarations for variable {}", node.name.name);
                return std::unexpected{ semantic_error{ msg } };
            }
        }

        // parser::identifier unique_name;
        if (node.storage_class == parser::storage_class::extern_storage)
        {
            variable_map.add(node.name, identifier_map::linkage::external);
            return parser::variable_declaration{ node.name,
                                                 copy_init(node.init),
                                                 copy_type(node.var_type),
                                                 node.storage_class };
        }

        parser::identifier unique_name = variable_map.add(node.name);
        std::optional<parser::expression> init;
        if (node.init.has_value())
        {
            auto a = process_expression(node.init.value(), variable_map);
            if (a.has_value() == false)
            {
                return std::unexpected{ a.error() };
            }
            init = std::move(a.value());
        }

        return parser::variable_declaration{ unique_name,
                                             std::move(init),
                                             copy_type(node.var_type),
                                             node.storage_class };
    }
}

auto process_while_statement(const std::unique_ptr<parser::while_statement> &node, identifier_map &variable_map)
  -> std::expected<std::unique_ptr<parser::while_statement>, semantic_error>
{
    auto cond = process_expression(node->condition, variable_map);
    if (cond.has_value() == false)
    {
        return std::unexpected{ cond.error() };
    }
    auto body = process_statement(node->body, variable_map);
    if (body.has_value() == false)
    {
        return std::unexpected{ body.error() };
    }

    return std::make_unique<parser::while_statement>(std::move(cond.value()), std::move(body.value()), node->label);
}

} // namespace wccff::sema::variable_resolution
