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
#include "type_checker.h"

#include "assembly_generation.h"
#include "identifier_resolution.h"
#include "labelled_statements.h"
#include "tacky.h"
#include <ranges>
#include <visitor.h>

namespace wccff::sema::type_checker {

auto convert_constant(const constant &c, const type &t) -> symbol_table::initial_value
{
    auto get_int_value = [](const constant &con) {
        return std::visit(visitor{ [](const auto &n) { return static_cast<int64_t>(n.value); } }, con);
    };

    auto get_uint_value = [](const constant &con) {
        return std::visit(visitor{ [](const auto &n) { return static_cast<uint64_t>(n.value); } }, con);
    };

    auto get_value_double = [](const constant &con) {
        return std::visit(visitor{ [](const auto &n) { return static_cast<double>(n.value); } }, con);
    };

    auto value = get_int_value(c);
    auto uint_value = get_uint_value(c);
    auto double_value = get_value_double(c);
    return std::visit(
      visitor{
        [double_value](const double_type &) -> symbol_table::initial_value { return double_initial{ double_value }; },
        [value](const int_type &) -> symbol_table::initial_value { return int_initial{ static_cast<int32_t>(value) }; },
        [value](const long_type &) -> symbol_table::initial_value { return long_initial{ value }; },
        [uint_value](const unsigned_int_type &) -> symbol_table::initial_value {
            return unsigned_int_initial{ static_cast<uint32_t>(uint_value) };
        },
        [uint_value](const unsigned_long_type &) -> symbol_table::initial_value {
            return unsigned_long_initial{ static_cast<uint64_t>(uint_value) };
        },
        [uint_value](const std::unique_ptr<pointer> &) -> symbol_table::initial_value {
            return unsigned_long_initial{ static_cast<uint64_t>(uint_value) };
        },
        [](const auto &) -> symbol_table::initial_value { throw std::logic_error("Undefined operator"); },
      },
      t);
}

auto process_address_of(const std::unique_ptr<parser::address_of> &node, symbol_table::symbol_table &table)
  -> std::expected<std::unique_ptr<parser::address_of>, semantic_error>
{
    if (is_lvalue(node->exp) == false)
    {
        auto msg = fmt::format("Can't take the address of a non-lvalue");
        return std::unexpected{ semantic_error{ msg } };
    }

    auto p_exp = process_expression(node->exp, table);
    if (p_exp.has_value() == false)
    {
        return std::unexpected{ p_exp.error() };
    }

    auto exp = std::get<parser::expression>(std::move(p_exp.value()));
    return std::make_unique<parser::address_of>(std::move(exp), std::make_unique<pointer>(get_type(exp)));
}

auto process_assignment_node(const std::unique_ptr<parser::assignment_node> &node, symbol_table::symbol_table &table)
  -> std::expected<std::variant<parser::expression, std::unique_ptr<parser::compound_statement>>, semantic_error>
{
    if (is_lvalue(node->lhs) == false)
    {
        auto msg = fmt::format("Left side of assignment is not an lvalue");
        return std::unexpected{ semantic_error{ msg } };
    }

    auto p_left = process_expression(node->lhs, table);
    if (p_left.has_value() == false)
    {
        return std::unexpected{ p_left.error() };
    }
    auto left = std::get<parser::expression>(std::move(p_left.value()));
    auto left_type = get_type(left);

    auto p_right = process_expression(node->rhs, table);
    if (p_right.has_value() == false)
    {
        return std::unexpected{ p_right.error() };
    }
    auto right = std::get<parser::expression>(std::move(p_right.value()));

    if (is_compound_operation(node->op))
    {
        if (std::holds_alternative<parser::var>(node->lhs))
        {
            // The left side is a variable, this means we can just reuse the left side
            // As it will not create side effects
            auto bin = std::make_unique<parser::binary_node>(to_binary_operator(node->op),
                                                             parser::copy_expression(left),
                                                             std::move(right));

            auto bin_typed = process_binary_node(bin, table);
            if (bin_typed.has_value() == false)
            {
                return std::unexpected{ bin_typed.error() };
            }

            auto converted_right = convert_by_assignment(parser::expression{ std::move(bin_typed.value()) }, left_type);
            if (converted_right.has_value() == false)
            {
                return std::unexpected{ converted_right.error() };
            }

            return std::make_unique<parser::assignment_node>(assign_operator{},
                                                             std::move(left),
                                                             std::move(converted_right.value()),
                                                             std::move(left_type));
        }
        else
        {
            // The left side is not a variable, so it's an arbitrary expression that can only be evaluated once.
            // So, we  create a temporary variable, of type pointer to the type of the left side.
            // The temporary variable is initialised with the address of result of the evaluation of the left side
            // Then, the compound assignment is split into two operations
            // A binary_node, that on the left side dereferences the temporary variable, and on the right side,
            // is the right of the original node.
            // and finally, an assignment_node that on the left side, dereferences the temporary variable and on
            // the right side, the previous created binary_node
            parser::block new_block;
            auto tmp_name = table.get_temporary_name();
            auto var_tmp = parser::variable_declaration(
              tmp_name,
              std::make_unique<parser::address_of>(parser::copy_expression(left)),
              std::make_unique<pointer>(copy_type(left_type)),
              parser::storage_class::no_storage);

            auto var_tmp_typed = process_variable_declaration_local_scope(var_tmp, table);
            if (var_tmp_typed.has_value() == false)
            {
                return std::unexpected{ var_tmp_typed.error() };
            }

            new_block.items.push_back(
              { parser::block_item{ parser::declaration{ std::move(var_tmp_typed.value()) } } });

            auto bin = std::make_unique<parser::binary_node>(
              to_binary_operator(node->op),
              std::make_unique<parser::dereference>(parser::var{ tmp_name }),
              std::move(right));

            auto bin_typed = process_binary_node(bin, table);
            if (bin_typed.has_value() == false)
            {
                return std::unexpected{ bin_typed.error() };
            }

            auto converted_right = convert_by_assignment(parser::expression(std::move(bin_typed.value())), left_type);
            if (converted_right.has_value() == false)
            {
                return std::unexpected{ converted_right.error() };
            }
            auto final_assign = std::make_unique<parser::assignment_node>(
              assign_operator{},
              std::make_unique<parser::dereference>(parser::var{ tmp_name }),
              std::move(converted_right.value()),
              std::move(left_type));

            new_block.items.push_back({ parser::block_item{ parser::statement{ std::move(final_assign) } } });

            return std::make_unique<parser::compound_statement>(std::move(new_block));

            // throw std::logic_error("NOT IMPLEMENTED");
        }
    }
    else
    {
        auto converted_right = convert_by_assignment(right, left_type);
        if (converted_right.has_value() == false)
        {
            return std::unexpected{ converted_right.error() };
        }

        return std::make_unique<parser::assignment_node>(node->op,
                                                         std::move(left),
                                                         std::move(converted_right.value()),
                                                         std::move(left_type));
    }
}

auto process_binary_node(const std::unique_ptr<parser::binary_node> &node, symbol_table::symbol_table &table)
  -> std::expected<std::unique_ptr<parser::binary_node>, semantic_error>
{
    auto p_left = process_expression(node->left, table);
    if (p_left.has_value() == false)
    {
        return std::unexpected{ p_left.error() };
    }
    auto left = std::get<parser::expression>(std::move(p_left.value()));

    auto p_right = process_expression(node->right, table);
    if (p_right.has_value() == false)
    {
        return std::unexpected{ p_right.error() };
    }
    auto right = std::get<parser::expression>(std::move(p_right.value()));

    if (std::holds_alternative<logical_and_operator>(node->op) || std::holds_alternative<logical_or_operator>(node->op))
    {
        return std::make_unique<parser::binary_node>(node->op,
                                                     parser::copy_expression(left),
                                                     parser::copy_expression(right),
                                                     int_type{});
    }

    if (get_type(left) == double_type{} || get_type(right) == double_type{})
    {
        // 6.5.5:2 Multiplicative operators
        // 6.5.7:2 Bitwise shift operators
        // 6.5.10:2 Bitwise AND operator
        // 6.5.11:2 Bitwise exclusive OR operator
        // 6.5.12:2 Bitwise inclusive OR operator
        if (std::holds_alternative<remainder_operator>(node->op) ||
            // std::holds_alternative<compound_remainder_operator>(node->op) ||
            std::holds_alternative<left_shift_operator>(node->op) ||
            // std::holds_alternative<compound_left_shift_operator>(node->op) ||
            std::holds_alternative<right_shift_operator>(node->op) ||
            // std::holds_alternative<compound_right_shift_operator>(node->op) ||
            std::holds_alternative<bitwise_and_operator>(node->op) ||
            // std::holds_alternative<compound_bitwise_and_operator>(node->op) ||
            std::holds_alternative<bitwise_xor_operator>(node->op) ||
            // std::holds_alternative<compound_bitwise_xor_operator>(node->op) ||
            std::holds_alternative<bitwise_or_operator>(node->op))
        // std::holds_alternative<compound_bitwise_or_operator>(node->op))
        {
            auto msg = fmt::format("Operation '{}' cannot be applied to a double", pretty_print(node->op));
            return std::unexpected{ semantic_error{ msg } };
        }
    }

    if (is_pointer(get_type(left)) || is_pointer(get_type(right)))
    {
        if (std::holds_alternative<plus_operator>(node->op) || std::holds_alternative<subtract_operator>(node->op) ||
            std::holds_alternative<multiply_operator>(node->op) || std::holds_alternative<divide_operator>(node->op) ||
            std::holds_alternative<remainder_operator>(node->op) ||
            std::holds_alternative<bitwise_and_operator>(node->op) ||
            std::holds_alternative<bitwise_or_operator>(node->op) ||
            std::holds_alternative<bitwise_xor_operator>(node->op))
        {
            auto msg = fmt::format("Operation '{}' cannot be applied to a pointer type", pretty_print(node->op));
            return std::unexpected{ semantic_error{ msg } };
        }
    }

    type common_type;
    if (is_pointer(get_type(left)) || is_pointer(get_type(right)))
    {
        auto ptr_type = wccff::sema::get_common_pointer_type(left, right);
        if (ptr_type.has_value() == false)
        {
            return std::unexpected(ptr_type.error());
        }
        common_type = std::move(ptr_type.value());
    }
    else
    {
        common_type = wccff::get_common_type(get_type(left), get_type(right));
    }
    auto converted_left = convert_to(left, common_type);
    auto converted_right = convert_to(right, common_type);

    if (std::holds_alternative<plus_operator>(node->op) || std::holds_alternative<subtract_operator>(node->op) ||
        std::holds_alternative<multiply_operator>(node->op) || std::holds_alternative<divide_operator>(node->op) ||
        std::holds_alternative<remainder_operator>(node->op) ||
        std::holds_alternative<bitwise_and_operator>(node->op) ||
        std::holds_alternative<bitwise_or_operator>(node->op) || std::holds_alternative<bitwise_xor_operator>(node->op))
    {
        return std::make_unique<parser::binary_node>(node->op,
                                                     std::move(converted_left),
                                                     std::move(converted_right),
                                                     std::move(common_type));
    }

    // The result of the shift operators is equals to the type of the left side.
    // It needs to be treated different than the other binary operator and relational operators
    // 6.5.7 Bitwise shift operators
    if (std::holds_alternative<left_shift_operator>(node->op) || std::holds_alternative<right_shift_operator>(node->op))
    {
        return std::make_unique<parser::binary_node>(node->op,
                                                     parser::copy_expression(left),
                                                     parser::copy_expression(right),
                                                     get_type(left));
    }

    return std::make_unique<parser::binary_node>(node->op,
                                                 std::move(converted_left),
                                                 std::move(converted_right),
                                                 int_type{});
}

auto process_block(const parser::block &node, symbol_table::symbol_table &table)
  -> std::expected<parser::block, semantic_error>
{
    std::vector<parser::block_item> block;
    for (const auto &item : node.items)
    {
        auto tmp = process_block_item(item, table);
        if (tmp.has_value() == false)
        {
            return std::unexpected{ tmp.error() };
        }
        block.push_back(std::move(tmp.value()));
    }

    return parser::block{ std::move(block) };
};

auto process_block_item(const parser::block_item &node, symbol_table::symbol_table &table)
  -> std::expected<parser::block_item, semantic_error>
{
    return std::visit(
      visitor{
        [&table](const parser::declaration &n) -> std::expected<parser::block_item, semantic_error> {
            return process_declaration(n, table, true);
        },
        [&table](const parser::statement &n) -> std::expected<parser::block_item, semantic_error> {
            return process_statement(n, table);
        },
        [](const std::monostate &) -> std::expected<parser::block_item, semantic_error> { return std::monostate{}; },
      },
      node);
}

auto process_cast_expression(const std::unique_ptr<parser::cast_expression> &node, symbol_table::symbol_table &table)
  -> std::expected<std::unique_ptr<parser::cast_expression>, semantic_error>
{
    using parser::copy_expression;
    auto p_exp = process_expression(node->exp, table);
    if (p_exp.has_value() == false)
    {
        return std::unexpected{ p_exp.error() };
    }
    auto exp = std::get<parser::expression>(std::move(p_exp.value()));

    if ((get_type(exp) == double_type{} && is_pointer(node->target)) ||
        (is_pointer(get_type(exp)) && node->target == double_type{}))
    {
        auto msg = fmt::format("Cannot cast from double to pointer or pointer to double");
        return std::unexpected{ semantic_error{ msg } };
    }

    return std::make_unique<parser::cast_expression>(copy_type(node->target),
                                                     copy_expression(exp),
                                                     copy_type(node->target));
}

auto process_compound_statement(const std::unique_ptr<parser::compound_statement> &node,
                                symbol_table::symbol_table &table)
  -> std::expected<std::unique_ptr<parser::compound_statement>, semantic_error>
{
    auto b = process_block(node->block, table);
    if (b.has_value() == false)
    {
        return std::unexpected{ b.error() };
    }

    return std::make_unique<parser::compound_statement>(std::move(b).value());
}

auto process_conditional_node(const std::unique_ptr<parser::conditional_node> &node, symbol_table::symbol_table &table)
  -> std::expected<std::unique_ptr<parser::conditional_node>, semantic_error>
{
    auto p_cond = process_expression(node->condition, table);
    if (p_cond.has_value() == false)
    {
        return std::unexpected{ p_cond.error() };
    }
    auto cond = std::get<parser::expression>(std::move(p_cond.value()));

    auto p_e1 = process_expression(node->e1, table);
    if (p_e1.has_value() == false)
    {
        return std::unexpected{ p_e1.error() };
    }
    auto e1 = std::get<parser::expression>(std::move(p_e1.value()));

    auto p_e2 = process_expression(node->e2, table);
    if (p_e2.has_value() == false)
    {
        return std::unexpected{ p_e2.error() };
    }
    auto e2 = std::get<parser::expression>(std::move(p_e2.value()));

    type common_type;
    if (is_pointer(get_type(e1)) || is_pointer(get_type(e2)))
    {
        auto ptr_type = wccff::sema::get_common_pointer_type(e1, e2);
        if (ptr_type.has_value() == false)
        {
            return std::unexpected(ptr_type.error());
        }
        common_type = std::move(ptr_type.value());
    }
    else
    {
        common_type = wccff::get_common_type(get_type(e1), get_type(e2));
    }

    auto converted_e1 = parser::convert_to(e1, common_type);
    auto converted_e2 = parser::convert_to(e2, common_type);

    return std::make_unique<parser::conditional_node>(std::move(cond),
                                                      std::move(converted_e1),
                                                      std::move(converted_e2),
                                                      std::move(common_type));
}

auto process_declaration(const parser::declaration &node, symbol_table::symbol_table &table, bool inner_block)
  -> std::expected<parser::declaration, semantic_error>
{
    return std::visit(visitor{
                        [&table, inner_block](
                          const parser::function_declaration &n) -> std::expected<parser::declaration, semantic_error> {
                            return process_function_declaration(n, table, inner_block);
                        },
                        [&table, inner_block](
                          const parser::variable_declaration &n) -> std::expected<parser::declaration, semantic_error> {
                            return process_variable_declaration(n, table, inner_block);
                        },
                      },
                      node);
}

auto process_dereference(const std::unique_ptr<parser::dereference> &node, symbol_table::symbol_table &table)
  -> std::expected<std::unique_ptr<parser::dereference>, semantic_error>
{
    auto p_exp = process_expression(node->exp, table);
    if (p_exp.has_value() == false)
    {
        return std::unexpected{ p_exp.error() };
    }
    auto exp = std::get<parser::expression>(std::move(p_exp.value()));

    auto exp_type = get_type(exp);
    if (is_pointer(exp_type) == false)
    {
        auto msg = fmt::format("Cannot dereference a non pointer type");
        return std::unexpected{ semantic_error{ msg } };
    }

    const auto &t = std::get<std::unique_ptr<pointer>>(exp_type);

    return std::make_unique<parser::dereference>(std::move(exp), copy_type(t->referenced));
}

auto process_do_while_statement(const std::unique_ptr<parser::do_while_statement> &node,
                                symbol_table::symbol_table &table)
  -> std::expected<std::unique_ptr<parser::do_while_statement>, semantic_error>
{
    auto body = process_statement(node->body, table);
    if (body.has_value() == false)
    {
        return std::unexpected{ body.error() };
    }

    auto p_cond = process_expression(node->condition, table);
    if (p_cond.has_value() == false)
    {
        return std::unexpected{ p_cond.error() };
    }
    auto cond = std::get<parser::expression>(std::move(p_cond.value()));

    return std::make_unique<parser::do_while_statement>(std::move(body.value()), std::move(cond), node->label);
}

auto process_expression(const parser::expression &node, symbol_table::symbol_table &table)
  -> std::expected<std::variant<parser::expression, std::unique_ptr<parser::compound_statement>>, semantic_error>
{
    using ret_type =
      std::expected<std::variant<parser::expression, std::unique_ptr<parser::compound_statement>>, semantic_error>;
    return std::visit(
      visitor{
        [&](const std::unique_ptr<parser::address_of> &n) -> ret_type { return process_address_of(n, table); },
        [&](const std::unique_ptr<parser::assignment_node> &n) { return process_assignment_node(n, table); },
        [&](const std::unique_ptr<parser::binary_node> &n) -> ret_type { return process_binary_node(n, table); },
        [&](const std::unique_ptr<parser::cast_expression> &n) -> ret_type {
            return process_cast_expression(n, table);
        },
        [&](const std::unique_ptr<parser::conditional_node> &n) -> ret_type {
            return process_conditional_node(n, table);
        },
        [&](const std::unique_ptr<parser::dereference> &n) -> ret_type { return process_dereference(n, table); },
        [&](const std::unique_ptr<parser::function_call> &n) -> ret_type { return process_function_call(n, table); },
        [&](const std::unique_ptr<parser::unary_node> &n) -> ret_type { return process_unary_node(n, table); },
        [&](const parser::var &n) -> ret_type { return process_var(n, table); },
        [&](const constant &n) -> ret_type { return n; },
        [&](const auto &n) -> ret_type { throw std::logic_error("unimplemented"); },
      },
      node);
}

auto process_for_init(const parser::for_init &node, symbol_table::symbol_table &table)
  -> std::expected<parser::for_init, semantic_error>
{
    return std::visit(visitor{
                        [&](const parser::init_declaration &n) -> std::expected<parser::for_init, semantic_error> {
                            return process_init_declaration(n, table);
                        },
                        [&](const parser::init_expression &n) -> std::expected<parser::for_init, semantic_error> {
                            return process_init_expression(n, table);
                        },
                      },
                      node);
}

auto process_for_statement(const std::unique_ptr<parser::for_statement> &node, symbol_table::symbol_table &table)
  -> std::expected<std::unique_ptr<parser::for_statement>, semantic_error>
{
    auto init = process_for_init(node->init, table);
    if (init.has_value() == false)
    {
        return std::unexpected{ init.error() };
    }
    std::optional<parser::expression> cond;
    if (node->condition.has_value())
    {
        auto p_tmp = process_expression(node->condition.value(), table);
        if (p_tmp.has_value() == false)
        {
            return std::unexpected{ p_tmp.error() };
        }
        auto tmp = std::get<parser::expression>(std::move(p_tmp.value()));
        cond = std::move(tmp);
    }

    std::optional<parser::expression> post;
    if (node->post.has_value())
    {
        auto p_tmp = process_expression(node->post.value(), table);
        if (p_tmp.has_value() == false)
        {
            return std::unexpected{ p_tmp.error() };
        }
        auto tmp = std::get<parser::expression>(std::move(p_tmp.value()));
        post = std::move(tmp);
    }

    auto body = process_statement(node->body, table);
    if (body.has_value() == false)
    {
        return std::unexpected{ body.error() };
    }

    return std::make_unique<parser::for_statement>(std::move(init.value()),
                                                   std::move(cond),
                                                   std::move(post),
                                                   std::move(body.value()),
                                                   node->label);
}

auto process_function_call(const std::unique_ptr<parser::function_call> &node, symbol_table::symbol_table &table)
  -> std::expected<std::unique_ptr<parser::function_call>, semantic_error>
{
    auto symbol = table.get(node->name);
    if (std::holds_alternative<std::unique_ptr<fun_type>>(symbol->type) == false)
    {
        auto msg = fmt::format("Variable '{}' used as a function call", node->name.name);
        return std::unexpected{ semantic_error{ msg } };
    }
    auto &params = std::get<std::unique_ptr<fun_type>>(symbol->type)->params;
    if (params.size() != node->arguments.size() && std::holds_alternative<void_type>(params.front()) == false)
    {
        auto msg = fmt::format("Number of parameters mismatch", node->name.name);
        return std::unexpected{ semantic_error{ msg } };
    }

    std::vector<parser::expression> args;
    for (const auto &[e, t] : std::views::zip(node->arguments, params))
    {
        auto p_result = process_expression(e, table);
        if (p_result.has_value() == false)
        {
            return std::unexpected{ p_result.error() };
        }
        auto result = std::get<parser::expression>(std::move(p_result.value()));

        auto p = convert_by_assignment(result, t);
        if (p.has_value() == false)
        {
            return std::unexpected{ p.error() };
        }

        args.push_back(std::move(p.value()));
    }

    auto &return_type = std::get<std::unique_ptr<fun_type>>(symbol->type)->return_type;

    return std::make_unique<parser::function_call>(node->name, std::move(args), copy_type(return_type));
}

auto process_function_declaration(const parser::function_declaration &node,
                                  symbol_table::symbol_table &table,
                                  bool inner_block) -> std::expected<parser::function_declaration, semantic_error>
{
    bool has_body = node.body.has_value();
    bool is_defined = false;
    bool is_global = node.storage_class != parser::storage_class::static_storage;
    if (node.storage_class == parser::storage_class::static_storage && inner_block == true)
    {
        auto msg = fmt::format("Function declaration {} cannot be static in a inner scope", node.name.name);
        return std::unexpected{ semantic_error{ msg } };
    }
    if (auto s = table.get(node.name); s.has_value())
    {
        if (s->type != node.f_type)
        {
            auto msg = fmt::format("Incompatible function declaration for '{}'", node.name.name);
            return std::unexpected{ semantic_error{ msg } };
        }
        if (std::holds_alternative<symbol_table::func_attributes>(s->attrs) == false)
        {
            auto msg = fmt::format("Internal error: function {} doesn't have func_attributes", s->name.name);
            return std::unexpected{ semantic_error{ msg } };
        }
        auto attrs = std::get<symbol_table::func_attributes>(s->attrs);
        is_defined = attrs.is_defined;
        if (attrs.is_defined && has_body)
        {
            auto msg = fmt::format("Duplicate function declaration for '{}'", node.name.name);
            return std::unexpected{ semantic_error{ msg } };
        }
        if (attrs.is_global && node.storage_class == parser::storage_class::static_storage)
        {
            auto msg = fmt::format("Static function declaration follows non static declarations for {}", s->name.name);
            return std::unexpected{ semantic_error{ msg } };
        }
        has_body = has_body || attrs.is_defined;
        is_global = attrs.is_global;
    }
    auto new_attrs = symbol_table::func_attributes{ is_defined || has_body, is_global };
    table.add(node.name, copy_type(node.f_type), new_attrs);
    std::optional<parser::block> block;
    if (node.body.has_value())
    {
        // Start processing the body of the function.
        // Set the name of the current function in the symbol table.
        table.current_processing_function = node.name;

        if (inner_block)
        {
            auto msg = fmt::format("Inner function definition for '{}'", node.name.name);
            return std::unexpected{ semantic_error{ msg } };
        }

        auto &params_type = std::get<std::unique_ptr<fun_type>>(node.f_type)->params;
        for (const auto &[arg_name, arg_type] : std::views::zip(node.arguments, params_type))
        {
            auto attrs = symbol_table::local_attributes{};
            table.add(arg_name, arg_type, attrs);
        }
        auto tmp = process_block(node.body.value(), table);
        if (tmp.has_value() == false)
        {
            return std::unexpected{ tmp.error() };
        }

        block = std::move(tmp.value());
    }

    return parser::function_declaration{ node.name,
                                         node.arguments,
                                         std::move(block),
                                         copy_type(node.f_type),
                                         node.storage_class };
}

auto process_if_node(const std::unique_ptr<parser::if_node> &node, symbol_table::symbol_table &table)
  -> std::expected<std::unique_ptr<parser::if_node>, semantic_error>
{
    auto p_op = process_expression(node->op, table);
    if (p_op.has_value() == false)
    {
        return std::unexpected{ p_op.error() };
    }
    auto op = std::get<parser::expression>(std::move(p_op.value()));

    auto then_stmt = process_statement(node->then_stmt, table);
    if (then_stmt.has_value() == false)
    {
        return std::unexpected{ then_stmt.error() };
    }

    std::optional<parser::statement> else_stmt;
    if (node->else_stmt.has_value())
    {
        auto tmp = process_statement(node->else_stmt.value(), table);
        if (tmp.has_value() == false)
        {
            return std::unexpected{ tmp.error() };
        }
        else_stmt = std::move(tmp.value());
    }

    return std::make_unique<parser::if_node>(std::move(op), std::move(then_stmt.value()), std::move(else_stmt));
}

auto process_init_declaration(const parser::init_declaration &node, symbol_table::symbol_table &table)
  -> std::expected<parser::init_declaration, semantic_error>
{
    auto tmp = process_variable_declaration_local_scope(node.decl, table);
    if (tmp.has_value() == false)
    {
        return std::unexpected{ tmp.error() };
    }
    return parser::init_declaration{ std::move(tmp.value()) };
}

auto process_init_expression(const parser::init_expression &node, symbol_table::symbol_table &table)
  -> std::expected<parser::init_expression, semantic_error>
{
    if (node.expression.has_value())
    {
        auto p_tmp = process_expression(node.expression.value(), table);
        if (p_tmp.has_value() == false)
        {
            return std::unexpected{ p_tmp.error() };
        }
        auto tmp = std::get<parser::expression>(std::move(p_tmp.value()));
        return parser::init_expression{ std::move(tmp) };
    }
    return parser::init_expression{ std::nullopt };
}

auto process_labelled_statement(const std::unique_ptr<parser::labelled_statement> &node,
                                symbol_table::symbol_table &table)
  -> std::expected<std::unique_ptr<parser::labelled_statement>, semantic_error>
{
    auto body = process_statement(node->body, table);
    if (body.has_value() == false)
    {
        return std::unexpected{ body.error() };
    }

    return std::make_unique<parser::labelled_statement>(node->label, std::move(body.value()));
}

auto process_program(const parser::program &node, symbol_table::symbol_table &table)
  -> std::expected<parser::program, semantic_error>
{
    std::vector<parser::declaration> function_declarations;
    for (const auto &f : node.f)
    {
        auto tmp = process_declaration(f, table, false);
        if (tmp.has_value() == false)
        {
            return std::unexpected{ tmp.error() };
        }
        function_declarations.push_back(std::move(tmp.value()));
    }
    return parser::program{ std::move(function_declarations) };
}

auto process_return_node(const parser::return_node &node, symbol_table::symbol_table &table)
  -> std::expected<parser::return_node, semantic_error>
{
    auto p_exp = process_expression(node.e, table);
    if (p_exp.has_value() == false)
    {
        return std::unexpected{ p_exp.error() };
    }
    auto exp = std::get<parser::expression>(std::move(p_exp.value()));

    auto current_function = table.get(table.current_processing_function);
    auto &ret_type = std::get<std::unique_ptr<fun_type>>(current_function->type);

    auto p = convert_by_assignment(exp, ret_type->return_type);
    if (p.has_value() == false)
    {
        return std::unexpected{ p.error() };
    }

    return parser::return_node{ std::move(p.value()) };
}

auto process_statement(const parser::statement &node, symbol_table::symbol_table &table)
  -> std::expected<parser::statement, semantic_error>
{
    return std::visit(
      visitor{
        [&](const parser::return_node &n) -> std::expected<parser::statement, semantic_error> {
            return process_return_node(n, table);
        },
        [&](const parser::expression &n) -> std::expected<parser::statement, semantic_error> {
            auto p_exp = process_expression(n, table);
            if (p_exp.has_value() == false)
            {
                return std::unexpected{ p_exp.error() };
            }

            if (std::holds_alternative<parser::expression>(p_exp.value()))
            {
                return std::get<parser::expression>(std::move(p_exp.value()));
            }
            else
            {
                return std::get<std::unique_ptr<parser::compound_statement>>(std::move(p_exp.value()));
            }
        },
        [&](const std::unique_ptr<parser::if_node> &n) -> std::expected<parser::statement, semantic_error> {
            return process_if_node(n, table);
        },
        [&](const std::unique_ptr<parser::compound_statement> &n) -> std::expected<parser::statement, semantic_error> {
            return process_compound_statement(n, table);
        },
        [&](const parser::break_statement &n) -> std::expected<parser::statement, semantic_error> { return n; },
        [&](const parser::continue_statement &n) -> std::expected<parser::statement, semantic_error> { return n; },
        [&](const parser::goto_statement &n) -> std::expected<parser::statement, semantic_error> { return n; },
        [&](const std::unique_ptr<parser::while_statement> &n) -> std::expected<parser::statement, semantic_error> {
            return process_while_statement(n, table);
        },
        [&](const std::unique_ptr<parser::do_while_statement> &n) -> std::expected<parser::statement, semantic_error> {
            return process_do_while_statement(n, table);
        },
        [&](const std::unique_ptr<parser::for_statement> &n) -> std::expected<parser::statement, semantic_error> {
            return process_for_statement(n, table);
        },
        [&](const std::monostate &n) -> std::expected<parser::statement, semantic_error> { return std::monostate{}; },
        [&](const std::unique_ptr<parser::labelled_statement> &n) -> std::expected<parser::statement, semantic_error> {
            return process_labelled_statement(n, table);
        },
      },
      node);
}

auto process_unary_node(const std::unique_ptr<parser::unary_node> &node, symbol_table::symbol_table &table)
  -> std::expected<std::unique_ptr<parser::unary_node>, semantic_error>
{
    auto p_exp = process_expression(node->exp, table);
    if (p_exp.has_value() == false)
    {
        return std::unexpected{ p_exp.error() };
    }
    auto exp = std::get<parser::expression>(std::move(p_exp.value()));

    if (is_pointer(get_type(exp)))
    {
        if (std::holds_alternative<bitwise_complement_operator>(node->op))
        {
            auto msg = fmt::format("Bitwise complement '~' cannot be applied to a pointer");
            return std::unexpected{ semantic_error{ msg } };
        }
        if (std::holds_alternative<negate_operator>(node->op))
        {
            auto msg = fmt::format("Negate '-' cannot be applied to a pointer");
            return std::unexpected{ semantic_error{ msg } };
        }
    }

    if (std::holds_alternative<bitwise_complement_operator>(node->op) && get_type(exp) == double_type{})
    {
        auto msg = fmt::format("Bitwise complement '~' cannot be applied to a double");
        return std::unexpected{ semantic_error{ msg } };
    }

    if (std::holds_alternative<negate_operator>(node->op) ||
        std::holds_alternative<bitwise_complement_operator>(node->op))
    {
        return std::make_unique<parser::unary_node>(node->op, std::move(exp), copy_optional_type(get_type(exp)));
    }
    else
    {
        return std::make_unique<parser::unary_node>(node->op, std::move(exp), int_type{});
    }
}

auto process_var(const parser::var &node, symbol_table::symbol_table &table)
  -> std::expected<parser::var, semantic_error>
{
    const auto s = table.get(node.name);
    if (s.has_value() == false)
    {
        auto msg = fmt::format("Variable '{}' used before declaration", node.name.name);
        return std::unexpected{ semantic_error{ msg } };
    }
    if (std::holds_alternative<std::unique_ptr<fun_type>>(s->type))
    {
        return std::unexpected<semantic_error>{ fmt::format("Function used as variable") };
    }

    return parser::var{ node.name, copy_type(s->type) };
}

auto process_variable_declaration(const parser::variable_declaration &node,
                                  symbol_table::symbol_table &table,
                                  bool inner_block) -> std::expected<parser::variable_declaration, semantic_error>
{
    if (auto s = table.get(node.name); s.has_value())
    {
        if (node.var_type != s->type)
        {
            auto msg = fmt::format("Type mismatch for variable '{}'", node.name.name);
            return std::unexpected<semantic_error>{ msg };
        }
    }
    return inner_block ? process_variable_declaration_local_scope(node, table)
                       : process_variable_declaration_file_scope(node, table);
}

auto process_variable_declaration_file_scope(const parser::variable_declaration &node,
                                             symbol_table::symbol_table &table)
  -> std::expected<parser::variable_declaration, semantic_error>
{
    auto copy_init = [](const std::optional<parser::expression> &init) {
        return init.has_value() ? std::make_optional(parser::copy_expression(init.value())) : std::nullopt;
    };
    symbol_table::initial_value init_value;
    if (node.init.has_value())
    {
        if (std::holds_alternative<constant>(node.init.value()))
        {
            if (is_null_pointer_constant(node.init.value()) == false && is_pointer(node.var_type))
            {
                return std::unexpected<semantic_error>{ fmt::format("Non Constant initialiser for {}",
                                                                    node.name.name) };
            }
            auto int_node = std::get<constant>(node.init.value());
            init_value = convert_constant(int_node, node.var_type);
        }
        else
        {
            return std::unexpected<semantic_error>{ fmt::format("Non Constant initialiser for {}", node.name.name) };
        }
    }
    else
    {
        if (node.storage_class == parser::storage_class::extern_storage)
        {
            init_value = symbol_table::no_initialiser{};
        }
        else
        {
            init_value = symbol_table::tentative{};
        }
    }

    bool global = node.storage_class != parser::storage_class::static_storage;

    if (auto s = table.get(node.name); s.has_value())
    {
        if (std::holds_alternative<std::unique_ptr<fun_type>>(s->type))
        {
            auto msg = fmt::format("Function redeclared as variable {}", node.name.name);
            return std::unexpected<semantic_error>{ msg };
        }

        if (std::holds_alternative<symbol_table::static_attributes>(s->attrs) == false)
        {
            auto msg = fmt::format("Internal error: File scope variable {} doesn't have static_attributes",
                                   s->name.name);
            return std::unexpected{ semantic_error{ msg } };
        }
        auto attrs = std::get<symbol_table::static_attributes>(s->attrs);

        if (node.storage_class == parser::storage_class::extern_storage)
        {
            global = attrs.is_global;
        }
        else if (attrs.is_global != global)
        {
            auto msg = fmt::format("Conflicting variable '{}' linkage", node.name.name);
            return std::unexpected<semantic_error>{ msg };
        }

        if (std::holds_alternative<initial>(attrs.init))
        {
            if (std::holds_alternative<initial>(init_value))
            {
                auto msg = fmt::format("Conflicting file scope variable '{}' definitions", node.name.name);
                return std::unexpected<semantic_error>{ msg };
            }
            else
            {
                init_value = attrs.init;
            }
        }
        else if (std::holds_alternative<initial>(init_value) == false &&
                 std::holds_alternative<symbol_table::tentative>(attrs.init))
        {
            init_value = symbol_table::tentative{};
        }
    }

    symbol_table::static_attributes attrs{ init_value, global };
    table.add(node.name, copy_type(node.var_type), attrs);
    return parser::variable_declaration{ node.name,
                                         copy_init(node.init),
                                         copy_type(node.var_type),
                                         node.storage_class };
}

auto process_variable_declaration_local_scope(const parser::variable_declaration &node,
                                              symbol_table::symbol_table &table)
  -> std::expected<parser::variable_declaration, semantic_error>
{
    auto copy_init = [](const std::optional<parser::expression> &init) {
        return init.has_value() ? std::make_optional(parser::copy_expression(init.value())) : std::nullopt;
    };

    if (node.storage_class == parser::storage_class::extern_storage)
    {
        if (node.init.has_value())
        {
            auto msg = fmt::format("Initializer on a local extern variable declaration {}", node.name.name);
            return std::unexpected<semantic_error>{ msg };
        }
        if (auto s = table.get(node.name); s.has_value())
        {
            if (std::holds_alternative<std::unique_ptr<fun_type>>(s->type))
            {
                auto msg = fmt::format("Function redeclared as variable {}", node.name.name);
                return std::unexpected{ semantic_error{ msg } };
            }
        }
        else
        {
            auto attrs = symbol_table::static_attributes{ symbol_table::no_initialiser{}, true };
            table.add(node.name, copy_type(node.var_type), attrs);
        }
    }
    else if (node.storage_class == parser::storage_class::static_storage)
    {
        symbol_table::initial_value init_value{};
        if (node.init.has_value() == false)
        {
            init_value = get_default_initial(node.var_type);
        }
        else if (std::holds_alternative<constant>(node.init.value()))
        {
            auto tmp = std::get<constant>(node.init.value());
            init_value = convert_constant(tmp, node.var_type);
        }
        else
        {
            auto msg = fmt::format("Non-Constant initialiser for {} on local static variable", node.name.name);
            return std::unexpected{ semantic_error{ msg } };
        }
        auto attrs = symbol_table::static_attributes{ init_value, false };
        table.add(node.name, copy_type(node.var_type), attrs);
    }
    else
    {
        auto attrs = symbol_table::local_attributes{};
        table.add(node.name, copy_type(node.var_type), attrs);
        std::optional<parser::expression> init;
        if (node.init.has_value())
        {
            auto p_tmp = process_expression(node.init.value(), table);
            if (p_tmp.has_value() == false)
            {
                return std::unexpected{ p_tmp.error() };
            }
            auto tmp = std::get<parser::expression>(std::move(p_tmp.value()));

            auto p = convert_by_assignment(tmp, node.var_type);
            if (p.has_value() == false)
            {
                return std::unexpected{ p.error() };
            }
            init = std::move(p.value());
        }

        return parser::variable_declaration{ node.name, std::move(init), copy_type(node.var_type), node.storage_class };
    }
    return parser::variable_declaration{ node.name,
                                         copy_init(node.init),
                                         copy_type(node.var_type),
                                         node.storage_class };
}

auto process_while_statement(const std::unique_ptr<parser::while_statement> &node, symbol_table::symbol_table &table)
  -> std::expected<std::unique_ptr<parser::while_statement>, semantic_error>
{
    auto p_cond = process_expression(node->condition, table);
    if (p_cond.has_value() == false)
    {
        return std::unexpected{ p_cond.error() };
    }
    auto cond = std::get<parser::expression>(std::move(p_cond.value()));

    auto body = process_statement(node->body, table);
    if (body.has_value() == false)
    {
        return std::unexpected{ body.error() };
    }

    return std::make_unique<parser::while_statement>(std::move(cond), std::move(body.value()), node->label);
}

} // namespace wccff::sema::type_checker
