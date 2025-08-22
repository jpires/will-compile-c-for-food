#include "symbol_table.h"
#include <catch2/catch_test_macros.hpp>
#include <ctll/parser.hpp>
#include <string_view>

TEST_CASE("Symbol Table", "[symbol_table]")
{
    using namespace wccff;
    using wccff::identifier;

    wccff::symbol_table::symbol_table table;

    SECTION("local_attributes")
    {
        REQUIRE(table.get(identifier{ "foo" }).has_value() == false);

        symbol_table::local_attributes attrs;
        table.add(identifier{ "foo" }, int_type{}, attrs);

        auto s = table.get(identifier{ "foo" });
        REQUIRE(s.has_value());
        REQUIRE(s.value().name == identifier{ "foo" });
        REQUIRE(std::holds_alternative<int_type>(s.value().type));
        REQUIRE(std::holds_alternative<symbol_table::local_attributes>(s.value().attrs));
    }

    SECTION("static_attributes")
    {
        REQUIRE(table.get(identifier{ "foo" }).has_value() == false);

        symbol_table::static_attributes attrs;
        attrs.is_global = true;
        attrs.init = symbol_table::no_initialiser{};
        table.add(identifier{ "foo" }, int_type{}, attrs);

        auto s = table.get(identifier{ "foo" });
        REQUIRE(s.has_value());
        REQUIRE(s.value().name == identifier{ "foo" });
        REQUIRE(std::holds_alternative<int_type>(s.value().type));
        REQUIRE(std::holds_alternative<symbol_table::static_attributes>(s.value().attrs));
        auto new_attrs = std::get<symbol_table::static_attributes>(s.value().attrs);
        REQUIRE(new_attrs.is_global);
        REQUIRE(std::holds_alternative<symbol_table::no_initialiser>(new_attrs.init));

        // Replace symbol with an initial value
        attrs.init = int_initial{ 42 };
        table.add(identifier{ "foo" }, int_type{}, attrs);
        auto s1 = table.get(identifier{ "foo" });
        REQUIRE(s1.has_value());
        REQUIRE(s1.value().name == identifier{ "foo" });
        REQUIRE(std::holds_alternative<int_type>(s1.value().type));
        REQUIRE(std::holds_alternative<symbol_table::static_attributes>(s1.value().attrs));
        auto new_attrs1 = std::get<symbol_table::static_attributes>(s1.value().attrs);
        REQUIRE(new_attrs1.is_global);
        REQUIRE(std::holds_alternative<initial>(new_attrs1.init));
        auto init_value = std::get<initial>(new_attrs1.init);
        REQUIRE(std::holds_alternative<int_initial>(init_value));
        REQUIRE(std::get<int_initial>(init_value).value == 42);
    }

    SECTION("func_attributes")
    {
        // Check that the identifier is not present in the table
        REQUIRE(table.get(identifier{ "foo" }).has_value() == false);

        // Insert a new foo symbol without a body
        symbol_table::func_attributes attrs{};
        attrs.is_defined = false;
        attrs.is_global = true;

        table.add(identifier{ "foo" }, int_type{}, attrs);

        auto s = table.get(identifier{ "foo" });
        REQUIRE(s.has_value());
        REQUIRE(s.value().name == identifier{ "foo" });
        REQUIRE(std::holds_alternative<int_type>(s.value().type));
        REQUIRE(std::holds_alternative<symbol_table::func_attributes>(s.value().attrs));
        auto new_attrs = std::get<symbol_table::func_attributes>(s.value().attrs);
        REQUIRE(new_attrs.is_global);
        REQUIRE(new_attrs.is_defined == false);

        // Replace the symbol foo with a body defined
        attrs.is_defined = true;
        table.add(identifier{ "foo" }, int_type{}, attrs);

        auto s1 = table.get(identifier{ "foo" });
        REQUIRE(s1.has_value());
        REQUIRE(s1.value().name == identifier{ "foo" });
        REQUIRE(std::holds_alternative<int_type>(s1.value().type));
        REQUIRE(std::holds_alternative<symbol_table::func_attributes>(s1.value().attrs));
        auto new_attrs1 = std::get<symbol_table::func_attributes>(s1.value().attrs);
        REQUIRE(new_attrs1.is_global);
        REQUIRE(new_attrs1.is_defined);
    }
}

