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

#include "parser.h"
#include "utils.h"
#include "visitor.h"
#include <algorithm>
#include <charconv>
#include <fast_float/fast_float.h>
#include <fmt/core.h>
#include <ranges>

namespace wccff::parser {

static parser_error generate_unexpected_end_of_tokens(const tokens &tokens)
{
    auto previous = tokens.previous_token();
    auto msg = fmt::format("{}: Error: Unexpected end of tokens after '{}'", previous.loc, previous.text);
    return { msg };
}

assignment_node::assignment_node(expression lhs_, expression rhs_)
  : lhs(std::move(lhs_))
  , rhs(std::move(rhs_))
{
}
assignment_node::assignment_node(expression lhs_, expression rhs_, std::optional<wccff::type> type_)
  : lhs(std::move(lhs_))
  , rhs(std::move(rhs_))
  , type(std::move(type_))
{
}

binary_node::binary_node(binary_operator op_, expression left_, expression right_)
  : op(op_)
  , left(std::move(left_))
  , right(std::move(right_))
{
}
binary_node::binary_node(binary_operator op_, expression left_, expression right_, std::optional<wccff::type> type_)
  : op(op_)
  , left(std::move(left_))
  , right(std::move(right_))
  , type(std::move(type_))
{
}

unary_node::unary_node(unary_operator op_, expression expression_)
  : op(op_)
  , exp(std::move(expression_))
{
}
unary_node::unary_node(unary_operator op_, expression expression_, std::optional<wccff::type> type_)
  : op(op_)
  , exp(std::move(expression_))
  , type(std::move(type_))
{
}

expression convert_to(const expression &e, const type &t)
{
    if (get_type(e) == t)
    {
        return copy_expression(e);
    }

    return std::make_unique<cast_expression>(copy_type(t), copy_expression(e), copy_type(t));
}

std::optional<parser_error> consume_tokens(tokens &tokens, const std::vector<lexer::token_type> &list)
{
    if (list.size() > tokens.remaining_tokens())
    {
        return generate_unexpected_end_of_tokens(tokens);
    }

    for (const auto &t : list)
    {
        auto next_token = tokens.get_next_token_safe();
        if (next_token.type != t)
        {
            auto msg = fmt::format("Parse failure at: {}. Expected '{}' found {}", next_token.loc, t, next_token.text);
            return parser_error{ msg };
        }
    }

    return std::nullopt;
}

std::unique_ptr<address_of> copy_address_of(const std::unique_ptr<address_of> &node)
{
    return std::make_unique<address_of>(copy_expression(node->exp), copy_optional_type(node->type));
}

std::unique_ptr<assignment_node> copy_assignment_node(const std::unique_ptr<assignment_node> &node)
{
    return std::make_unique<assignment_node>(copy_expression(node->lhs),
                                             copy_expression(node->rhs),
                                             copy_optional_type(node->type));
}
std::unique_ptr<binary_node> copy_binary_node(const std::unique_ptr<binary_node> &node)
{
    return std::make_unique<binary_node>(node->op,
                                         copy_expression(node->left),
                                         copy_expression(node->right),
                                         copy_optional_type(node->type));
}
std::unique_ptr<cast_expression> copy_cast_expresion(const std::unique_ptr<cast_expression> &node)
{
    return std::make_unique<cast_expression>(copy_type(node->target),
                                             copy_expression(node->exp),
                                             copy_optional_type(node->type));
}
std::unique_ptr<conditional_node> copy_conditional_node(const std::unique_ptr<conditional_node> &node)
{
    return std::make_unique<conditional_node>(copy_expression(node->condition),
                                              copy_expression(node->e1),
                                              copy_expression(node->e2),
                                              copy_optional_type(node->type));
}
variable_declaration copy_declaration(const variable_declaration &node)
{
    std::optional<expression> init;
    if (node.init.has_value())
    {
        init = copy_expression(node.init.value());
    }
    return variable_declaration{ node.name, std::move(init), copy_type(node.var_type), node.storage_class };
}

std::unique_ptr<dereference> copy_dereference(const std::unique_ptr<dereference> &node)
{
    return std::make_unique<dereference>(copy_expression(node->exp), copy_optional_type(node->type));
}

expression copy_expression(const expression &exp)
{
    return std::visit(
      visitor{
        [](const constant &n) -> expression { return n; },
        [](const var &n) -> expression { return var{ n.name, copy_optional_type(n.type) }; },
        [](const std::unique_ptr<unary_node> &n) -> expression { return copy_unary_node(n); },
        [](const std::unique_ptr<binary_node> &n) -> expression { return copy_binary_node(n); },
        [](const std::unique_ptr<cast_expression> &n) -> expression { return copy_cast_expresion(n); },
        [](const std::unique_ptr<assignment_node> &n) -> expression { return copy_assignment_node(n); },
        [](const std::unique_ptr<conditional_node> &n) -> expression { return copy_conditional_node(n); },
        [](const std::unique_ptr<function_call> &n) -> expression { return copy_function_call(n); },
        [](const std::unique_ptr<address_of> &n) -> expression { return copy_address_of(n); },
        [](const std::unique_ptr<dereference> &n) -> expression { return copy_dereference(n); },
      },
      exp);
}
std::unique_ptr<function_call> copy_function_call(const std::unique_ptr<function_call> &n)
{
    std::vector<expression> args;
    for (const auto &a : n->arguments)
    {
        args.push_back(copy_expression(a));
    }
    return std::make_unique<function_call>(n->name, std::move(args), copy_optional_type(n->type));
}

std::unique_ptr<unary_node> copy_unary_node(const std::unique_ptr<unary_node> &node)
{
    return std::make_unique<unary_node>(node->op, copy_expression(node->exp), copy_optional_type(node->type));
}

std::optional<parser_error> parse_semicolon(tokens &tokens)
{
    auto next_token = tokens.get_next_token();
    if (next_token.has_value() == false)
    {
        return generate_unexpected_end_of_tokens(tokens);
    }
    if (next_token->type != lexer::token_type::semicolon)
    {
        auto msg = fmt::format("Parse failure at: {}. Expected ';' found {}", next_token->loc, next_token->type);
        return parser_error{ msg };
    }

    return std::nullopt;
}

type get_type(const constant &n)
{
    return std::visit(visitor{
                        [](const double_constant &) -> type { return double_type{}; },
                        [](const int_constant &) -> type { return int_type{}; },
                        [](const long_constant &) -> type { return long_type{}; },
                        [](const unsigned_int_constant &) -> type { return unsigned_int_type{}; },
                        [](const unsigned_long_constant &) -> type { return unsigned_long_type{}; },
                      },
                      n);
}

type get_type(const expression &n)
{
    return std::visit(visitor{
                        [](const constant &n) { return get_type(n); },
                        [](const std::unique_ptr<address_of> &n) -> type { return get_type(n); },
                        [](const std::unique_ptr<assignment_node> &n) { return get_type(n); },
                        [](const std::unique_ptr<binary_node> &n) { return get_type(n); },
                        [](const std::unique_ptr<cast_expression> &n) { return get_type(n); },
                        [](const std::unique_ptr<conditional_node> &n) { return get_type(n); },
                        [](const std::unique_ptr<dereference> &n) { return copy_type(n->type.value()); },
                        [](const std::unique_ptr<function_call> &n) { return get_type(n); },
                        [](const std::unique_ptr<unary_node> &n) { return get_type(n); },
                        [](const var &n) { return get_type(n); },
                        [&](const auto &) -> type {
                            throw std::logic_error(
                              fmt::format("unimplemented: {}", std::source_location::current().function_name()));
                        },
                      },
                      n);
}
type get_type(const std::unique_ptr<address_of> &n)
{
    return copy_type(n->type.value());
}
inline type get_type(const std::unique_ptr<assignment_node> &n)
{
    return copy_type(n->type.value());
}
inline type get_type(const std::unique_ptr<binary_node> &n)
{
    return copy_type(n->type.value());
}
inline type get_type(const std::unique_ptr<cast_expression> &n)
{
    return copy_type(n->type.value());
}
inline type get_type(const std::unique_ptr<conditional_node> &n)
{
    return copy_type(n->type.value());
}
inline type get_type(const std::unique_ptr<function_call> &n)
{
    return copy_type(n->type.value());
}
inline type get_type(const std::unique_ptr<unary_node> &n)
{
    return copy_type(n->type.value());
}
inline type get_type(const var &n)
{
    return copy_type(n.type.value());
}

bool is_type_specifier(const lexer::token &t)
{
    using enum lexer::token_type;
    return t.type == double_keyword || t.type == int_keyword || t.type == long_keyword || t.type == signed_keyword ||
           t.type == unsigned_keyword;
}

bool is_storage_specifier(const lexer::token &t)
{
    using enum lexer::token_type;
    return t.type == extern_keyword || t.type == static_keyword;
}

std::expected<abstract_declarator, parser_error> parse_abstract_declarator(tokens &tokens)
{
    if (tokens.peek().type == lexer::token_type::multiplication_operator)
    {
        if (auto ec = consume_tokens(tokens, { lexer::token_type::multiplication_operator }); ec.has_value())
        {
            return std::unexpected{ ec.value() };
        }

        if (tokens.peek().type != lexer::token_type::multiplication_operator &&
            tokens.peek().type != lexer::token_type::open_parenthesis)
        {
            return std::make_unique<abstract_pointer>(abstract_base{});
        }

        auto r = parse_abstract_declarator(tokens);
        if (r.has_value() == false)
        {
            return std::unexpected{ r.error() };
        }

        return std::make_unique<abstract_pointer>(std::move(r.value()));
    }

    return parse_direct_abstract_declarator(tokens);
}

std::expected<std::vector<expression>, parser_error> parse_argument_list(tokens &tokens)
{
    std::vector<expression> arguments;
    while (tokens.peek(0).type != lexer::token_type::close_parenthesis)
    {
        auto arg = parse_expression(tokens);
        if (arg.has_value() == false)
        {
            return std::unexpected{ arg.error() };
        }

        arguments.push_back(std::move(arg.value()));

        if (tokens.peek(0).type == lexer::token_type::comma &&
            tokens.peek(1).type != lexer::token_type::close_parenthesis)
        {
            if (auto p = consume_tokens(tokens, { lexer::token_type::comma }); p.has_value())
            {
                return std::unexpected{ p.value() };
            }
        }
    }

    return arguments;
}

std::expected<block_item, parser_error> parse_block_item(tokens &tokens)
{
    using enum lexer::token_type;
    auto next_token = tokens.peek();
    if (is_storage_specifier(next_token) || is_type_specifier(next_token))
    {
        return parse_declaration(tokens);
    }
    return parse_statement(tokens);
}

std::expected<block, parser_error> parse_block(tokens &tokens)
{
    // Discard the '{'
    tokens.discard_token();

    block b;
    while (tokens.peek().type != lexer::token_type::close_brace)
    {
        auto item = parse_block_item(tokens);
        if (item.has_value() == false)
        {
            return std::unexpected{ item.error() };
        }
        b.items.push_back(std::move(item.value()));
    }

    auto token = tokens.get_next_token();
    if (token.has_value() == false)
    {
        return std::unexpected{ generate_unexpected_end_of_tokens(tokens) };
    }
    if (token->type != lexer::token_type::close_brace)
    {
        auto msg = fmt::format("Parse failure at: {}. Expected '}}' found {}", token->loc, token->type);
        return std::unexpected{ parser_error{ msg } };
    }

    return b;
}
std::expected<std::unique_ptr<cast_expression>, parser_error> parse_cast_expression(tokens &tokens)
{
    using enum lexer::token_type;
    if (auto p = consume_tokens(tokens, { open_parenthesis }); p.has_value())
    {
        return std::unexpected{ p.value() };
    }

    auto type = parse_type_specifier(tokens, close_parenthesis);
    if (type.has_value() == false)
    {
        return std::unexpected{ type.error() };
    }

    if (tokens.peek().type != lexer::token_type::close_parenthesis)
    {
        auto dec = parse_abstract_declarator(tokens);
        if (dec.has_value() == false)
        {
            return std::unexpected{ dec.error() };
        }

        auto p1 = process_abstract_declarator(dec.value(), type.value());
        if (p1.has_value() == false)
        {
            return std::unexpected{ p1.error() };
        }

        type = std::move(p1.value());
    }

    if (auto p = consume_tokens(tokens, { close_parenthesis }); p.has_value())
    {
        return std::unexpected{ p.value() };
    }

    auto exp = parse_factor(tokens);
    if (exp.has_value() == false)
    {
        return std::unexpected{ exp.error() };
    }

    return std::make_unique<cast_expression>(std::move(type.value()), std::move(exp.value()));
}

std::expected<std::unique_ptr<compound_statement>, parser_error> parse_compound_statement(tokens &tokens)
{
    auto block = parse_block(tokens);
    if (block.has_value() == false)
    {
        return std::unexpected{ block.error() };
    }
    return std::make_unique<compound_statement>(std::move(block.value()));
}
std::expected<expression, parser_error> parse_conditional(tokens &tokens)
{
    tokens.discard_token();
    auto exp = parse_expression(tokens, 0);
    if (exp.has_value() == false)
    {
        return std::unexpected{ exp.error() };
    }
    auto next_token = tokens.get_next_token();
    if (next_token.has_value() == false)
    {
        return std::unexpected{ generate_unexpected_end_of_tokens(tokens) };
    }

    if (next_token.value().type != lexer::token_type::colon)
    {
        auto msg = fmt::format("Parse failure at: {}. Expected ':' found {}", next_token->loc, next_token->type);
        return std::unexpected{ parser_error{ msg } };
    }

    return exp;
}

std::expected<std::unique_ptr<do_while_statement>, parser_error> parse_do_while(tokens &tokens)
{
    using enum lexer::token_type;
    if (auto p = consume_tokens(tokens, { do_keyword }); p.has_value())
    {
        return std::unexpected{ p.value() };
    }

    auto body = parse_statement(tokens);
    if (body.has_value() == false)
    {
        return std::unexpected{ body.error() };
    }

    if (auto p = consume_tokens(tokens, { while_keyword, open_parenthesis }); p.has_value())
    {
        return std::unexpected{ p.value() };
    }

    auto conditional = parse_expression(tokens);
    if (conditional.has_value() == false)
    {
        return std::unexpected{ conditional.error() };
    }

    if (auto p = consume_tokens(tokens, { close_parenthesis, semicolon }); p.has_value())
    {
        return std::unexpected{ p.value() };
    }

    return std::make_unique<do_while_statement>(std::move(body.value()), std::move(conditional.value()));
}

std::expected<type, parser_error> process_abstract_declarator(const abstract_declarator &declarator,
                                                              const type &base_type)
{
    return std::visit(
      visitor{
        [&base_type](abstract_base) -> std::expected<type, parser_error> { return copy_type(base_type); },
        [&base_type](const std::unique_ptr<abstract_pointer> &node) -> std::expected<type, parser_error> {
            const type derived_type = std::make_unique<wccff::pointer>(copy_type(base_type));
            return process_abstract_declarator(node->inner, derived_type);
        },
      },
      declarator);
}

std::expected<declarator_tmp, parser_error> process_declarator(const declarator &declarator, const type &base_type)
{
    return std::visit(
      visitor{
        [&](const wccff::identifier &id) -> std::expected<declarator_tmp, parser_error> {
            return declarator_tmp{ id, copy_type(base_type), {} };
        },
        [&](const std::unique_ptr<func_declarator> &f) -> std::expected<declarator_tmp, parser_error> {
            if (std::holds_alternative<wccff::identifier>(f->inner))
            {
                auto f_name = std::get<wccff::identifier>(f->inner);
                std::vector<wccff::identifier> p_name;
                std::vector<wccff::type> p_type;

                for (const auto &[i, t] : f->params)
                {
                    auto result = process_declarator(i, t);
                    if (result.has_value() == false)
                    {
                        return std::unexpected{ result.error() };
                    }

                    const auto &[p_i, p_t, c] = result.value();
                    if (std::holds_alternative<std::unique_ptr<wccff::fun_type>>(p_t))
                    {
                        auto msg = fmt::format("Function pointers in parameters not supported");
                        return std::unexpected{ parser_error{ msg } };
                    }
                    p_name.push_back(p_i);
                    p_type.push_back(copy_type(p_t));
                }

                auto derived_type = std::make_unique<wccff::fun_type>(std::move(p_type), copy_type(base_type));
                return declarator_tmp{ f_name, std::move(derived_type), std::move(p_name) };
            }

            auto msg = fmt::format("Can't apply additional type derivations to a function type");
            return std::unexpected{ parser_error{ msg } };
        },
        [&](const std::unique_ptr<pointer_declarator> &d) -> std::expected<declarator_tmp, parser_error> {
            const type derived_type = std::make_unique<pointer>(copy_type(base_type));
            return process_declarator(d->inner, derived_type);
        },
      },
      declarator);
}

std::expected<declaration, parser_error> parse_declaration(tokens &tokens)
{
    auto specifier = parse_specifier(tokens);
    if (specifier.has_value() == false)
    {
        return std::unexpected{ specifier.error() };
    }

    auto p = parse_declarator(tokens);
    if (p.has_value() == false)
    {
        return std::unexpected{ p.error() };
    }

    auto dec = process_declarator(p.value(), specifier.value().t);
    if (dec.has_value() == false)
    {
        return std::unexpected{ dec.error() };
    }

    if (std::holds_alternative<std::unique_ptr<fun_type>>(dec->t))
    {
        return parse_function_declaration(tokens, dec->name, dec->params, specifier->storage, dec->t);
    }

    return parse_variable_declaration(tokens, dec->name, dec->t, specifier->storage);
}

std::expected<declarator, parser_error> parse_declarator(tokens &tokens)
{
    if (tokens.peek().type == lexer::token_type::multiplication_operator)
    {
        if (auto ec = consume_tokens(tokens, { lexer::token_type::multiplication_operator }); ec.has_value())
        {
            return std::unexpected{ ec.value() };
        }

        auto inner = parse_declarator(tokens);
        if (inner.has_value() == false)
        {
            return std::unexpected{ inner.error() };
        }

        return std::make_unique<pointer_declarator>(std::move(inner.value()));
    }

    return parse_direct_declarator(tokens);
}

std::expected<declarator, parser_error> parse_direct_declarator(tokens &tokens)
{
    auto simple = parse_simple_declarator(tokens);
    if (simple.has_value() == false)
    {
        return std::unexpected{ simple.error() };
    }

    if (tokens.peek().type != lexer::token_type::open_parenthesis)
    {
        return simple;
    }

    auto params = parse_params_list(tokens);
    if (params.has_value() == false)
    {
        return std::unexpected{ params.error() };
    }

    return std::make_unique<func_declarator>(std::move(simple.value()), std::move(params.value()));
}

std::expected<declarator, parser_error> parse_simple_declarator(tokens &tokens)
{
    if (tokens.peek().type == lexer::token_type::identifier)
    {
        return parse_identifier(tokens);
    }

    if (auto ec = consume_tokens(tokens, { lexer::token_type::open_parenthesis }); ec.has_value())
    {
        return std::unexpected{ ec.value() };
    }

    auto r = parse_declarator(tokens);
    if (r.has_value() == false)
    {
        return std::unexpected{ r.error() };
    }

    if (auto ec = consume_tokens(tokens, { lexer::token_type::close_parenthesis }); ec.has_value())
    {
        return std::unexpected{ ec.value() };
    }

    return r;
}

std::expected<abstract_declarator, parser_error> parse_direct_abstract_declarator(tokens &tokens)
{
    if (auto ec = consume_tokens(tokens, { lexer::token_type::open_parenthesis }); ec.has_value())
    {
        return std::unexpected{ ec.value() };
    }

    auto r = parse_abstract_declarator(tokens);
    if (r.has_value() == false)
    {
        return std::unexpected{ r.error() };
    }
    if (auto ec = consume_tokens(tokens, { lexer::token_type::close_parenthesis }); ec.has_value())
    {
        return std::unexpected{ ec.value() };
    }

    return std::move(r.value());
}

std::expected<std::optional<expression>, parser_error> parse_optional_expression(tokens &tokens,
                                                                                 lexer::token_type end_token)
{
    if (tokens.peek().type == end_token)
    {
        tokens.discard_token();
        return std::nullopt;
    }

    auto expr = parse_expression(tokens);
    if (expr.has_value() == false)
    {
        return std::unexpected{ expr.error() };
    }

    if (auto p = consume_tokens(tokens, { end_token }); p.has_value())
    {
        return std::unexpected{ p.value() };
    }

    return std::move(expr.value());
}

std::expected<for_init, parser_error> parse_for_init(tokens &tokens)
{
    using enum lexer::token_type;
    if (tokens.peek().type != identifier && tokens.peek().type != semicolon)
    {
        auto type_result = parse_type_specifier(tokens, identifier);
        if (type_result.has_value() == false)
        {
            return std::unexpected{ type_result.error() };
        }

        auto p = parse_declarator(tokens);
        if (p.has_value() == false)
        {
            return std::unexpected{ p.error() };
        }

        auto dec = process_declarator(p.value(), type_result.value());
        if (dec.has_value() == false)
        {
            return std::unexpected{ dec.error() };
        }

        if (std::holds_alternative<std::unique_ptr<wccff::fun_type>>(dec->t))
        {
            auto msg = fmt::format("Illegal function declaration at in for-init");
            return std::unexpected{ parser_error{ msg } };
        }

        auto decl = parse_variable_declaration(tokens, dec->name, dec->t, storage_class::no_storage);
        if (decl.has_value() == false)
        {
            return std::unexpected{ decl.error() };
        }
        return init_declaration{ std::move(decl.value()) };
    }
    auto expr = parse_optional_expression(tokens, semicolon);
    if (expr.has_value() == false)
    {
        return std::unexpected{ expr.error() };
    }

    return init_expression{ std::move(expr.value()) };
}
std::expected<std::unique_ptr<for_statement>, parser_error> parse_for_statement(tokens &tokens)
{
    using enum lexer::token_type;

    if (auto p = consume_tokens(tokens, { for_keyword, open_parenthesis }); p.has_value())
    {
        return std::unexpected{ p.value() };
    }

    auto init = parse_for_init(tokens);
    if (init.has_value() == false)
    {
        return std::unexpected{ init.error() };
    }

    auto conditional = parse_optional_expression(tokens, semicolon);
    if (conditional.has_value() == false)
    {
        return std::unexpected{ conditional.error() };
    }

    auto post = parse_optional_expression(tokens, close_parenthesis);
    if (post.has_value() == false)
    {
        return std::unexpected{ post.error() };
    }

    auto stmt = parse_statement(tokens);
    if (stmt.has_value() == false)
    {
        return std::unexpected{ stmt.error() };
    }

    return std::make_unique<for_statement>(std::move(init.value()),
                                           std::move(conditional.value()),
                                           std::move(post.value()),
                                           std::move(stmt.value()));
}

std::expected<std::unique_ptr<if_node>, parser_error> parse_if_node(tokens &tokens)
{
    // Discard the if keyword
    tokens.discard_token();

    auto open_parens = tokens.get_next_token();
    if (open_parens.has_value() == false)
    {
        return std::unexpected{ generate_unexpected_end_of_tokens(tokens) };
    }

    auto condition = parse_expression(tokens);
    if (condition.has_value() == false)
    {
        return std::unexpected{ condition.error() };
    }

    auto close_parens = tokens.get_next_token();
    if (close_parens.has_value() == false)
    {
        return std::unexpected{ generate_unexpected_end_of_tokens(tokens) };
    }

    auto then_stmt = parse_statement(tokens);
    if (then_stmt.has_value() == false)
    {
        return std::unexpected{ then_stmt.error() };
    }

    std::optional<statement> else_stmt;
    // Since the else is optional, don't consume the token yet.
    auto else_token = tokens.peek();
    if (else_token.type == lexer::token_type::else_keyword)
    {
        tokens.discard_token();
        auto tmp = parse_statement(tokens);
        if (tmp.has_value() == false)
        {
            return std::unexpected{ tmp.error() };
        }
        else_stmt = std::move(tmp.value());
    }

    return std::make_unique<if_node>(std::move(condition.value()), std::move(then_stmt.value()), std::move(else_stmt));
}

std::expected<std::unique_ptr<function_call>, parser_error> parse_function_call(tokens &tokens)
{
    auto name = parse_identifier(tokens);
    if (name.has_value() == false)
    {
        return std::unexpected{ name.error() };
    }

    if (auto p = consume_tokens(tokens, { lexer::token_type::open_parenthesis }); p.has_value())
    {
        return std::unexpected{ p.value() };
    }

    auto arguments = parse_argument_list(tokens);
    if (arguments.has_value() == false)
    {
        return std::unexpected{ arguments.error() };
    }

    if (auto p = consume_tokens(tokens, { lexer::token_type::close_parenthesis }); p.has_value())
    {
        return std::unexpected{ p.value() };
    }

    return std::make_unique<function_call>(std::move(name.value()), std::move(arguments.value()));
}

std::expected<function_declaration, parser_error> parse_function_declaration(tokens &tokens,
                                                                             wccff::identifier name,
                                                                             std::vector<identifier> params,
                                                                             storage_class storage,
                                                                             const type &t)
{
    std::optional<block> body;
    if (tokens.peek(0).type == lexer::token_type::open_brace)
    {
        auto tmp = parse_block(tokens);
        if (tmp.has_value() == false)
        {
            return std::unexpected{ tmp.error() };
        }
        body = std::move(tmp.value());
    }
    else
    {
        if (auto p = consume_tokens(tokens, { lexer::token_type::semicolon }); p.has_value())
        {
            return std::unexpected{ p.value() };
        }
    }

    return function_declaration{ std::move(name), std::move(params), std::move(body), copy_type(t), storage };
}

std::expected<program, parser_error> parse_program(tokens &tokens)
{
    std::vector<declaration> functions;

    while (tokens.remaining_tokens() != 0)
    {
        auto function = parse_declaration(tokens);
        if (function.has_value() == false)
        {
            return std::unexpected{ function.error() };
        }
        functions.push_back(std::move(function.value()));
    }

    return program{ std::move(functions) };
}

std::expected<return_node, parser_error> parse_return_node(tokens &tokens)
{
    auto t1 = tokens.get_next_token();
    if (t1.has_value() == false)
    {
        return std::unexpected{ generate_unexpected_end_of_tokens(tokens) };
    }

    if (t1->type != lexer::token_type::return_keyword)
    {
        auto msg = fmt::format("Parse failure at: {}. Expected return keyword found {}", t1->loc, t1->type);
        return std::unexpected{ parser_error{ msg } };
    }
    auto e = parse_expression(tokens);
    if (e.has_value() == false)
    {
        return std::unexpected{ e.error() };
    }

    auto s = parse_semicolon(tokens);
    if (s.has_value())
    {
        return std::unexpected{ s.value() };
    }

    return return_node{ std::move(e.value()) };
}

std::expected<std::vector<param_declarator>, parser_error> parse_params_list(tokens &tokens)
{
    std::vector<param_declarator> arguments;

    if (auto p = consume_tokens(tokens, { lexer::token_type::open_parenthesis }); p.has_value())
    {
        return std::unexpected{ p.value() };
    }

    if (tokens.peek().type == lexer::token_type::void_keyword)
    {
        if (auto p = consume_tokens(tokens, { lexer::token_type::void_keyword }); p.has_value())
        {
            return std::unexpected{ p.value() };
        }

        if (auto p = consume_tokens(tokens, { lexer::token_type::close_parenthesis }); p.has_value())
        {
            return std::unexpected{ p.value() };
        }

        return arguments;
    }

    while (tokens.peek().type != lexer::token_type::close_parenthesis)
    {
        // Parse parameters list
        // A parameter list, has a
        auto p_type = parse_type_specifier(tokens, lexer::token_type::identifier);
        if (p_type.has_value() == false)
        {
            return std::unexpected{ p_type.error() };
        }

        auto arg = parse_declarator(tokens);
        if (arg.has_value() == false)
        {
            return std::unexpected{ arg.error() };
        }

        if (tokens.peek(0).type == lexer::token_type::comma &&
            tokens.peek(1).type != lexer::token_type::close_parenthesis)
        {
            if (auto p = consume_tokens(tokens, { lexer::token_type::comma }); p.has_value())
            {
                return std::unexpected{ p.value() };
            }
        }

        arguments.emplace_back(std::move(arg.value()), std::move(p_type.value()));
    }

    if (auto p = consume_tokens(tokens, { lexer::token_type::close_parenthesis }); p.has_value())
    {
        return std::unexpected{ p.value() };
    }

    return arguments;
}
std::expected<specifier, parser_error> parse_specifier(tokens &tokens)
{
    std::vector<storage_class> storage;
    std::vector<lexer::token> type;
    for (auto token = tokens.peek(); is_storage_specifier(token) || is_type_specifier(token); token = tokens.peek())
    {
        if (is_type_specifier(token))
        {
            type.push_back(tokens.get_next_token_safe());
        }
        else if (token.type == lexer::token_type::static_keyword)
        {
            consume_tokens(tokens, { lexer::token_type::static_keyword });
            storage.emplace_back(storage_class::static_storage);
        }
        else if (token.type == lexer::token_type::extern_keyword)
        {
            consume_tokens(tokens, { lexer::token_type::extern_keyword });
            storage.emplace_back(storage_class::extern_storage);
        }
    }

    auto type_ret = parse_type(type);
    if (type_ret.has_value() == false)
    {
        return std::unexpected{ type_ret.error() };
    }

    if (storage.size() > 1)
    {
        auto msg = fmt::format("Found more than one storage specifier");
        return std::unexpected{ parser_error{ msg } };
    }

    if (storage.size() == 1)
    {
        return specifier{ storage.front(), std::move(type_ret.value()) };
    }

    return specifier{ storage_class::no_storage, std::move(type_ret.value()) };
}

std::expected<statement, parser_error> parse_statement(tokens &tokens)
{
    using enum lexer::token_type;
    auto next_token = tokens.peek();
    if (next_token.type == lexer::token_type::return_keyword)
    {
        return parse_return_node(tokens);
    }
    if (next_token.type == lexer::token_type::semicolon)
    {
        tokens.discard_token();
        return std::monostate{};
    }
    if (next_token.type == lexer::token_type::if_keyword)
    {
        return parse_if_node(tokens);
    }
    if (next_token.type == lexer::token_type::open_brace)
    {
        return parse_compound_statement(tokens);
    }
    if (next_token.type == lexer::token_type::break_keyword)
    {
        if (auto p = consume_tokens(tokens, { break_keyword, semicolon }); p.has_value())
        {
            return std::unexpected{ p.value() };
        }
        return break_statement{};
    }
    if (next_token.type == lexer::token_type::continue_keyword)
    {
        if (auto p = consume_tokens(tokens, { continue_keyword, semicolon }); p.has_value())
        {
            return std::unexpected{ p.value() };
        }
        return continue_statement{};
    }
    if (next_token.type == lexer::token_type::while_keyword)
    {
        return parse_while_statement(tokens);
    }
    if (next_token.type == lexer::token_type::do_keyword)
    {
        return parse_do_while(tokens);
    }
    if (next_token.type == lexer::token_type::for_keyword)
    {
        return parse_for_statement(tokens);
    }
    if (next_token.type == identifier && tokens.peek_after_next().type == colon)
    {
        auto label = parse_identifier(tokens);
        if (label.has_value() == false)
        {
            return std::unexpected{ label.error() };
        }
        consume_tokens(tokens, { colon });
        auto next_stmt = parse_statement(tokens);
        if (next_stmt.has_value() == false)
        {
            return std::unexpected{ next_stmt.error() };
        }

        return std::make_unique<labelled_statement>(label.value(), std::move(next_stmt.value()));
    }
    if (next_token.type == goto_keyword)
    {
        consume_tokens(tokens, { goto_keyword });
        auto label = parse_identifier(tokens);
        if (label.has_value() == false)
        {
            return std::unexpected{ label.error() };
        }
        consume_tokens(tokens, { semicolon });
        return goto_statement{ label.value() };
    }

    auto e = parse_expression(tokens);
    if (e.has_value() == false)
    {
        return std::unexpected{ e.error() };
    }

    if (auto p = consume_tokens(tokens, { semicolon }); p.has_value())
    {
        return std::unexpected{ p.value() };
    }
    return statement{ std::move(e.value()) };
}

std::expected<type, parser_error> parse_type(const std::vector<lexer::token> &tokens)
{
    auto contains = [](const std::vector<lexer::token> &tokens, lexer::token_type type) {
        return std::ranges::find(tokens, type, &lexer::token::type) != tokens.end();
    };

    auto has_duplicates = [](const std::vector<lexer::token> &t) {
        auto local_tokens = t;
        std::ranges::sort(local_tokens, [](lexer::token &l, lexer::token &r) { return l.type < r.type; });

        return std::ranges::adjacent_find(local_tokens, [](lexer::token &l, lexer::token &r) {
                   return l.type == r.type;
               }) != local_tokens.end();
    };
    auto generate_error_msg = [](const std::vector<lexer::token> &tokens) {
        std::string comb;
        for (const auto &t : tokens)
        {
            comb = fmt::format("{} {}", comb, t.type);
        }
        return fmt::format("Unexpected type specifier combination found '{}'", comb);
    };

    if (tokens.empty())
    {
        auto msg = fmt::format("Missing type specifier");
        return std::unexpected{ parser_error{ msg } };
    }

    if (has_duplicates(tokens) ||
        (contains(tokens, lexer::token_type::signed_keyword) && contains(tokens, lexer::token_type::unsigned_keyword)))
    {
        return std::unexpected{ parser_error{ generate_error_msg(tokens) } };
    }

    if (tokens.size() == 1)
    {
        if (tokens.front().type == lexer::token_type::int_keyword ||
            tokens.front().type == lexer::token_type::signed_keyword)
        {
            return int_type{};
        }
        if (tokens.front().type == lexer::token_type::long_keyword)
        {
            return long_type{};
        }
        if (tokens.front().type == lexer::token_type::unsigned_keyword)
        {
            return unsigned_int_type{};
        }
        if (tokens.front().type == lexer::token_type::double_keyword)
        {
            return double_type{};
        }

        auto msg = fmt::format("Unexpected type specifier, expected 'int' or 'long' or 'double' but found '{}'",
                               tokens.front().type);
        return std::unexpected{ parser_error{ msg } };
    }

    // The double type specifier doesn't have any other "modifiers"
    // If there is more than one token, and one of them is the double.
    // Then, it's an error
    if (contains(tokens, lexer::token_type::double_keyword))
    {
        return std::unexpected{ parser_error{ generate_error_msg(tokens) } };
    }

    if (contains(tokens, lexer::token_type::unsigned_keyword) && contains(tokens, lexer::token_type::long_keyword))
    {
        return unsigned_long_type{};
    }
    if (contains(tokens, lexer::token_type::unsigned_keyword))
    {
        return unsigned_int_type{};
    }
    if (contains(tokens, lexer::token_type::long_keyword))
    {
        return long_type{};
    }

    return int_type{};
}

std::expected<type, parser_error> parse_type_specifier(tokens &tokens, lexer::token_type stop_token)
{
    std::vector<lexer::token> type_specifier;
    while (is_type_specifier(tokens.peek()))
    {
        auto t = tokens.get_next_token();
        if (t.has_value() == false)
        {
            return std::unexpected{ generate_unexpected_end_of_tokens(tokens) };
        }
        if (is_type_specifier(t.value()) == false)
        {
            auto msg = fmt::format("Unexpected token: Expected an type specifier but found {}", t->type);
            return std::unexpected{ parser_error{ msg } };
        }
        type_specifier.push_back(t.value());
    }

    return parse_type(type_specifier);
}

std::expected<binary_operator, parser_error> parse_binary_operator(tokens &tokens)
{
    auto t = tokens.get_next_token();
    if (t.has_value() == false)
    {
        return std::unexpected{ generate_unexpected_end_of_tokens(tokens) };
    }

    switch (t->type)
    {
        case lexer::token_type::bitwise_and_operator:
            return bitwise_and_operator{};
        case lexer::token_type::bitwise_or_operator:
            return bitwise_or_operator{};
        case lexer::token_type::bitwise_xor_operator:
            return bitwise_xor_operator{};
        case lexer::token_type::plus_operator:
            return plus_operator{};
        case lexer::token_type::negation_operator:
            return subtract_operator{};
        case lexer::token_type::multiplication_operator:
            return multiply_operator{};
        case lexer::token_type::division_operator:
            return divide_operator{};
        case lexer::token_type::remainder_operator:
            return remainder_operator{};
        case lexer::token_type::left_shift_operator:
            return left_shift_operator{};
        case lexer::token_type::right_shift_operator:
            return right_shift_operator{};
        case lexer::token_type::and_operator:
            return logical_and_operator{};
        case lexer::token_type::or_operator:
            return logical_or_operator{};
        case lexer::token_type::equals_operator:
            return equals_operator{};
        case lexer::token_type::not_equals_operator:
            return not_equals_operator{};
        case lexer::token_type::less_than_operator:
            return less_than_operator{};
        case lexer::token_type::less_than_or_equal_operator:
            return less_than_or_equal_operator{};
        case lexer::token_type::greater_than_operator:
            return greater_than_operator{};
        case lexer::token_type::greater_than_or_equal_operator:
            return greater_than_or_equal_operator{};
        case lexer::token_type::assignment_operator:
            return assignment_operator{};
        case lexer::token_type::compound_plus:
            return compound_plus_operator{};
        case lexer::token_type::compound_minus:
            return compound_subtract_operator{};
        case lexer::token_type::compound_multiplication:
            return compound_multiply_operator{};
        case lexer::token_type::compound_division:
            return compound_divide_operator{};
        case lexer::token_type::compound_remainder:
            return compound_remainder_operator{};
        case lexer::token_type::compound_bitwise_and:
            return compound_bitwise_and_operator{};
        case lexer::token_type::compound_bitwise_or:
            return compound_bitwise_or_operator{};
        case lexer::token_type::compound_bitwise_xor:
            return compound_bitwise_xor_operator{};
        case lexer::token_type::compound_left_shift:
            return compound_left_shift_operator{};
        case lexer::token_type::compound_right_shift:
            return compound_right_shift_operator{};

        default:
            auto msg = fmt::format("Expected Binary Operator but found '{}'", t->text);
            return std::unexpected{ parser_error{ msg } };
    }
}
std::expected<std::unique_ptr<unary_node>, parser_error> parse_unary_node(tokens &tokens)
{
    auto t = tokens.get_next_token();
    if (t.has_value() == false)
    {
        return std::unexpected{ generate_unexpected_end_of_tokens(tokens) };
    }
    unary_operator op;
    switch (t->type)
    {
        case lexer::token_type::bitwise_complement_operator:
            op = bitwise_complement_operator{};
            break;
        case lexer::token_type::negation_operator:
            op = negate_operator{};
            break;
        case lexer::token_type::not_operator:
            op = logical_not_operator{};
            break;
        case lexer::token_type::decrement_operator:
            op = prefix_decrement_operator{};
            break;
        case lexer::token_type::increment_operator:
            op = prefix_increment_operator{};
            break;

        default:
            auto msg = fmt::format("Parse failure at: {}. Expected Unary Operator '~' or '-' but found {}",
                                   t->loc,
                                   t->type);
            return std::unexpected{ parser_error{ msg } };
    }

    auto exp = parse_factor(tokens);
    if (exp.has_value() == false)
    {
        return std::unexpected{ exp.error() };
    }

    return std::make_unique<unary_node>(op, std::move(exp.value()));
}

std::expected<expression, parser_error> parse_factor(tokens &tokens)
{
    auto next_toke = tokens.peek();
    switch (next_toke.type)
    {
        case lexer::token_type::identifier:
        {
            if (tokens.peek(1).type == lexer::token_type::open_parenthesis)
            {
                auto f_call = parse_function_call(tokens);
                if (f_call.has_value() == false)
                {
                    return std::unexpected{ f_call.error() };
                }

                return expression{ std::move(f_call.value()) };
            }

            auto i = parse_identifier(tokens);
            if (i.has_value() == false)
            {
                return std::unexpected{ i.error() };
            }

            return var{ i.value() };
        }
        case lexer::token_type::floating_porint_constant:
        case lexer::token_type::int_constant:
        case lexer::token_type::long_constant:
        case lexer::token_type::unsigned_int_constant:
        case lexer::token_type::unsigned_long_constant:
        {
            auto e = parse_constant(tokens);
            if (e.has_value() == false)
            {
                return std::unexpected{ e.error() };
            }

            return std::move(e.value());
        }
        case lexer::token_type::bitwise_complement_operator:
        case lexer::token_type::decrement_operator:
        case lexer::token_type::increment_operator:
        case lexer::token_type::negation_operator:
        case lexer::token_type::not_operator:
        {
            auto u = parse_unary_node(tokens);
            if (u.has_value() == false)
            {
                return std::unexpected{ u.error() };
            }
            return std::move(u.value());
        }
        case lexer::token_type::bitwise_and_operator:
        {
            if (auto p = consume_tokens(tokens, { lexer::token_type::bitwise_and_operator }); p.has_value())
            {
                return std::unexpected{ p.value() };
            }
            auto exp = parse_factor(tokens);
            if (exp.has_value() == false)
            {
                return std::unexpected{ exp.error() };
            }

            return std::make_unique<address_of>(std::move(exp.value()));
        }
        case lexer::token_type::multiplication_operator:
        {
            if (auto p = consume_tokens(tokens, { lexer::token_type::multiplication_operator }); p.has_value())
            {
                return std::unexpected{ p.value() };
            }
            auto exp = parse_factor(tokens);
            if (exp.has_value() == false)
            {
                return std::unexpected{ exp.error() };
            }

            return std::make_unique<dereference>(std::move(exp.value()));
        }
        case lexer::token_type::open_parenthesis:
        {
            if (is_type_specifier(tokens.peek(1)))
            {
                return parse_cast_expression(tokens);
            }

            tokens.get_next_token_safe();
            auto inner_expr = parse_expression(tokens);
            auto n_t = tokens.get_next_token();
            if (n_t.has_value() == false)
            {
                return std::unexpected{ generate_unexpected_end_of_tokens(tokens) };
            }
            if (n_t->type != lexer::token_type::close_parenthesis)
            {
                auto msg = fmt::format("Parse failure at: {}. Expected return keyword found {}", n_t->loc, n_t->type);
                return std::unexpected{ parser_error{ msg } };
            }
            return inner_expr;
        }
        default:
        {
            auto msg = fmt::format("Parse failure at: Unexpected token '{}', expected an Expression", next_toke.text);
            return std::unexpected{ parser_error{ msg } };
        }
    }
}

std::expected<expression, parser_error> parse_expression(tokens &tokens, int32_t min_precedence)
{
    auto is_compound_assignment = [](const binary_operator &op) {
        return std::visit(visitor{ [](const compound_plus_operator &) { return true; },
                                   [](const compound_subtract_operator &) { return true; },
                                   [](const compound_multiply_operator &) { return true; },
                                   [](const compound_divide_operator &) { return true; },
                                   [](const compound_remainder_operator &) { return true; },
                                   [](const compound_bitwise_and_operator &) { return true; },
                                   [](const compound_bitwise_or_operator &) { return true; },
                                   [](const compound_bitwise_xor_operator &) { return true; },
                                   [](const compound_left_shift_operator &) { return true; },
                                   [](const compound_right_shift_operator &) { return true; },
                                   [](const auto &) { return false; } },
                          op);
    };

    auto compound_assignment_to_single = [](const binary_operator &op) {
        return std::visit(
          visitor{ [](const compound_plus_operator &) -> binary_operator { return plus_operator{}; },
                   [](const compound_subtract_operator &) -> binary_operator { return subtract_operator{}; },
                   [](const compound_multiply_operator &) -> binary_operator { return multiply_operator{}; },
                   [](const compound_divide_operator &) -> binary_operator { return divide_operator{}; },
                   [](const compound_remainder_operator &) -> binary_operator { return remainder_operator{}; },
                   [](const compound_bitwise_and_operator &) -> binary_operator { return bitwise_and_operator{}; },
                   [](const compound_bitwise_or_operator &) -> binary_operator { return bitwise_or_operator{}; },
                   [](const compound_bitwise_xor_operator &) -> binary_operator { return bitwise_xor_operator{}; },
                   [](const compound_left_shift_operator &) -> binary_operator { return left_shift_operator{}; },
                   [](const compound_right_shift_operator &) -> binary_operator { return right_shift_operator{}; },
                   [](const auto &) -> binary_operator { throw std::logic_error{ "Invalid Compound assignment" }; } },
          op);
    };
    auto is_right_associative = [](lexer::token_type type) {
        using enum lexer::token_type;
        return type == assignment_operator || type == compound_plus || type == compound_minus ||
               type == compound_multiplication || type == compound_division || type == compound_remainder ||
               type == compound_bitwise_and || type == compound_bitwise_or || type == compound_bitwise_xor ||
               type == compound_left_shift || type == compound_right_shift;
    };

    auto is_binary_operator = [](lexer::token_type type) {
        using enum lexer::token_type;
        return type == plus_operator || type == negation_operator || type == multiplication_operator ||
               type == division_operator || type == remainder_operator || type == bitwise_and_operator ||
               type == bitwise_or_operator || type == bitwise_xor_operator || type == left_shift_operator ||
               type == right_shift_operator || type == and_operator || type == or_operator || type == equals_operator ||
               type == not_equals_operator || type == less_than_operator || type == less_than_or_equal_operator ||
               type == greater_than_operator || type == greater_than_or_equal_operator || type == assignment_operator ||
               type == compound_plus || type == compound_minus || type == compound_multiplication ||
               type == compound_division || type == compound_remainder || type == compound_bitwise_and ||
               type == compound_bitwise_or || type == compound_bitwise_xor || type == compound_left_shift ||
               type == compound_right_shift || type == question_mark;
    };

    auto get_precedende = [](lexer::token_type type) {
        using enum lexer::token_type;
        switch (type)
        {
            case assignment_operator:
            case compound_plus:
            case compound_minus:
            case compound_multiplication:
            case compound_division:
            case compound_remainder:
            case compound_bitwise_and:
            case compound_bitwise_or:
            case compound_bitwise_xor:
            case compound_left_shift:
            case compound_right_shift:
                return 1;
            case question_mark:
                return 3;
            case or_operator:
                return 5;
            case and_operator:
                return 10;
            case bitwise_or_operator:
                return 15;
            case bitwise_xor_operator:
                return 20;
            case bitwise_and_operator:
                return 25;
            case equals_operator:
            case not_equals_operator:
                return 30;
            case less_than_operator:
            case less_than_or_equal_operator:
            case greater_than_operator:
            case greater_than_or_equal_operator:
                return 35;
            case left_shift_operator:
            case right_shift_operator:
                return 40;
            case plus_operator:
            case negation_operator:
                return 45;
            case multiplication_operator:
            case division_operator:
            case remainder_operator:
                return 50;
        }
        return 0;
    };

    auto left = parse_factor(tokens);
    if (left.has_value() == false)
    {
        return std::unexpected{ left.error() };
    }

    auto next_token = tokens.peek();
    while (is_binary_operator(next_token.type) && min_precedence <= get_precedende(next_token.type))
    {
        if (is_right_associative(next_token.type))
        {
            auto op = parse_binary_operator(tokens);
            if (op.has_value() == false)
            {
                return std::unexpected{ op.error() };
            }
            auto right = parse_expression(tokens, get_precedende(next_token.type));
            if (right.has_value() == false)
            {
                return std::unexpected{ right.error() };
            }

            if (is_compound_assignment(op.value()))
            {
                auto new_right = std::make_unique<binary_node>(compound_assignment_to_single(op.value()),
                                                               copy_expression(left.value()),
                                                               std::move(right.value()));
                left = std::make_unique<assignment_node>(std::move(left.value()), std::move(new_right));
            }
            else
            {
                left = std::make_unique<assignment_node>(std::move(left.value()), std::move(right.value()));
            }
        }
        else if (next_token.type == lexer::token_type::question_mark)
        {
            auto middle = parse_conditional(tokens);
            if (middle.has_value() == false)
            {
                return std::unexpected{ middle.error() };
            }
            auto right = parse_expression(tokens, get_precedende(next_token.type));
            if (right.has_value() == false)
            {
                return std::unexpected{ right.error() };
            }
            left = std::make_unique<conditional_node>(std::move(left.value()),
                                                      std::move(middle.value()),
                                                      std::move(right.value()));
        }
        else
        {
            auto op = parse_binary_operator(tokens);
            if (op.has_value() == false)
            {
                return std::unexpected{ op.error() };
            }

            auto right = parse_expression(tokens, get_precedende(next_token.type) + 1);
            if (right.has_value() == false)
            {
                return std::unexpected{ right.error() };
            }

            left = std::make_unique<binary_node>(op.value(), std::move(left.value()), std::move(right.value()));
        }

        next_token = tokens.peek();
    }

    if (next_token.type == lexer::token_type::decrement_operator ||
        next_token.type == lexer::token_type::increment_operator)
    {
        auto op = next_token.type == lexer::token_type::increment_operator
                    ? unary_operator{ postfix_increment_operator{} }
                    : unary_operator{ postfix_decrement_operator{} };

        tokens.discard_token();
        return std::make_unique<unary_node>(op, std::move(left.value()));
    }
    return left;
}

std::expected<identifier, parser_error> parse_identifier(tokens &tokens)
{
    auto token = tokens.get_next_token();
    if (token.has_value() == false)
    {
        return std::unexpected{ generate_unexpected_end_of_tokens(tokens) };
    }

    if (token->type != lexer::token_type::identifier)
    {
        auto msg = fmt::format("Parse failure at: {}. Expected Identifier found {}", token->loc, token->type);
        return std::unexpected{ parser_error{ msg } };
    }

    identifier c;
    c.name = token->text;
    return c;
}

std::expected<constant, parser_error> parse_constant(tokens &tokens)
{
    auto token = tokens.get_next_token();
    if (token.has_value() == false)
    {
        return std::unexpected{ generate_unexpected_end_of_tokens(tokens) };
    }

    if (token->type == lexer::token_type::int_constant || token->type == lexer::token_type::long_constant)
    {
        using fast_float::from_chars;
        int64_t value = 0;
        if (auto [ptr, ec] = from_chars(token->text.data(), token->text.data() + token->text.size(), value, 10);
            ec != std::errc{})
        {
            if (ec == std::errc::result_out_of_range)
            {
                auto msg = fmt::format("Parse failure at: {}. Constant {} doesn't fit in int or long",
                                       token->loc,
                                       token->text);
                return std::unexpected{ parser_error{ msg } };
            }

            auto msg = fmt::format(
              "Internal Error at {}. Expect constant {}, but failed to parse it as decimal constant",
              token->loc,
              token->text);
            return std::unexpected{ parser_error{ msg } };
        }

        if (token->type == lexer::token_type::int_constant && value <= std::numeric_limits<int32_t>::max())
        {
            return wccff::constant(int_constant{ static_cast<int32_t>(value) });
        }

        return wccff::constant(long_constant{ value });
    }

    if (token->type == lexer::token_type::unsigned_int_constant ||
        token->type == lexer::token_type::unsigned_long_constant)
    {
        using fast_float::from_chars;
        uint64_t value = 0;
        if (auto [ptr, ec] = from_chars(token->text.data(), token->text.data() + token->text.size(), value, 10);
            ec != std::errc{})
        {
            if (ec == std::errc::result_out_of_range)
            {
                auto msg = fmt::format("Parse failure at: {}. Constant {} doesn't fit in int or long",
                                       token->loc,
                                       token->text);
                return std::unexpected{ parser_error{ msg } };
            }

            auto msg = fmt::format(
              "Internal Error at {}. Expect constant {}, but failed to parse it as decimal constant",
              token->loc,
              token->text);
            return std::unexpected{ parser_error{ msg } };
        }

        if (token->type == lexer::token_type::unsigned_int_constant && value <= std::numeric_limits<uint32_t>::max())
        {
            return wccff::constant(unsigned_int_constant{ static_cast<uint32_t>(value) });
        }

        return wccff::constant(unsigned_long_constant{ value });
    }

    if (token->type == lexer::token_type::floating_porint_constant)
    {
        using fast_float::from_chars;
        double value = 0;
        if (auto [ptr, ec] = from_chars(token->text.data(), token->text.data() + token->text.size(), value);
            ec != std::errc{})
        {
            if (ec == std::errc::invalid_argument)
            {
                auto msg = fmt::format(
                  "Internal Error at {}. Expect constant {}, but failed to parse it as decimal constant",
                  token->loc,
                  token->text);
                return std::unexpected{ parser_error{ msg } };
            }
            // Don't handle the out-of-range error.
            // from_chars will round the value to zero, or to infinity depending on if it's too small or too large.
        }
        return wccff::constant(double_constant{ value });
    }
    auto msg = fmt::format("Parse failure at: {}. Expected Constant but found {}", token->loc, token->type);
    return std::unexpected{ parser_error{ msg } };
}

std::expected<variable_declaration, parser_error> parse_variable_declaration(tokens &tokens,
                                                                             wccff::identifier name,
                                                                             const wccff::type &t,
                                                                             storage_class storage)
{
    auto next_token = tokens.get_next_token();
    if (next_token.has_value() == false)
    {
        return std::unexpected{ generate_unexpected_end_of_tokens(tokens) };
    }

    if (next_token->type == lexer::token_type::semicolon)
    {
        return variable_declaration{ std::move(name), std::nullopt, copy_type(t), storage };
    }

    if (next_token->type == lexer::token_type::assignment_operator)
    {
        auto init = parse_expression(tokens);
        if (init.has_value() == false)
        {
            return std::unexpected{ init.error() };
        }

        auto s = parse_semicolon(tokens);
        if (s.has_value())
        {
            return std::unexpected{ s.value() };
        }

        return variable_declaration{ std::move(name), std::optional{ std::move(init.value()) }, copy_type(t), storage };
    }

    auto msg = fmt::format("Parse failure at: {}. Expected '=' or ';' but found {}", next_token->loc, next_token->type);
    return std::unexpected{ parser_error{ msg } };
}

std::expected<std::unique_ptr<while_statement>, parser_error> parse_while_statement(tokens &tokens)
{
    using enum lexer::token_type;
    if (auto p = consume_tokens(tokens, { while_keyword, open_parenthesis }); p.has_value())
    {
        return std::unexpected{ p.value() };
    }

    auto condition = parse_expression(tokens);
    if (condition.has_value() == false)
    {
        return std::unexpected{ condition.error() };
    }

    if (auto p = consume_tokens(tokens, { close_parenthesis }); p.has_value())
    {
        return std::unexpected{ p.value() };
    }

    auto stmt = parse_statement(tokens);
    if (stmt.has_value() == false)
    {
        return std::unexpected{ stmt.error() };
    }

    return std::make_unique<while_statement>(std::move(condition.value()), std::move(stmt.value()));
}

std::expected<program, parser_error> parse(tokens &tokens)
{
    auto p = parse_program(tokens);
    if (p.has_value() == false)
    {
        return std::unexpected{ p.error() };
    }

    if (tokens.remaining_tokens() != 0)
    {
        // There are more tokens at the end of the program.
        // Which is invalid
        return std::unexpected{ parser_error{ "Unexpected tokens at the end of the input" } };
    }
    return p;
}

std::string pretty_print(const std::unique_ptr<address_of> &node, int32_t ident)
{
    return wccff::format_indented(ident, "AddrOf({})", pretty_print(node->exp));
}

std::string pretty_print(const block &node, int32_t ident)
{
    std::string output;
    for (const auto &item : node.items)
    {
        output += pretty_print(item, ident);
        output += "\n";
    }
    if (node.items.empty() == false)
    {
        output.erase(output.size() - 1);
    }

    return output;
}
std::string pretty_print(const block_item &node, int32_t ident)
{
    return std::visit(visitor{
                        [ident](const declaration &n) { return pretty_print(n, ident); },
                        [ident](const statement &n) { return pretty_print(n, ident); },
                        [ident](const std::monostate &) { return wccff::format_indented(ident, "EMPTY Block item"); },
                      },
                      node);
}
std::string pretty_print(const break_statement &node, int32_t ident)
{
    return wccff::format_indented(ident, "Break({})", pretty_print(node.label));
}
std::string pretty_print(const constant &node, int32_t ident)
{
    return std::visit(visitor{
                        [ident](const double_constant &n) { return pretty_print(n, ident); },
                        [ident](const int_constant &n) { return pretty_print(n, ident); },
                        [ident](const long_constant &n) { return pretty_print(n, ident); },
                        [ident](const unsigned_int_constant &n) { return pretty_print(n, ident); },
                        [ident](const unsigned_long_constant &n) { return pretty_print(n, ident); },
                      },
                      node);
}
std::string pretty_print(const continue_statement &node, int32_t ident)
{
    return wccff::format_indented(ident, "Continue({})", pretty_print(node.label));
}
std::string pretty_print(const declaration &node, int32_t ident)
{
    return std::visit(visitor{
                        [ident](const function_declaration &n) { return pretty_print(n, ident); },
                        [ident](const variable_declaration &n) { return pretty_print(n, ident); },
                      },
                      node);
}
std::string pretty_print(const std::unique_ptr<dereference> &node, int32_t ident)
{
    return wccff::format_indented(ident, "Dereference({})", pretty_print(node->exp));
}

std::string pretty_print(const double_constant &node, int32_t ident)
{
    return wccff::format_indented(ident, "DoubleConstant({})", node.value);
}

std::string pretty_print(const expression &node, int32_t ident)
{
    return std::visit(wccff::visitor{
                        [ident](const var &n) { return pretty_print(n, ident); },
                        [ident](const constant &n) { return pretty_print(n, ident); },
                        [ident](const std::unique_ptr<address_of> &n) { return pretty_print(n, ident); },
                        [ident](const std::unique_ptr<dereference> &n) { return pretty_print(n, ident); },
                        [ident](const std::unique_ptr<unary_node> &n) { return pretty_print(n, ident); },
                        [ident](const std::unique_ptr<binary_node> &n) { return pretty_print(n, ident); },
                        [ident](const std::unique_ptr<cast_expression> &n) { return pretty_print(n, ident); },
                        [ident](const std::unique_ptr<assignment_node> &n) { return pretty_print(n, ident); },
                        [ident](const std::unique_ptr<conditional_node> &n) { return pretty_print(n, ident); },
                        [ident](const std::unique_ptr<function_call> &n) { return pretty_print(n, ident); },
                      },
                      node);
}
std::string pretty_print(const for_init &node, int32_t ident)
{
    return std::visit(wccff::visitor{
                        [ident](const init_declaration &n) { return pretty_print(n.decl, ident); },
                        [ident](const init_expression &n) { return pretty_print(n.expression, ident); },
                      },
                      node);
}

std::string pretty_print(const function &node, int32_t ident)
{
    auto prefix = wccff::format_indented(ident, "Function({}", pretty_print(node.function_name));
    auto left = wccff::format_indented(0, "{}", pretty_print(node.body, ident + 9));
    auto sufix = wccff::format_indented(ident, ")");

    return fmt::format("{}\n{}\n{}", prefix, left, sufix);
}

std::string pretty_print(const function_declaration &node, int32_t ident)
{
    auto prefix = wccff::format_indented(ident, "FunctionDecl({}", pretty_print(node.name));
    auto storage = pretty_print(node.storage_class, ident + 13);
    auto type = pretty_print(node.f_type, ident + 13);
    auto params = wccff::format_indented(ident + 13, "(");
    for (const auto &p : node.arguments)
    {
        if (p == identifier{ "NOT.VALID" })
        {
            // This is horible hack... When there are not arguments, the type of said arguments is void.
            // Since, we don't have the type here, just check that the identifier is NOT.VALID
            // If it is, then just don't print it.
            // However, still print the ',' to keep the code the same after.
            params += ", ";
            continue;
        }
        params += pretty_print(p, 0);
        params += ", ";
    }

    if (node.arguments.empty() == false)
    {
        params.erase(params.size() - 2, 2);
    }
    params += ')';

    std::string body;
    if (node.body.has_value())
    {
        body = wccff::format_indented(ident, "{}", pretty_print(node.body.value(), ident + 13));
    }
    else
    {
        body = wccff::format_indented(ident + 13, "EMPTY BODY");
    }

    auto sufix = wccff::format_indented(ident, ")");

    return fmt::format("{}\n{}\n{}\n{}\n{}\n{}", prefix, storage, type, params, body, sufix);
}
std::string pretty_print(const goto_statement &node, int32_t ident)
{
    return wccff::format_indented(ident, "GoTo({})", pretty_print(node.label));
}
std::string pretty_print(const identifier &node, int32_t ident)
{
    return wccff::format_indented(ident, "{}", node.name);
}
std::string pretty_print(const int_constant &node, int32_t ident)
{
    return wccff::format_indented(ident, "IntConstant({})", node.value);
}
std::string pretty_print(const long_constant &node, int32_t ident)
{
    return wccff::format_indented(ident, "LongConstant({})", node.value);
}
std::string pretty_print(const program &node, int32_t ident)
{
    std::string output;
    for (const auto &f : node.f)
    {
        output += pretty_print(f, ident);
        output += '\n';
    }
    return output;
}

std::string pretty_print(const statement &node, int32_t ident)
{
    return std::visit(
      visitor{
        [ident](const return_node &n) { return pretty_print(n, ident); },
        [ident](const expression &n) { return pretty_print(n, ident); },
        [ident](const std::unique_ptr<if_node> &n) { return pretty_print(n, ident); },
        [ident](const std::unique_ptr<compound_statement> &n) { return pretty_print(n, ident); },
        [ident](const break_statement &n) { return pretty_print(n, ident); },
        [ident](const continue_statement &n) { return pretty_print(n, ident); },
        [ident](const goto_statement &n) { return pretty_print(n, ident); },
        [ident](const std::unique_ptr<while_statement> &n) { return pretty_print(n, ident); },
        [ident](const std::unique_ptr<do_while_statement> &n) { return pretty_print(n, ident); },
        [ident](const std::unique_ptr<for_statement> &n) { return pretty_print(n, ident); },
        [ident](const std::monostate &) { return wccff::format_indented(ident, "EMPTY STATEMENT\n"); },
        [ident](const std::unique_ptr<parser::labelled_statement> &n) { return pretty_print(n, ident); },
      },
      node);
}
std::string pretty_print(const storage_class &node, int32_t ident)
{
    auto prefix = wccff::format_indented(ident, "Storage(");
    switch (node)
    {
        case storage_class::extern_storage:
            return wccff::format_indented(ident, "Storage(extern)");
        case storage_class::static_storage:
            return wccff::format_indented(ident, "Storage(static)");
        case storage_class::no_storage:
            return wccff::format_indented(ident, "Storage(none)");
    }
    return wccff::format_indented(ident, "Storage(INVALID-ERROR)");
}

std::string pretty_print(const return_node &node, int32_t ident)
{
    auto prefix = wccff::format_indented(ident, "Return(");
    auto a = pretty_print(node.e, ident + 7);
    auto sufix = wccff::format_indented(ident, ")");
    return fmt::format("{}\n{}\n{}", prefix, a, sufix);
}

std::string pretty_print(const std::optional<expression> &node, int32_t ident)
{
    if (node.has_value())
    {
        return pretty_print(node.value(), ident);
    }
    return wccff::format_indented(ident, "EMPTY EXPRESSION");
}

std::string pretty_print(const std::optional<type> &node, int32_t ident)
{
    if (node.has_value())
    {
        return pretty_print(node.value(), ident);
    }
    return wccff::format_indented(ident, "EMPTY TYPE");
}

std::string pretty_print(const std::unique_ptr<assignment_node> &node, int32_t ident)
{
    auto prefix = wccff::format_indented(ident, "Assign(");
    auto left = wccff::format_indented(0, "{}", pretty_print(node->lhs));
    auto right = wccff::format_indented(0, "{}", pretty_print(node->rhs, ident + 7));
    auto sufix = wccff::format_indented(ident, ")");

    return fmt::format("{}{}\n{}\n{}", prefix, left, right, sufix);
}

std::string pretty_print(const std::unique_ptr<binary_node> &node, int32_t ident)
{
    auto prefix = wccff::format_indented(ident, "Binary({}", pretty_print(node->op, 0));
    auto left = wccff::format_indented(0, "{}", pretty_print(node->left, ident + 7));
    auto right = wccff::format_indented(0, "{}", pretty_print(node->right, ident + 7));
    auto sufix = wccff::format_indented(ident, ")");

    return fmt::format("{}\n{}\n{}\n{}", prefix, left, right, sufix);
}

std::string pretty_print(const std::unique_ptr<cast_expression> &node, int32_t ident)
{
    auto prefix = wccff::format_indented(ident, "Cast(");
    auto type_text = wccff::format_indented(ident + 6, "Target({})", pretty_print(node->target));
    auto exp_text = wccff::format_indented(0, "{}", pretty_print(node->exp, ident + 6));
    auto sufix = wccff::format_indented(ident, ")");
    return fmt::format("{}\n{}\n{}\n{}", prefix, type_text, exp_text, sufix);
}

std::string pretty_print(const std::unique_ptr<compound_statement> &node, int32_t ident)
{
    auto prefix = format_indented(ident, "Compound(");
    auto middle = format_indented(0, "{}", pretty_print(node->block, ident + 9));
    auto sufix = format_indented(ident, ")");

    return fmt::format("{}\n{}\n{}", prefix, middle, sufix);
}
std::string pretty_print(const std::unique_ptr<conditional_node> &node, int32_t ident)
{
    auto prefix = wccff::format_indented(ident, "Conditional({}", pretty_print(node->condition, 0));
    auto middle = wccff::format_indented(0, "{}", pretty_print(node->e1, ident + 7));
    auto right = wccff::format_indented(0, "{}", pretty_print(node->e2, ident + 7));
    auto sufix = wccff::format_indented(ident, ")");

    return fmt::format("{}\n{}\n{}\n{}", prefix, middle, right, sufix);
}
std::string pretty_print(const std::unique_ptr<do_while_statement> &node, int32_t ident)
{
    auto prefix = wccff::format_indented(ident, "Do While({}", pretty_print(node->label, 0));
    auto body = wccff::format_indented(0, "{}", pretty_print(node->body, ident + 9));
    auto condition = wccff::format_indented(0, "{}", pretty_print(node->condition, ident + 9));

    auto sufix = wccff::format_indented(ident, ")");

    return fmt::format("{}\n{}\n{}\n{}", prefix, body, condition, sufix);
}
std::string pretty_print(const std::unique_ptr<for_statement> &node, int32_t ident)
{
    auto prefix = wccff::format_indented(ident, "For Loop({}", pretty_print(node->label, 0));
    auto init = wccff::format_indented(0, "{}", pretty_print(node->init, ident + 9));
    auto condition = wccff::format_indented(0, "{}", pretty_print(node->condition, ident + 9));
    auto post = wccff::format_indented(0, "{}", pretty_print(node->post, ident + 9));
    auto body = wccff::format_indented(0, "{}", pretty_print(node->body, ident + 9));
    auto sufix = wccff::format_indented(ident, ")");

    return fmt::format("{}\n{}\n{}\n{}\n{}\n{}", prefix, init, condition, post, body, sufix);
}

std::string pretty_print(const std::unique_ptr<function_call> &node, int32_t ident)
{
    auto prefix = wccff::format_indented(ident, "Function Call({}", pretty_print(node->name, 0));
    std::string params;
    if (node->arguments.empty() == false)
    {
        for (const auto &a : node->arguments)
        {
            params += pretty_print(a, ident + 14);
            params += "\n";
        }
        params.erase(params.length() - 1);
    }
    auto sufix = wccff::format_indented(ident, ")");

    return fmt::format("{}\n{}\n{}", prefix, params, sufix);
}

std::string pretty_print(const std::unique_ptr<fun_type> &node, int32_t ident)
{
    auto prefix = wccff::format_indented(ident, "FunType(RetType({})", pretty_print(node->return_type, 0));
    std::string params = "Param(";
    if (node->params.empty())
    {
        params += "Void)";
    }
    else
    {
        for (const auto &p : node->params)
        {
            params += fmt::format("{},", pretty_print(p));
        }
        // The last char will be a comma ',' remove that comma.
        params[params.size() - 1] = ')';
    }

    auto params_final = wccff::format_indented(ident + 8, "{}", params);

    auto sufix = wccff::format_indented(ident, ")");
    return fmt::format("{}\n{}\n{}", prefix, params_final, sufix);
}

std::string pretty_print(const std::unique_ptr<if_node> &node, int32_t ident)
{
    auto prefix = wccff::format_indented(ident, "If({}", pretty_print(node->op, 0));
    auto then_stmt = wccff::format_indented(0, "{}", pretty_print(node->then_stmt, ident + 7));

    std::string else_stmt;
    if (node->else_stmt.has_value())
    {
        else_stmt = wccff::format_indented(ident, "Else\n");
        else_stmt += wccff::format_indented(0, "{}", pretty_print(node->else_stmt.value(), ident + 7));
    }
    auto sufix = wccff::format_indented(ident, ")");

    return fmt::format("{}\n{}\n{}\n{}", prefix, then_stmt, else_stmt, sufix);
}
std::string pretty_print(const std::unique_ptr<labelled_statement> &node, int32_t ident)
{
    auto prefix = wccff::format_indented(ident, "LabeledStatement({}", pretty_print(node->label, 0));
    auto stmt = pretty_print(node->body, ident + 17);
    auto sufix = wccff::format_indented(ident, ")");

    return fmt::format("{}\n{}\n{}", prefix, stmt, sufix);
}

std::string pretty_print(const std::unique_ptr<pointer> &node, int32_t ident)
{
    return wccff::format_indented(ident, "Ptr({})", pretty_print(node->referenced));
}

std::string pretty_print(const std::unique_ptr<unary_node> &node, int32_t ident)
{
    auto prefix = wccff::format_indented(ident, "Unary({}", pretty_print(node->op, 0));
    auto inner = wccff::format_indented(0, "{}", pretty_print(node->exp, ident + 6));
    auto sufix = wccff::format_indented(ident, ")");

    return fmt::format("{}\n{}\n{}", prefix, inner, sufix);
}

std::string pretty_print(const std::unique_ptr<while_statement> &node, int32_t ident)
{
    auto prefix = wccff::format_indented(ident, "While({}", pretty_print(node->label, 0));
    auto condition = wccff::format_indented(0, "{}", pretty_print(node->condition, ident + 6));
    auto body = wccff::format_indented(0, "{}", pretty_print(node->body, ident + 6));
    auto sufix = wccff::format_indented(ident, ")");

    return fmt::format("{}\n{}\n{}\n{}", prefix, condition, body, sufix);
}

std::string pretty_print(const type &node, int32_t ident)
{
    return std::visit(visitor{
                        [ident](const double_type) { return wccff::format_indented(ident, "Double"); },
                        [ident](const int_type) { return wccff::format_indented(ident, "Int"); },
                        [ident](const long_type) { return wccff::format_indented(ident, "Long"); },
                        [ident](const std::unique_ptr<fun_type> &node) { return pretty_print(node, ident); },
                        [ident](const std::unique_ptr<pointer> &node) { return pretty_print(node, ident); },
                        [ident](const unsigned_int_type) { return wccff::format_indented(ident, "UInt"); },
                        [ident](const unsigned_long_type) { return wccff::format_indented(ident, "ULong"); },
                        [ident](const void_type) { return wccff::format_indented(ident, "Void"); },
                      },
                      node);
}

std::string pretty_print(const unsigned_int_constant &node, int32_t ident)
{
    return wccff::format_indented(ident, "UIntConstant({})", node.value);
}

std::string pretty_print(const unsigned_long_constant &node, int32_t ident)
{
    return wccff::format_indented(ident, "ULongConstant({})", node.value);
}

std::string pretty_print(const var &node, int32_t ident)
{
    return wccff::format_indented(ident, "Var({})", pretty_print(node.name));
}

std::string pretty_print(const variable_declaration &node, int32_t ident)
{
    auto name = wccff::format_indented(0, "{}", pretty_print(node.name));
    auto storage = pretty_print(node.storage_class, ident + 8);
    std::string init;
    if (node.init.has_value())
    {
        init = pretty_print(node.init.value(), ident + 8);
    }
    else
    {
        init = format_indented(ident + 8, "NO_INIT");
    }
    auto type = format_indented(ident + 8, "Type({})", pretty_print(node.var_type));
    auto sufix = wccff::format_indented(ident, ")");

    return format_indented(ident, "DeclVar({}\n{}\n{}\n{}\n{}", name, storage, type, init, sufix);
}
} // namespace wccff::parser