TEST_CASE("Backend Symbol Table", "[symbol_table]")
{
    using namespace wccff;
    wccff::symbol_table::symbol_table frontend_table;
    wccff::symbol_table::backend_symbol_table backend_table;

    SECTION("Local Attributes")
    {
        REQUIRE(frontend_table.size() == 0);
        REQUIRE(backend_table.size() == 0);

        frontend_table.add(identifier{ "foo_int" }, int_type{}, symbol_table::local_attributes{});
        frontend_table.add(identifier{ "foo_long" }, long_type{}, symbol_table::local_attributes{});

        backend_table.build(frontend_table);

        REQUIRE(frontend_table.size() == 2);
        REQUIRE(backend_table.size() == 2);

        auto foo_int = backend_table.get(identifier{ "foo_int" });
        REQUIRE(foo_int.has_value());
        REQUIRE(std::holds_alternative<symbol_table::obj_entry>(foo_int.value()));
        auto foo_int_obj_entry = std::get<symbol_table::obj_entry>(foo_int.value());
        REQUIRE(foo_int_obj_entry.is_static == false);
        REQUIRE(std::holds_alternative<long_word>(foo_int_obj_entry.asm_type));

        auto foo_long = backend_table.get(identifier{ "foo_long" });
        REQUIRE(foo_long.has_value());
        REQUIRE(std::holds_alternative<symbol_table::obj_entry>(foo_long.value()));
        auto foo_long_obj_entry = std::get<symbol_table::obj_entry>(foo_long.value());
        REQUIRE(foo_long_obj_entry.is_static == false);
        REQUIRE(std::holds_alternative<quad_word>(foo_long_obj_entry.asm_type));
    }

    SECTION("Static Attributes")
    {
        REQUIRE(frontend_table.size() == 0);
        REQUIRE(backend_table.size() == 0);

        frontend_table.add(identifier{ "foo_int" }, int_type{}, symbol_table::static_attributes{});
        frontend_table.add(identifier{ "foo_long" }, long_type{}, symbol_table::static_attributes{});

        backend_table.build(frontend_table);

        REQUIRE(frontend_table.size() == 2);
        REQUIRE(backend_table.size() == 2);

        auto foo_int = backend_table.get(identifier{ "foo_int" });
        REQUIRE(foo_int.has_value());
        REQUIRE(std::holds_alternative<symbol_table::obj_entry>(foo_int.value()));
        auto foo_int_obj_entry = std::get<symbol_table::obj_entry>(foo_int.value());
        REQUIRE(foo_int_obj_entry.is_static);
        REQUIRE(std::holds_alternative<long_word>(foo_int_obj_entry.asm_type));

        auto foo_long = backend_table.get(identifier{ "foo_long" });
        REQUIRE(foo_long.has_value());
        REQUIRE(std::holds_alternative<symbol_table::obj_entry>(foo_long.value()));
        auto foo_long_obj_entry = std::get<symbol_table::obj_entry>(foo_long.value());
        REQUIRE(foo_long_obj_entry.is_static);
        REQUIRE(std::holds_alternative<quad_word>(foo_long_obj_entry.asm_type));
    }

    SECTION("Func Attributes")
    {
        REQUIRE(frontend_table.size() == 0);
        REQUIRE(backend_table.size() == 0);
        auto f_type = type{ std::make_unique<fun_type>(std::vector<type>{}, long_type{}) };
        frontend_table.add(identifier{ "foo_defined" }, f_type, symbol_table::func_attributes{ .is_defined = true });
        frontend_table.add(identifier{ "foo_not_defined" },
                           f_type,
                           symbol_table::func_attributes{ .is_defined = false });

        backend_table.build(frontend_table);

        REQUIRE(frontend_table.size() == 2);
        REQUIRE(backend_table.size() == 2);

        auto foo_defined = backend_table.get(identifier{ "foo_defined" });
        REQUIRE(foo_defined.has_value());
        REQUIRE(std::holds_alternative<symbol_table::fun_entry>(foo_defined.value()));
        auto foo_defined_fun_entry = std::get<symbol_table::fun_entry>(foo_defined.value());
        REQUIRE(foo_defined_fun_entry.is_defined);

        auto foo_not_defined = backend_table.get(identifier{ "foo_not_defined" });
        REQUIRE(foo_not_defined.has_value());
        REQUIRE(std::holds_alternative<symbol_table::fun_entry>(foo_not_defined.value()));
        auto foo_not_defined_fun_entry = std::get<symbol_table::fun_entry>(foo_not_defined.value());
        REQUIRE(foo_not_defined_fun_entry.is_defined == false);
    }
}
