#include "../assembly_generation.h"
#include "../parser.h"
#include "../tacky.h"
#include <ApprovalTests.hpp>
#include <catch2/catch_test_macros.hpp>
#include <ctll/parser.hpp>

TEST_CASE("Binary Operations", "[assembly_generation]")
{
    using wccff::identifier;
    using wccff::assembly_generation::pretty_print;
    auto directoryDisposer = ApprovalTests::Approvals::useApprovalsSubdirectory("assembly_generation_results");

    wccff::tacky::var double_var{ "double_var" };
    wccff::tacky::var int_var{ "int_var" };
    wccff::tacky::var long_var{ "long_var" };
    wccff::tacky::var uint_var{ "unsigned_int_var" };
    wccff::tacky::var ulong_var{ "unsigned_long_var" };

    wccff::symbol_table::symbol_table table;
    table.add(identifier{ "tacky-1" }, wccff::int_type{}, wccff::symbol_table::local_attributes{});
    table.add(identifier{ int_var.id.name }, wccff::int_type{}, wccff::symbol_table::local_attributes{});
    table.add(identifier{ long_var.id.name }, wccff::long_type{}, wccff::symbol_table::local_attributes{});
    table.add(identifier{ uint_var.id.name }, wccff::unsigned_int_type{}, wccff::symbol_table::local_attributes{});
    table.add(identifier{ ulong_var.id.name }, wccff::unsigned_long_type{}, wccff::symbol_table::local_attributes{});
    table.add(identifier{ double_var.id.name }, wccff::double_type{}, wccff::symbol_table::local_attributes{});

    auto ams_process = wccff::assembly_generation::assembly_generation(table);
    std::string result;

    wccff::double_constant double_const_1{ 1.0 };
    wccff::double_constant double_const_2{ 2.0 };
    wccff::int_constant int_const_1{ 1 };
    wccff::int_constant int_const_2{ 2 };
    wccff::long_constant long_const_1{ 1 };
    wccff::long_constant long_const_2{ 2 };
    wccff::unsigned_int_constant uint_const_1{ 1 };
    wccff::unsigned_int_constant uint_const_2{ 2 };
    wccff::unsigned_long_constant ulong_const_1{ 1 };
    wccff::unsigned_long_constant ulong_const_2{ 2 };
    wccff::tacky::var dst{ "tacky-1" };

    result += "--op=binary_and; src1 = int_constant; src2 = int_constant; dst = var--\n";
    wccff::tacky::binary_statement stmt1{ wccff::tacky::binary_and_operator{}, int_const_1, int_const_2, dst };
    ams_process.process(stmt1);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result += "--op=binary_or; src1 = long_constant; src2 = long_constant; dst = var--\n";
    wccff::tacky::binary_statement stmt2{ wccff::tacky::binary_or_operator{}, long_const_1, long_const_2, dst };
    ams_process.process(stmt2);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result += "--op=binary_xor; src1 = unsigned_int_constant; src2 = unsigned_int_constant; dst = var--\n";
    wccff::tacky::binary_statement stmt3{ wccff::tacky::binary_xor_operator{}, uint_const_1, uint_const_2, dst };
    ams_process.process(stmt3);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result += "--op=plus_operator; src1 = unsigned_long_constant; src2 = unsigned_long_constant; dst = var--\n";
    wccff::tacky::binary_statement stmt4{ wccff::tacky::plus_operator{}, ulong_const_1, ulong_const_2, dst };
    ams_process.process(stmt4);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result += "--op=subtract_operator; src1 = int_var; src2 = int_constant; dst = var--\n";
    wccff::tacky::binary_statement stmt5{ wccff::tacky::subtract_operator{}, int_var, int_const_2, dst };
    ams_process.process(stmt5);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result += "--op=multiply_operator; src1 = long_var; src2 = long_constant; dst = var--\n";
    wccff::tacky::binary_statement stmt6{ wccff::tacky::multiply_operator{}, long_var, long_const_2, dst };
    ams_process.process(stmt6);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result += "--op=left_shift_operator; src1 = ulong_var; src2 = long_constant; dst = var--\n";
    wccff::tacky::binary_statement stmt7{ wccff::tacky::left_shift_operator{}, ulong_var, ulong_const_2, dst };
    ams_process.process(stmt7);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result += "--op=right_shift_operator; src1 = uint_constant; src2 = uint_var; dst = var--\n";
    wccff::tacky::binary_statement stmt8{ wccff::tacky::right_shift_operator{}, uint_const_1, uint_var, dst };
    ams_process.process(stmt8);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result += "--op=divide_operator; src1 = uint_constant; src2 = uint_var; dst = var--\n";
    wccff::tacky::binary_statement stmt9{ wccff::tacky::divide_operator{}, uint_const_1, uint_var, dst };
    ams_process.process(stmt9);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result += "--op=remainder_operator; src1 = uint_constant; src2 = uint_var; dst = var--\n";
    wccff::tacky::binary_statement stmt10{ wccff::tacky::remainder_operator{}, uint_const_1, uint_var, dst };
    ams_process.process(stmt10);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result += "--op=equal_operator; src1 = uint_constant; src2 = uint_var; dst = var--\n";
    wccff::tacky::binary_statement stmt11{ wccff::tacky::equal_operator{}, uint_const_1, uint_var, dst };
    ams_process.process(stmt11);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result += "--op=not_equal_operator; src1 = int_constant; src2 = int_var; dst = var--\n";
    wccff::tacky::binary_statement stmt12{ wccff::tacky::not_equal_operator{}, int_const_1, int_var, dst };
    ams_process.process(stmt12);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result += "--op=less_than_operator; src1 = long_constant; src2 = long_var; dst = var--\n";
    wccff::tacky::binary_statement stmt13{ wccff::tacky::less_than_operator{}, long_const_1, long_var, dst };
    ams_process.process(stmt13);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result += "--op=less_than_operator; src1 = ulong_constant; src2 = ulong_var; dst = var--\n";
    wccff::tacky::binary_statement stmt14{ wccff::tacky::less_than_operator{}, ulong_const_1, ulong_var, dst };
    ams_process.process(stmt14);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result += "--op=less_than_or_equal_operator; src1 = long_constant; src2 = long_var; dst = var--\n";
    wccff::tacky::binary_statement stmt15{ wccff::tacky::less_than_or_equal_operator{}, long_const_1, long_var, dst };
    ams_process.process(stmt15);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result += "--op=less_than_or_equal_operator; src1 = ulong_constant; src2 = ulong_var; dst = var--\n";
    wccff::tacky::binary_statement stmt16{ wccff::tacky::less_than_or_equal_operator{}, ulong_const_1, ulong_var, dst };
    ams_process.process(stmt16);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result += "--op=greater_than_operator; src1 = long_constant; src2 = long_var; dst = var--\n";
    wccff::tacky::binary_statement stmt17{ wccff::tacky::greater_than_operator{}, long_const_1, long_var, dst };
    ams_process.process(stmt17);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result += "--op=greater_than_operator; src1 = ulong_constant; src2 = ulong_var; dst = var--\n";
    wccff::tacky::binary_statement stmt18{ wccff::tacky::greater_than_operator{}, ulong_const_1, ulong_var, dst };
    ams_process.process(stmt18);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result += "--op=greater_than_or_equal_operator; src1 = long_constant; src2 = long_var; dst = var--\n";
    wccff::tacky::binary_statement stmt19{ wccff::tacky::greater_than_or_equal_operator{},
                                           long_const_1,
                                           long_var,
                                           dst };
    ams_process.process(stmt19);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result += "--op=greater_than_or_equal_operator; src1 = ulong_constant; src2 = ulong_var; dst = var--\n";
    wccff::tacky::binary_statement stmt20{ wccff::tacky::greater_than_or_equal_operator{},
                                           ulong_const_1,
                                           ulong_var,
                                           dst };
    ams_process.process(stmt20);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    // Double Operations
    result += "---Double Operations---\n";
    result += "--op=plus_operator; src1 = double_constant; src2 = double_var; dst = var--\n";
    wccff::tacky::binary_statement stmt21{ wccff::tacky::plus_operator{}, double_const_1, double_var, dst };
    ams_process.process(stmt21);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result += "--op=subtract_operator; src1 = double_var; src2 = double_constant; dst = var--\n";
    wccff::tacky::binary_statement stmt22{ wccff::tacky::subtract_operator{}, double_var, double_const_1, dst };
    ams_process.process(stmt22);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result += "--op=multiply_operator; src1 = double_var; src2 = double_constant; dst = var--\n";
    wccff::tacky::binary_statement stmt23{ wccff::tacky::multiply_operator{}, double_var, double_const_1, dst };
    ams_process.process(stmt23);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result += "--op=divide_operator; src1 = double_var; src2 = double_constant; dst = var--\n";
    wccff::tacky::binary_statement stmt24{ wccff::tacky::divide_operator{}, double_var, double_const_1, dst };
    ams_process.process(stmt24);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result += "--op=equal_operator; src1 = double_var; src2 = double_constant; dst = var--\n";
    wccff::tacky::binary_statement stmt25{ wccff::tacky::equal_operator{}, double_var, double_const_1, dst };
    ams_process.process(stmt25);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result += "--op=not_equal_operator; src1 = double_var; src2 = double_constant; dst = var--\n";
    wccff::tacky::binary_statement stmt26{ wccff::tacky::not_equal_operator{}, double_var, double_const_1, dst };
    ams_process.process(stmt26);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result += "--op=less_than_operator; src1 = double_var; src2 = double_constant; dst = var--\n";
    wccff::tacky::binary_statement stmt27{ wccff::tacky::less_than_operator{}, double_var, double_const_1, dst };
    ams_process.process(stmt27);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result += "--op=less_than_or_equal_operator; src1 = double_constant; src2 = double_constant; dst = var--\n";
    wccff::tacky::binary_statement stmt28{ wccff::tacky::less_than_or_equal_operator{},
                                           double_const_2,
                                           double_const_1,
                                           dst };
    ams_process.process(stmt28);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result += "--op=greater_than_operator; src1 = double_var; src2 = double_constant; dst = var--\n";
    wccff::tacky::binary_statement stmt29{ wccff::tacky::greater_than_operator{}, double_var, double_const_1, dst };
    ams_process.process(stmt29);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result += "--op=greater_than_or_equal_operator; src1 = double_constant; src2 = double_constant; dst = var--\n";
    wccff::tacky::binary_statement stmt30{ wccff::tacky::greater_than_or_equal_operator{},
                                           double_const_2,
                                           double_const_1,
                                           dst };
    ams_process.process(stmt30);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    ApprovalTests::Approvals::verify(result);
}

TEST_CASE("Double to Uint", "[assembly_generation]")
{
    using wccff::identifier;
    using wccff::assembly_generation::pretty_print;
    auto directoryDisposer = ApprovalTests::Approvals::useApprovalsSubdirectory("assembly_generation_results");

    wccff::tacky::var double_var{ "double_var" };
    wccff::tacky::var int_var{ "int_var" };
    wccff::tacky::var long_var{ "long_var" };
    wccff::tacky::var uint_var{ "unsigned_int_var" };
    wccff::tacky::var ulong_var{ "unsigned_long_var" };

    wccff::symbol_table::symbol_table table;
    table.add(identifier{ "tacky-1" }, wccff::int_type{}, wccff::symbol_table::local_attributes{});
    table.add(identifier{ int_var.id.name }, wccff::int_type{}, wccff::symbol_table::local_attributes{});
    table.add(identifier{ long_var.id.name }, wccff::long_type{}, wccff::symbol_table::local_attributes{});
    table.add(identifier{ uint_var.id.name }, wccff::unsigned_int_type{}, wccff::symbol_table::local_attributes{});
    table.add(identifier{ ulong_var.id.name }, wccff::unsigned_long_type{}, wccff::symbol_table::local_attributes{});
    table.add(identifier{ double_var.id.name }, wccff::double_type{}, wccff::symbol_table::local_attributes{});

    auto ams_process = wccff::assembly_generation::assembly_generation(table);
    std::string result;

    wccff::double_constant double_const{ 1.0 };
    wccff::tacky::var dst{ "tacky-1" };

    result += "--src = double_constant; dst = uint_var--\n";
    wccff::tacky::double_to_uint stmt01{ double_const, uint_var };
    ams_process.process(stmt01);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result += "--src = double_var; dst = uint_var--\n";
    wccff::tacky::double_to_uint stmt02{ double_var, uint_var };
    ams_process.process(stmt02);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result += "--src = double_constant; dst = ulong_var--\n";
    wccff::tacky::double_to_uint stmt03{ double_const, ulong_var };
    ams_process.process(stmt03);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result += "--src = double_var; dst = ulong_var--\n";
    wccff::tacky::double_to_uint stmt04{ double_var, ulong_var };
    ams_process.process(stmt04);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    ApprovalTests::Approvals::verify(result);
}

TEST_CASE("Function Call", "[assembly_generation]")
{
    using wccff::identifier;
    using wccff::assembly_generation::pretty_print;
    auto directoryDisposer = ApprovalTests::Approvals::useApprovalsSubdirectory("assembly_generation_results");

    wccff::tacky::var double_var{ "double_var" };
    wccff::tacky::var int_var{ "int_var" };
    wccff::tacky::var long_var{ "long_var" };
    wccff::tacky::var uint_var{ "unsigned_int_var" };
    wccff::tacky::var ulong_var{ "unsigned_long_var" };

    wccff::symbol_table::symbol_table table;
    table.add(identifier{ "tacky-1" }, wccff::int_type{}, wccff::symbol_table::local_attributes{});
    table.add(identifier{ int_var.id.name }, wccff::int_type{}, wccff::symbol_table::local_attributes{});
    table.add(identifier{ long_var.id.name }, wccff::long_type{}, wccff::symbol_table::local_attributes{});
    table.add(identifier{ uint_var.id.name }, wccff::unsigned_int_type{}, wccff::symbol_table::local_attributes{});
    table.add(identifier{ ulong_var.id.name }, wccff::unsigned_long_type{}, wccff::symbol_table::local_attributes{});
    table.add(identifier{ double_var.id.name }, wccff::double_type{}, wccff::symbol_table::local_attributes{});

    auto ams_process = wccff::assembly_generation::assembly_generation(table);
    std::string result;

    wccff::double_constant double_const_1{ 1.0 };
    wccff::double_constant double_const_2{ 2.0 };
    wccff::int_constant int_const_1{ 1 };
    wccff::int_constant int_const_2{ 2 };
    wccff::long_constant long_const_1{ 1 };
    wccff::long_constant long_const_2{ 2 };
    wccff::unsigned_int_constant uint_const_1{ 1 };
    wccff::unsigned_int_constant uint_const_2{ 2 };
    wccff::unsigned_long_constant ulong_const_1{ 1 };
    wccff::unsigned_long_constant ulong_const_2{ 2 };
    wccff::tacky::var dst{ "tacky-1" };

    identifier no_params{ "no_params" };
    identifier one_param{ "one_param" };
    identifier six_params{ "six_params" };
    identifier seven_params{ "seven_params" };
    identifier eight_params{ "eight_params" };
    identifier nine_params{ "nine_params" };

    result += "----fun_name = no_params; params = NONE; dst = tacky-1\n";
    wccff::tacky::fun_call stmt1{ no_params, {}, dst };
    ams_process.process(stmt1);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result += "----fun_name = one_param; params = int_const; dst = tacky-1\n";
    wccff::tacky::fun_call stmt2{ one_param, { int_const_1 }, dst };
    ams_process.process(stmt2);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result += "----fun_name = one_param; params = int_var; dst = tacky-1\n";
    wccff::tacky::fun_call stmt3{ one_param, { int_var }, dst };
    ams_process.process(stmt3);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result += "----fun_name = one_param; params = long_const; dst = tacky-1\n";
    wccff::tacky::fun_call stmt4{ one_param, { long_const_1 }, dst };
    ams_process.process(stmt4);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result += "----fun_name = one_param; params = long_var; dst = tacky-1\n";
    wccff::tacky::fun_call stmt5{ one_param, { long_var }, dst };
    ams_process.process(stmt5);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result += "----fun_name = one_param; params = uint_const; dst = tacky-1\n";
    wccff::tacky::fun_call stmt6{ one_param, { uint_const_1 }, dst };
    ams_process.process(stmt6);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result += "----fun_name = one_param; params = uint_var; dst = tacky-1\n";
    wccff::tacky::fun_call stmt7{ one_param, { uint_var }, dst };
    ams_process.process(stmt7);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result += "----fun_name = one_param; params = ulong_var; dst = tacky-1\n";
    wccff::tacky::fun_call stmt8{ one_param, { ulong_var }, dst };
    ams_process.process(stmt8);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result += "----fun_name = six_params; params = int_const, int_var, long_const, long_var, uint_const, uint_var; dst "
              "= tacky-1\n";
    wccff::tacky::fun_call stmt9{ six_params,
                                  {
                                    int_const_1,
                                    int_var,
                                    long_var,
                                    long_const_1,
                                    uint_const_1,
                                    uint_var,
                                  },
                                  dst };
    ams_process.process(stmt9);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result +=
      "----fun_name = seven_params; params = int_const, int_var, long_const, long_var, uint_const, uint_var; dst "
      "= tacky-1\n";
    wccff::tacky::fun_call stmt10{ seven_params,
                                   {
                                     int_const_1,
                                     int_var,
                                     long_const_1,
                                     long_var,
                                     uint_const_1,
                                     uint_var,
                                     ulong_const_1,
                                   },
                                   dst };
    ams_process.process(stmt10);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result +=
      "----fun_name = eight_params; params = int_const, int_var, long_const, long_var, uint_const, uint_var; dst "
      "= tacky-1\n";
    wccff::tacky::fun_call stmt11{ eight_params,
                                   {
                                     int_const_1,
                                     int_var,
                                     long_const_1,
                                     long_var,
                                     uint_const_1,
                                     uint_var,
                                     ulong_const_1,
                                     ulong_var,
                                   },
                                   dst };
    ams_process.process(stmt11);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result += "----fun_name = one_param; params = double_const; dst = tacky-1\n";
    wccff::tacky::fun_call stmt12{ one_param, { double_const_1 }, dst };
    ams_process.process(stmt12);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result += "----fun_name = one_param; params = double_var; dst = tacky-1\n";
    wccff::tacky::fun_call stmt13{ one_param, { double_var }, dst };
    ams_process.process(stmt13);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result += "----fun_name = eight_params; params = (double_const; dst = tacky-1\n";
    wccff::tacky::fun_call stmt14{ eight_params,
                                   {
                                     double_const_1,
                                     double_var,
                                     double_const_2,
                                     double_var,
                                     double_const_1,
                                     double_var,
                                     double_const_2,
                                     double_var,
                                   },
                                   dst };
    ams_process.process(stmt14);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result += "----fun_name = nine_params; params = (double_const; dst = tacky-1\n";
    wccff::tacky::fun_call stmt15{ nine_params,
                                   {
                                     double_const_1,
                                     double_var,
                                     double_const_2,
                                     double_var,
                                     double_const_1,
                                     double_var,
                                     double_const_2,
                                     double_var,
                                     double_var,
                                   },
                                   dst };
    ams_process.process(stmt15);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    ApprovalTests::Approvals::verify(result);
}

TEST_CASE("Function Definition", "[assembly_generation]")
{
    using wccff::identifier;
    using wccff::assembly_generation::pretty_print;
    using wccff::symbol_table::local_attributes;
    auto directoryDisposer = ApprovalTests::Approvals::useApprovalsSubdirectory("assembly_generation_results");

    identifier double_var_1{ "double_var_1" };
    identifier double_var_2{ "double_var_2" };
    identifier double_var_3{ "double_var_3" };
    identifier double_var_4{ "double_var_4" };
    identifier double_var_5{ "double_var_5" };
    identifier double_var_6{ "double_var_6" };
    identifier double_var_7{ "double_var_7" };
    identifier double_var_8{ "double_var_8" };
    identifier double_var_9{ "double_var_9" };
    identifier int_var_1{ "int_var_1" };
    identifier int_var_2{ "int_var_2" };
    identifier long_var_1{ "long_var_1" };
    identifier long_var_2{ "long_var_2" };
    identifier uint_var_1{ "unsigned_int_var_1" };
    identifier uint_var_2{ "unsigned_int_var_2" };
    identifier ulong_var_1{ "unsigned_long_var_1" };
    identifier ulong_var_2{ "unsigned_long_var_2" };

    wccff::symbol_table::symbol_table table;
    table.add(identifier{ double_var_1.name }, wccff::double_type{}, local_attributes{});
    table.add(identifier{ double_var_2.name }, wccff::double_type{}, local_attributes{});
    table.add(identifier{ double_var_3.name }, wccff::double_type{}, local_attributes{});
    table.add(identifier{ double_var_4.name }, wccff::double_type{}, local_attributes{});
    table.add(identifier{ double_var_5.name }, wccff::double_type{}, local_attributes{});
    table.add(identifier{ double_var_6.name }, wccff::double_type{}, local_attributes{});
    table.add(identifier{ double_var_7.name }, wccff::double_type{}, local_attributes{});
    table.add(identifier{ double_var_8.name }, wccff::double_type{}, local_attributes{});
    table.add(identifier{ double_var_9.name }, wccff::double_type{}, local_attributes{});
    table.add(identifier{ int_var_1.name }, wccff::int_type{}, local_attributes{});
    table.add(identifier{ int_var_2.name }, wccff::int_type{}, local_attributes{});
    table.add(identifier{ long_var_1.name }, wccff::long_type{}, local_attributes{});
    table.add(identifier{ long_var_2.name }, wccff::long_type{}, local_attributes{});
    table.add(identifier{ uint_var_1.name }, wccff::unsigned_int_type{}, local_attributes{});
    table.add(identifier{ uint_var_2.name }, wccff::unsigned_int_type{}, local_attributes{});
    table.add(identifier{ ulong_var_1.name }, wccff::unsigned_long_type{}, local_attributes{});
    table.add(identifier{ ulong_var_2.name }, wccff::unsigned_long_type{}, local_attributes{});

    auto ams_process = wccff::assembly_generation::assembly_generation(table);
    std::string result;

    wccff::double_constant double_const{ 1.0 };
    wccff::int_constant int_const{ 1 };
    wccff::long_constant long_const{ 1 };
    wccff::unsigned_int_constant uint_const{ 1 };
    wccff::unsigned_long_constant ulong_const{ 1 };
    wccff::tacky::var dst{ "tacky-1" };

    std::vector<wccff::tacky::instruction> double_body;
    double_body.emplace_back(wccff::tacky::return_statement{ double_const });
    std::vector<wccff::tacky::instruction> int_body;
    int_body.emplace_back(wccff::tacky::return_statement{ int_const });
    std::vector<wccff::tacky::instruction> long_body;
    long_body.emplace_back(wccff::tacky::return_statement{ long_const });

    identifier no_params{ "no_params" };
    identifier one_param{ "one_param" };
    identifier six_params{ "six_params" };
    identifier seven_params{ "seven_params" };
    identifier eight_params{ "eight_params" };
    identifier nine_params{ "nine_params" };

    result += "----fun_name = no_params; global = false; params = NONE; body = int_body\n";
    wccff::tacky::function_definition stmt1{ no_params, false, {}, int_body };
    result += pretty_print(ams_process.process(stmt1));

    result += "\n----fun_name = one_param; global = true; params = int_var_1; body = int_body\n";
    wccff::tacky::function_definition stmt2{ one_param, true, { int_var_1 }, int_body };
    result += pretty_print(ams_process.process(stmt2));

    result += "\n----fun_name = one_param; global = false; params = long_var_1; body = long_body\n";
    wccff::tacky::function_definition stmt3{ one_param, false, { long_var_1 }, long_body };
    result += pretty_print(ams_process.process(stmt3));

    result += "\n----fun_name = six_params; global = false; params = int_var_1, int_var_2, long_var_1, long_var_2, "
              "uint_var_1, uint_var_2; body = long_body\n";
    wccff::tacky::function_definition stmt4{ six_params,
                                             false,
                                             { int_var_1, int_var_2, long_var_1, long_var_2, uint_var_1, uint_var_2 },
                                             long_body };
    result += pretty_print(ams_process.process(stmt4));

    result += "\n----fun_name = seven_params; global = false; params = int_var_1, int_var_2, long_var_1, long_var_2, "
              "uint_var_1, uint_var_2, ulong_var_1; body = long_body\n";
    wccff::tacky::function_definition stmt5{
        seven_params,
        false,
        { int_var_1, int_var_2, long_var_1, long_var_2, uint_var_1, uint_var_2, ulong_var_1 },
        long_body
    };
    result += pretty_print(ams_process.process(stmt5));

    result += "----fun_name = no_params; global = false; params = NONE; body = double_body\n";
    wccff::tacky::function_definition stmt6{ no_params, false, {}, double_body };
    result += pretty_print(ams_process.process(stmt6));

    result += "----fun_name = eight_params; global = false; params = double_var_1, double_var_2, double_var_3, "
              "double_var_4, double_var_5, double_var_6, double_var_7, double_var_8; body = double_body\n";
    wccff::tacky::function_definition stmt7{ eight_params,
                                             false,
                                             { double_var_1,
                                               double_var_2,
                                               double_var_3,
                                               double_var_4,
                                               double_var_5,
                                               double_var_6,
                                               double_var_7,
                                               double_var_8 },
                                             double_body };
    result += pretty_print(ams_process.process(stmt7));

    result +=
      "----fun_name = nine_params; global = false; params = double_var_1, double_var_2, double_var_3, "
      "double_var_4, double_var_5, double_var_6, double_var_7, double_var_8, double_var_8; body = double_body\n";
    wccff::tacky::function_definition stmt8{ nine_params,
                                             false,
                                             {
                                               double_var_1,
                                               double_var_2,
                                               double_var_3,
                                               double_var_4,
                                               double_var_5,
                                               double_var_6,
                                               double_var_7,
                                               double_var_8,
                                               double_var_9,
                                             },
                                             double_body };
    result += pretty_print(ams_process.process(stmt8));

    ApprovalTests::Approvals::verify(result);
}

TEST_CASE("Others", "[assembly_generation]")
{
    using wccff::identifier;
    using wccff::assembly_generation::pretty_print;
    auto directoryDisposer = ApprovalTests::Approvals::useApprovalsSubdirectory("assembly_generation_results");

    wccff::tacky::var double_var{ "double_var" };
    wccff::tacky::var int_var{ "int_var" };
    wccff::tacky::var long_var{ "long_var" };
    wccff::tacky::var uint_var{ "unsigned_int_var" };
    wccff::tacky::var ulong_var{ "unsigned_long_var" };

    wccff::symbol_table::symbol_table table;
    table.add(identifier{ "tacky-1" }, wccff::int_type{}, wccff::symbol_table::local_attributes{});
    table.add(identifier{ int_var.id.name }, wccff::int_type{}, wccff::symbol_table::local_attributes{});
    table.add(identifier{ long_var.id.name }, wccff::long_type{}, wccff::symbol_table::local_attributes{});
    table.add(identifier{ uint_var.id.name }, wccff::unsigned_int_type{}, wccff::symbol_table::local_attributes{});
    table.add(identifier{ ulong_var.id.name }, wccff::unsigned_long_type{}, wccff::symbol_table::local_attributes{});
    table.add(identifier{ double_var.id.name }, wccff::double_type{}, wccff::symbol_table::local_attributes{});

    auto ams_process = wccff::assembly_generation::assembly_generation(table);
    std::string result;

    wccff::double_constant double_const{ 1.0 };
    wccff::int_constant int_const{ 1 };
    wccff::long_constant long_const{ 1 };
    wccff::unsigned_int_constant uint_const{ 1 };
    wccff::unsigned_long_constant ulong_const{ 1 };
    wccff::tacky::var dst{ "tacky-1" };

    identifier target{ "jump_target" };

    {
        result += "---Jump If Not Zero---\n";
        result += "--condition = double_constant; target = jump_target--\n";
        wccff::tacky::jump_if_not_zero_statement stmt01{ double_const, target };
        ams_process.process(stmt01);
        result += pretty_print(ams_process.get_instructions());
        ams_process.reset_instructions();

        result += "--condition = double_var; target = jump_target--\n";
        wccff::tacky::jump_if_not_zero_statement stmt02{ double_var, target };
        ams_process.process(stmt02);
        result += pretty_print(ams_process.get_instructions());
        ams_process.reset_instructions();

        result += "--condition = int_constant; target = jump_target--\n";
        wccff::tacky::jump_if_not_zero_statement stmt03{ int_const, target };
        ams_process.process(stmt03);
        result += pretty_print(ams_process.get_instructions());
        ams_process.reset_instructions();

        result += "--condition = int_var; target = jump_target--\n";
        wccff::tacky::jump_if_not_zero_statement stmt04{ int_var, target };
        ams_process.process(stmt04);
        result += pretty_print(ams_process.get_instructions());
        ams_process.reset_instructions();

        result += "--condition = long_constant; target = jump_target--\n";
        wccff::tacky::jump_if_not_zero_statement stmt05{ long_const, target };
        ams_process.process(stmt05);
        result += pretty_print(ams_process.get_instructions());
        ams_process.reset_instructions();

        result += "--condition = long_var; target = jump_target--\n";
        wccff::tacky::jump_if_not_zero_statement stmt06{ long_var, target };
        ams_process.process(stmt06);
        result += pretty_print(ams_process.get_instructions());
        ams_process.reset_instructions();

        result += "--condition = unsigned_int_constant; target = jump_target--\n";
        wccff::tacky::jump_if_not_zero_statement stmt07{ uint_const, target };
        ams_process.process(stmt07);
        result += pretty_print(ams_process.get_instructions());
        ams_process.reset_instructions();

        result += "--condition = unsigned_int_var; target = jump_target--\n";
        wccff::tacky::jump_if_not_zero_statement stmt08{ uint_var, target };
        ams_process.process(stmt08);
        result += pretty_print(ams_process.get_instructions());
        ams_process.reset_instructions();

        result += "--condition = unsigned_long_constant; target = jump_target--\n";
        wccff::tacky::jump_if_not_zero_statement stmt09{ ulong_const, target };
        ams_process.process(stmt09);
        result += pretty_print(ams_process.get_instructions());
        ams_process.reset_instructions();

        result += "--condition = unsigned_long_var; target = jump_target--\n";
        wccff::tacky::jump_if_not_zero_statement stmt10{ ulong_var, target };
        ams_process.process(stmt10);
        result += pretty_print(ams_process.get_instructions());
        ams_process.reset_instructions();
    }
    {
        result += "---Jump If Zero---\n";
        result += "--condition = double_constant; target = jump_target--\n";
        wccff::tacky::jump_if_zero_statement stmt01{ double_const, target };
        ams_process.process(stmt01);
        result += pretty_print(ams_process.get_instructions());
        ams_process.reset_instructions();

        result += "--condition = double_var; target = jump_target--\n";
        wccff::tacky::jump_if_zero_statement stmt02{ double_var, target };
        ams_process.process(stmt02);
        result += pretty_print(ams_process.get_instructions());
        ams_process.reset_instructions();

        result += "--condition = int_constant; target = jump_target--\n";
        wccff::tacky::jump_if_zero_statement stmt03{ int_const, target };
        ams_process.process(stmt03);
        result += pretty_print(ams_process.get_instructions());
        ams_process.reset_instructions();

        result += "--condition = int_var; target = jump_target--\n";
        wccff::tacky::jump_if_zero_statement stmt04{ int_var, target };
        ams_process.process(stmt04);
        result += pretty_print(ams_process.get_instructions());
        ams_process.reset_instructions();

        result += "--condition = long_constant; target = jump_target--\n";
        wccff::tacky::jump_if_zero_statement stmt05{ long_const, target };
        ams_process.process(stmt05);
        result += pretty_print(ams_process.get_instructions());
        ams_process.reset_instructions();

        result += "--condition = long_var; target = jump_target--\n";
        wccff::tacky::jump_if_zero_statement stmt06{ long_var, target };
        ams_process.process(stmt06);
        result += pretty_print(ams_process.get_instructions());
        ams_process.reset_instructions();

        result += "--condition = unsigned_int_constant; target = jump_target--\n";
        wccff::tacky::jump_if_zero_statement stmt07{ uint_const, target };
        ams_process.process(stmt07);
        result += pretty_print(ams_process.get_instructions());
        ams_process.reset_instructions();

        result += "--condition = unsigned_int_var; target = jump_target--\n";
        wccff::tacky::jump_if_zero_statement stmt08{ uint_var, target };
        ams_process.process(stmt08);
        result += pretty_print(ams_process.get_instructions());
        ams_process.reset_instructions();

        result += "--condition = unsigned_long_constant; target = jump_target--\n";
        wccff::tacky::jump_if_zero_statement stmt09{ ulong_const, target };
        ams_process.process(stmt09);
        result += pretty_print(ams_process.get_instructions());
        ams_process.reset_instructions();

        result += "--condition = unsigned_long_var; target = jump_target--\n";
        wccff::tacky::jump_if_zero_statement stmt10{ ulong_var, target };
        ams_process.process(stmt10);
        result += pretty_print(ams_process.get_instructions());
        ams_process.reset_instructions();
    }
    ApprovalTests::Approvals::verify(result);
}

TEST_CASE("Return Statement", "[assembly_generation]")
{
    using wccff::identifier;
    using wccff::assembly_generation::pretty_print;
    auto directoryDisposer = ApprovalTests::Approvals::useApprovalsSubdirectory("assembly_generation_results");

    wccff::tacky::var double_var{ "double_var" };
    wccff::tacky::var int_var{ "int_var" };
    wccff::tacky::var long_var{ "long_var" };
    wccff::tacky::var uint_var{ "unsigned_int_var" };
    wccff::tacky::var ulong_var{ "unsigned_long_var" };

    wccff::symbol_table::symbol_table table;
    table.add(identifier{ "tacky-1" }, wccff::int_type{}, wccff::symbol_table::local_attributes{});
    table.add(identifier{ int_var.id.name }, wccff::int_type{}, wccff::symbol_table::local_attributes{});
    table.add(identifier{ long_var.id.name }, wccff::long_type{}, wccff::symbol_table::local_attributes{});
    table.add(identifier{ uint_var.id.name }, wccff::unsigned_int_type{}, wccff::symbol_table::local_attributes{});
    table.add(identifier{ ulong_var.id.name }, wccff::unsigned_long_type{}, wccff::symbol_table::local_attributes{});
    table.add(identifier{ double_var.id.name }, wccff::double_type{}, wccff::symbol_table::local_attributes{});

    auto ams_process = wccff::assembly_generation::assembly_generation(table);
    std::string result;

    wccff::double_constant double_const{ 1.0 };
    wccff::int_constant int_const{ 1 };
    wccff::long_constant long_const{ 1 };
    wccff::unsigned_int_constant uint_const{ 1 };
    wccff::unsigned_long_constant ulong_const{ 1 };
    wccff::tacky::var dst{ "tacky-1" };

    result += "--src1 = double_const--\n";
    wccff::tacky::return_statement stmt01{ double_const };
    ams_process.process(stmt01);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result += "--src1 = double_var--\n";
    wccff::tacky::return_statement stmt02{ double_var };
    ams_process.process(stmt02);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result += "--src1 = int_const--\n";
    wccff::tacky::return_statement stmt03{ int_const };
    ams_process.process(stmt03);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result += "--src1 = int_var--\n";
    wccff::tacky::return_statement stmt04{ int_var };
    ams_process.process(stmt04);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result += "--src1 = long_const--\n";
    wccff::tacky::return_statement stmt05{ long_const };
    ams_process.process(stmt05);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result += "--src1 = long_var--\n";
    wccff::tacky::return_statement stmt06{ long_var };
    ams_process.process(stmt06);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result += "--src1 = uint_const--\n";
    wccff::tacky::return_statement stmt07{ uint_const };
    ams_process.process(stmt07);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result += "--src1 = uint_var--\n";
    wccff::tacky::return_statement stmt08{ uint_var };
    ams_process.process(stmt08);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result += "--src1 = ulong_const--\n";
    wccff::tacky::return_statement stmt09{ ulong_const };
    ams_process.process(stmt09);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result += "--src1 = ulong_var--\n";
    wccff::tacky::return_statement stmt10{ ulong_var };
    ams_process.process(stmt10);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    ApprovalTests::Approvals::verify(result);
}

TEST_CASE("Unary Operators", "[assembly_generation]")
{
    using wccff::identifier;
    using wccff::assembly_generation::pretty_print;
    auto directoryDisposer = ApprovalTests::Approvals::useApprovalsSubdirectory("assembly_generation_results");

    wccff::tacky::var double_var{ "double_var" };
    wccff::tacky::var int_var{ "int_var" };
    wccff::tacky::var long_var{ "long_var" };
    wccff::tacky::var uint_var{ "unsigned_int_var" };
    wccff::tacky::var ulong_var{ "unsigned_long_var" };

    wccff::symbol_table::symbol_table table;
    table.add(identifier{ "tacky-1" }, wccff::int_type{}, wccff::symbol_table::local_attributes{});
    table.add(identifier{ int_var.id.name }, wccff::int_type{}, wccff::symbol_table::local_attributes{});
    table.add(identifier{ long_var.id.name }, wccff::long_type{}, wccff::symbol_table::local_attributes{});
    table.add(identifier{ uint_var.id.name }, wccff::unsigned_int_type{}, wccff::symbol_table::local_attributes{});
    table.add(identifier{ ulong_var.id.name }, wccff::unsigned_long_type{}, wccff::symbol_table::local_attributes{});
    table.add(identifier{ double_var.id.name }, wccff::double_type{}, wccff::symbol_table::local_attributes{});

    auto ams_process = wccff::assembly_generation::assembly_generation(table);
    std::string result;

    wccff::double_constant double_const{ 1.0 };
    wccff::int_constant int_const{ 1 };
    wccff::long_constant long_const{ 1 };
    wccff::unsigned_int_constant uint_const{ 1 };
    wccff::unsigned_long_constant ulong_const{ 1 };
    wccff::tacky::var dst{ "tacky-1" };

    result += "--op=binary_complement_operator; src1 = int_constant; dst = var--\n";
    wccff::tacky::unary_statement stmt01{ wccff::tacky::binary_complement_operator{}, int_const, dst };
    ams_process.process(stmt01);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result += "--op=negate_operator; src1 = long_constant; dst = var--\n";
    wccff::tacky::unary_statement stmt02{ wccff::tacky::negate_operator{}, long_const, dst };
    ams_process.process(stmt02);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result += "--op=not_operator; src1 = unsigned_int_constant; dst = var--\n";
    wccff::tacky::unary_statement stmt03{ wccff::tacky::not_operator{}, uint_const, dst };
    ams_process.process(stmt03);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result += "--op=binary_complement_operator; src1 = unsigned_long_constant; dst = var--\n";
    wccff::tacky::unary_statement stmt04{ wccff::tacky::binary_complement_operator{}, ulong_const, dst };
    ams_process.process(stmt04);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result += "---Double Operation---\n";
    result += "--op=negate_operator; src1 = double_constant; dst = var--\n";
    wccff::tacky::unary_statement stmt05{ wccff::tacky::negate_operator{}, double_const, dst };
    ams_process.process(stmt05);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    result += "--op=negate_operator; src1 = double_constant; dst = var--\n";
    wccff::tacky::unary_statement stmt06{ wccff::tacky::not_operator{}, double_const, dst };
    ams_process.process(stmt06);
    result += pretty_print(ams_process.get_instructions());
    ams_process.reset_instructions();

    ApprovalTests::Approvals::verify(result);
}

TEST_CASE("helpers", "[assembly_generation]")
{
    SECTION("is_larger_immediate")
    {
        using wccff::assembly_generation::data;
        using wccff::assembly_generation::immediate;
        using wccff::assembly_generation::is_larger_immediate;
        using wccff::assembly_generation::pseudo;
        using wccff::assembly_generation::R10;
        using wccff::assembly_generation::stack;

        REQUIRE(is_larger_immediate(data{ "data" }) == false);
        REQUIRE(is_larger_immediate(R10{}) == false);
        REQUIRE(is_larger_immediate(pseudo{ "pseudo" }) == false);
        REQUIRE(is_larger_immediate(stack{ 16 }) == false);

        REQUIRE(is_larger_immediate(immediate{ 0 }) == false);
        REQUIRE(is_larger_immediate(immediate{ std::numeric_limits<int32_t>::max() }) == false);
        REQUIRE(is_larger_immediate(immediate{ std::numeric_limits<int32_t>::min() }) == false);

        REQUIRE(is_larger_immediate(immediate{ std::numeric_limits<int32_t>::max() + 1ll }));
        REQUIRE(is_larger_immediate(immediate{ std::numeric_limits<int32_t>::min() - 1ll }));
    }
}

TEST_CASE("fixing_up_instructions", "[assembly_generation]")
{
    using wccff::long_word;
    using wccff::quad_word;
    using wccff::assembly_generation::ax;
    using wccff::assembly_generation::XMM0;
    using wccff::assembly_generation::XMM1;

    using wccff::assembly_generation::cx;
    using wccff::assembly_generation::data;
    using wccff::assembly_generation::immediate;
    using wccff::assembly_generation::pretty_print;
    using wccff::assembly_generation::stack;

    auto directoryDisposer = ApprovalTests::Approvals::useApprovalsSubdirectory("assembly_generation_results");

    SECTION("add")
    {
        using wccff::assembly_generation::add;
        using wccff::assembly_generation::binary;
        using wccff::assembly_generation::fixing_up_instructions_binary;
        SECTION("No Fixing needed")
        {
            binary b{ .op = add{}, .src = immediate{ 42 }, .dst = cx{}, .type = long_word{} };
            REQUIRE(fixing_up_instructions_binary(b).has_value() == false);

            binary b1{ .op = add{}, .src = immediate{ 42 }, .dst = data{ "data" }, .type = quad_word{} };
            REQUIRE(fixing_up_instructions_binary(b1).has_value() == false);

            binary b2{ .op = add{}, .src = immediate{ 42 }, .dst = stack{ 16 }, .type = long_word{} };
            REQUIRE(fixing_up_instructions_binary(b2).has_value() == false);

            binary b3{ .op = add{}, .src = ax{}, .dst = cx{}, .type = long_word{} };
            REQUIRE(fixing_up_instructions_binary(b3).has_value() == false);

            binary b4{ .op = add{}, .src = ax{}, .dst = data{ "data" }, .type = quad_word{} };
            REQUIRE(fixing_up_instructions_binary(b4).has_value() == false);

            binary b5{ .op = add{}, .src = ax{}, .dst = stack{ 16 }, .type = long_word{} };
            REQUIRE(fixing_up_instructions_binary(b5).has_value() == false);

            binary b6{ .op = add{}, .src = data{ "data" }, .dst = cx{}, .type = quad_word{} };
            REQUIRE(fixing_up_instructions_binary(b6).has_value() == false);

            binary b7{ .op = add{}, .src = stack{ 16 }, .dst = cx{}, .type = long_word{} };
            REQUIRE(fixing_up_instructions_binary(b7).has_value() == false);
        }

        std::string result;
        result += "--src = stack; dst = stack; type = long_word--\n";
        binary b{ .op = add{}, .src = stack{ 8 }, .dst = stack{ 16 }, .type = long_word{} };
        auto b_result = fixing_up_instructions_binary(b);
        REQUIRE(b_result.has_value());
        result += pretty_print(b_result.value());

        result += "\n--src = stack; dst = data; type = long_word--\n";
        binary b1{ .op = add{}, .src = stack{ 8 }, .dst = data{ "foo_dst" }, .type = quad_word{} };
        auto b1_result = fixing_up_instructions_binary(b1);
        REQUIRE(b1_result.has_value());
        result += pretty_print(b1_result.value());

        result += "\n--src = data; dst = stack; type = long_word--\n";
        binary b2{ .op = add{}, .src = data{ "foo_src" }, .dst = stack{ 16 }, .type = long_word{} };
        auto b2_result = fixing_up_instructions_binary(b2);
        REQUIRE(b2_result.has_value());
        result += pretty_print(b2_result.value());

        result += "\n--src = data; dst = data; type = long_word--\n";
        binary b3{ .op = add{}, .src = data{ "foo_src" }, .dst = data{ "foo_dst" }, .type = quad_word{} };
        auto b3_result = fixing_up_instructions_binary(b3);
        REQUIRE(b3_result.has_value());
        result += pretty_print(b3_result.value());

        ApprovalTests::Approvals::verify(result);
    }

    SECTION("cmp")
    {
        using wccff::assembly_generation::cmp;
        using wccff::assembly_generation::fixing_up_instructions11;
        using wccff::assembly_generation::mul;

        SECTION("No Fixing needed")
        {
            cmp c1{ .lhs = immediate{ 42 }, .rhs = ax{}, .type = quad_word{} };
            REQUIRE(fixing_up_instructions11(c1).has_value() == false);

            cmp c2{ .lhs = immediate{ 42 }, .rhs = data{ "data_dst" }, .type = quad_word{} };
            REQUIRE(fixing_up_instructions11(c2).has_value() == false);

            cmp c3{ .lhs = immediate{ 42 }, .rhs = stack{ 16 }, .type = quad_word{} };
            REQUIRE(fixing_up_instructions11(c3).has_value() == false);

            cmp c4{ .lhs = cx{}, .rhs = ax{}, .type = quad_word{} };
            REQUIRE(fixing_up_instructions11(c4).has_value() == false);

            cmp c5{ .lhs = cx{}, .rhs = data{ "data_dst" }, .type = quad_word{} };
            REQUIRE(fixing_up_instructions11(c5).has_value() == false);

            cmp c6{ .lhs = cx{}, .rhs = stack{ 16 }, .type = quad_word{} };
            REQUIRE(fixing_up_instructions11(c6).has_value() == false);

            cmp c7{ .lhs = data{ "data_src" }, .rhs = ax{}, .type = quad_word{} };
            REQUIRE(fixing_up_instructions11(c7).has_value() == false);

            cmp c8{ .lhs = stack{ 8 }, .rhs = ax{}, .type = quad_word{} };
            REQUIRE(fixing_up_instructions11(c8).has_value() == false);

            cmp c9{ .lhs = XMM0{}, .rhs = XMM1{}, .type = wccff::double_asm{} };
            REQUIRE(fixing_up_instructions11(c9).has_value() == false);

            cmp c10{ .lhs = data{ "data_src" }, .rhs = XMM1{}, .type = wccff::double_asm{} };
            REQUIRE(fixing_up_instructions11(c10).has_value() == false);

            cmp c11{ .lhs = stack{ 8 }, .rhs = XMM1{}, .type = wccff::double_asm{} };
            REQUIRE(fixing_up_instructions11(c11).has_value() == false);
        }

        std::string result;
        result += "--lhs = stack; rhs = stack; type = long_word--\n";
        cmp c1{ .lhs = stack{ 8 }, .rhs = stack{ 16 }, .type = long_word{} };
        auto c1_result = fixing_up_instructions11(c1);
        REQUIRE(c1_result.has_value());
        result += pretty_print(c1_result.value());

        result += "\n--lhs = stack; rhs = data; type = quad_word--\n";
        cmp c2{ .lhs = stack{ 8 }, .rhs = data{ "data_rhs" }, .type = quad_word{} };
        auto c2_result = fixing_up_instructions11(c2);
        REQUIRE(c2_result.has_value());
        result += pretty_print(c2_result.value());

        result += "\n--lhs = data; rhs = stack; type = long_word--\n";
        cmp c3{ .lhs = data{ "data_lhs" }, .rhs = stack{ 16 }, .type = long_word{} };
        auto c3_result = fixing_up_instructions11(c3);
        REQUIRE(c3_result.has_value());
        result += pretty_print(c3_result.value());

        result += "\n--lhs = data; rhs = data; type = quad_word--\n";
        cmp c4{ .lhs = data{ "data_lhs" }, .rhs = data{ "data_rhs" }, .type = quad_word{} };
        auto c4_result = fixing_up_instructions11(c4);
        REQUIRE(c4_result.has_value());
        result += pretty_print(c4_result.value());

        result += "\n--lhs = immediate; rhs = immediate; type = long_word--\n";
        cmp c5{ .lhs = immediate{ 42 }, .rhs = immediate{ 55 }, .type = long_word{} };
        auto c5_result = fixing_up_instructions11(c5);
        REQUIRE(c5_result.has_value());
        result += pretty_print(c5_result.value());

        result += "\n--lhs = reg; rhs = immediate; type = long_word--\n";
        cmp c6{ .lhs = cx{}, .rhs = immediate{ 55 }, .type = long_word{} };
        auto c6_result = fixing_up_instructions11(c6);
        REQUIRE(c6_result.has_value());
        result += pretty_print(c6_result.value());

        result += "\n--lhs = data; rhs = immediate; type = long_word--\n";
        cmp c7{ .lhs = data{ "data_lhs" }, .rhs = immediate{ 55 }, .type = long_word{} };
        auto c7_result = fixing_up_instructions11(c7);
        REQUIRE(c7_result.has_value());
        result += pretty_print(c7_result.value());

        result += "\n--lhs = stack; rhs = immediate; type = long_word--\n";
        cmp c8{ .lhs = stack{ 8 }, .rhs = immediate{ 55 }, .type = long_word{} };
        auto c8_result = fixing_up_instructions11(c8);
        REQUIRE(c8_result.has_value());
        result += pretty_print(c8_result.value());

        int64_t long_immediate = std::numeric_limits<int32_t>::max();
        long_immediate++;

        result += "\n--lhs = big immediate; rhs = reg; type = long_word--\n";
        cmp c9{ .lhs = immediate{ long_immediate }, .rhs = ax{}, .type = quad_word{} };
        auto c9_result = fixing_up_instructions11(c9);
        REQUIRE(c9_result.has_value());
        result += pretty_print(c9_result.value());

        result += "\n--lhs = big immediate; rhs = data; type = long_word--\n";
        cmp c10{ .lhs = immediate{ long_immediate }, .rhs = data{ "data_rhs" }, .type = quad_word{} };
        auto c10_result = fixing_up_instructions11(c10);
        REQUIRE(c10_result.has_value());
        result += pretty_print(c10_result.value());

        result += "\n--lhs = big immediate; rhs = stack; type = long_word--\n";
        cmp c11{ .lhs = immediate{ long_immediate }, .rhs = stack{ 16 }, .type = quad_word{} };
        auto c11_result = fixing_up_instructions11(c11);
        REQUIRE(c11_result.has_value());
        result += pretty_print(c11_result.value());

        result += "\n--lhs = big immediate; rhs = big immediate; type = long_word--\n";
        cmp c12{ .lhs = immediate{ long_immediate }, .rhs = immediate{ long_immediate + 1 }, .type = quad_word{} };
        auto c12_result = fixing_up_instructions11(c12);
        REQUIRE(c12_result.has_value());
        result += pretty_print(c12_result.value());

        result += "\n--lhs = reg; rhs = data; type = double_asm--\n";
        cmp c13{ .lhs = XMM0{}, .rhs = data{ "data_dst" }, .type = wccff::double_asm{} };
        auto c13_result = fixing_up_instructions11(c13);
        REQUIRE(c13_result.has_value());
        result += pretty_print(c13_result.value());

        result += "\n--lhs = reg; rhs = stack; type = double_asm--\n";
        cmp c14{ .lhs = XMM0{}, .rhs = stack{ 16 }, .type = wccff::double_asm{} };
        auto c14_result = fixing_up_instructions11(c14);
        REQUIRE(c14_result.has_value());
        result += pretty_print(c14_result.value());

        result += "\n--lhs = immediate; rhs = data; type = double_asm--\n";
        cmp c15{ .lhs = immediate{ 8 }, .rhs = data{ "data_dst" }, .type = wccff::double_asm{} };
        auto c15_result = fixing_up_instructions11(c15);
        REQUIRE(c15_result.has_value());
        result += pretty_print(c15_result.value());

        result += "\n--lhs = immediate; rhs = stack; type = double_asm--\n";
        cmp c16{ .lhs = immediate{ 8 }, .rhs = stack{ 16 }, .type = wccff::double_asm{} };
        auto c16_result = fixing_up_instructions11(c16);
        REQUIRE(c16_result.has_value());
        result += pretty_print(c16_result.value());

        result += "\n--lhs = data; rhs = data; type = double_asm--\n";
        cmp c17{ .lhs = data{ "data_src" }, .rhs = data{ "data_dst" }, .type = wccff::double_asm{} };
        auto c17_result = fixing_up_instructions11(c17);
        REQUIRE(c17_result.has_value());
        result += pretty_print(c17_result.value());

        result += "\n--lhs = data; rhs = stack; type = double_asm--\n";
        cmp c18{ .lhs = data{ "data_src" }, .rhs = stack{ 16 }, .type = wccff::double_asm{} };
        auto c18_result = fixing_up_instructions11(c18);
        REQUIRE(c18_result.has_value());
        result += pretty_print(c18_result.value());

        ApprovalTests::Approvals::verify(result);
    }

    SECTION("cvtsi2sd")
    {
        using wccff::assembly_generation::cvtsi2sd;
        using wccff::assembly_generation::fixing_up_instructions11;
        using wccff::assembly_generation::XMM1;

        SECTION("No Fixing needed")
        {
            cvtsi2sd b1{ .src = ax{}, .dst = XMM1{}, .src_type = long_word{} };
            REQUIRE(fixing_up_instructions11(b1).has_value() == false);

            cvtsi2sd b2{ .src = data{ "data_src" }, .dst = XMM1{}, .src_type = quad_word{} };
            REQUIRE(fixing_up_instructions11(b2).has_value() == false);

            cvtsi2sd b3{ .src = stack{ 8 }, .dst = XMM1{}, .src_type = long_word{} };
            REQUIRE(fixing_up_instructions11(b3).has_value() == false);
        }
        std::string result;

        result += "--src = immediate; dst = XMM1; type = long_word--\n";
        cvtsi2sd b1{ .src = immediate{ 8 }, .dst = XMM1{}, .src_type = long_word{} };
        auto b1_result = fixing_up_instructions11(b1);
        REQUIRE(b1_result.has_value());
        result += pretty_print(b1_result.value());

        result += "\n--src = immediate; dst = data_dst; type = long_word--\n";
        cvtsi2sd b2{ .src = immediate{ 8 }, .dst = data{ "data_dst" }, .src_type = quad_word{} };
        auto b2_result = fixing_up_instructions11(b2);
        REQUIRE(b2_result.has_value());
        result += pretty_print(b2_result.value());

        result += "\n--src = immediate; dst = stack; type = long_word--\n";
        cvtsi2sd b3{ .src = immediate{ 8 }, .dst = stack{ 8 }, .src_type = long_word{} };
        auto b3_result = fixing_up_instructions11(b3);
        REQUIRE(b3_result.has_value());
        result += pretty_print(b3_result.value());

        result += "\n--src = reg; dst = data_dst; type = long_word--\n";
        cvtsi2sd b4{ .src = ax{}, .dst = data{ "data_dst" }, .src_type = quad_word{} };
        auto b4_result = fixing_up_instructions11(b4);
        REQUIRE(b4_result.has_value());
        result += pretty_print(b4_result.value());

        result += "\n--src = reg; dst = stack; type = long_word--\n";
        cvtsi2sd b5{ .src = ax{}, .dst = stack{ 8 }, .src_type = long_word{} };
        auto b5_result = fixing_up_instructions11(b5);
        REQUIRE(b5_result.has_value());
        result += pretty_print(b5_result.value());

        ApprovalTests::Approvals::verify(result);
    }

    SECTION("cvttsd2si")
    {
        using wccff::assembly_generation::cvttsd2si;
        using wccff::assembly_generation::fixing_up_instructions11;
        using wccff::assembly_generation::XMM1;

        SECTION("No Fixing needed")
        {
            cvttsd2si b1{ .src = XMM1{}, .dst = ax{}, .dst_type = long_word{} };
            REQUIRE(fixing_up_instructions11(b1).has_value() == false);

            cvttsd2si b2{ .src = data{ "data_src" }, .dst = ax{}, .dst_type = quad_word{} };
            REQUIRE(fixing_up_instructions11(b2).has_value() == false);

            cvttsd2si b3{ .src = stack{ 8 }, .dst = ax{}, .dst_type = long_word{} };
            REQUIRE(fixing_up_instructions11(b3).has_value() == false);
        }
        std::string result;

        result += "--src = reg; dst = data_dst; type = long_word--\n";
        cvttsd2si b1{ .src = XMM1{}, .dst = data{ "data_dst" }, .dst_type = long_word{} };
        auto b1_result = fixing_up_instructions11(b1);
        REQUIRE(b1_result.has_value());
        result += pretty_print(b1_result.value());

        result += "\n--src = reg; dst = stack; type = long_word--\n";
        cvttsd2si b2{ .src = XMM1{}, .dst = stack{ 8 }, .dst_type = quad_word{} };
        auto b2_result = fixing_up_instructions11(b2);
        REQUIRE(b2_result.has_value());
        result += pretty_print(b2_result.value());

        ApprovalTests::Approvals::verify(result);
    }

    SECTION("mul")
    {
        using wccff::assembly_generation::binary;
        using wccff::assembly_generation::fixing_up_instructions_binary;
        using wccff::assembly_generation::mul;

        SECTION("No Fixing needed")
        {
            binary b{ .op = mul{}, .src = immediate{ 42 }, .dst = cx{}, .type = long_word{} };
            REQUIRE(fixing_up_instructions_binary(b).has_value() == false);

            binary b1{ .op = mul{}, .src = ax{}, .dst = cx{}, .type = long_word{} };
            REQUIRE(fixing_up_instructions_binary(b1).has_value() == false);

            binary b2{ .op = mul{}, .src = stack{ 16 }, .dst = cx{}, .type = long_word{} };
            REQUIRE(fixing_up_instructions_binary(b2).has_value() == false);

            binary b3{ .op = mul{}, .src = data{ "foo" }, .dst = cx{}, .type = long_word{} };
            REQUIRE(fixing_up_instructions_binary(b3).has_value() == false);

            binary b4{ .op = mul{}, .src = immediate{ 42 }, .dst = cx{}, .type = quad_word{} };
            REQUIRE(fixing_up_instructions_binary(b4).has_value() == false);

            binary b5{ .op = mul{}, .src = ax{}, .dst = cx{}, .type = quad_word{} };
            REQUIRE(fixing_up_instructions_binary(b5).has_value() == false);

            binary b6{ .op = mul{}, .src = stack{ 16 }, .dst = cx{}, .type = quad_word{} };
            REQUIRE(fixing_up_instructions_binary(b6).has_value() == false);

            binary b7{ .op = mul{}, .src = data{ "foo" }, .dst = cx{}, .type = quad_word{} };
            REQUIRE(fixing_up_instructions_binary(b7).has_value() == false);
        }
        std::string result;
        result += "--dst = stack; type = long_word--\n";
        binary b{ .op = mul{}, .src = immediate{ 42 }, .dst = stack{ 16 }, .type = long_word{} };
        auto b_result = fixing_up_instructions_binary(b);
        REQUIRE(b_result.has_value());
        result += pretty_print(b_result.value());

        result += "\n--dst = data; type = long_word--\n";
        binary b1{ .op = mul{}, .src = ax{}, .dst = data{ "foo" }, .type = long_word{} };
        auto b1_result = fixing_up_instructions_binary(b1);
        REQUIRE(b1_result.has_value());
        result += pretty_print(b1_result.value());

        result += "\n--dst = stack; type = quad_word--\n";
        binary b2{ .op = mul{}, .src = immediate{ 42 }, .dst = stack{ 16 }, .type = quad_word{} };
        auto b2_result = fixing_up_instructions_binary(b2);
        REQUIRE(b2_result.has_value());
        result += pretty_print(b2_result.value());

        result += "\n--dst = data; type = quad_word--\n";
        binary b3{ .op = mul{}, .src = ax{}, .dst = data{ "foo" }, .type = quad_word{} };
        auto b3_result = fixing_up_instructions_binary(b3);
        REQUIRE(b3_result.has_value());
        result += pretty_print(b3_result.value());

        result += "\n--src = long immediate; dst = ax; type = quad_word--\n";
        int64_t long_value = std::numeric_limits<int32_t>::max();
        long_value++;
        binary b4{ .op = mul{}, .src = immediate{ long_value }, .dst = ax{}, .type = quad_word{} };
        auto b4_result = fixing_up_instructions_binary(b4);
        REQUIRE(b4_result.has_value());
        result += pretty_print(b4_result.value());

        result += "\n--src = long immediate; dst = data; type = quad_word--\n";
        binary b5{ .op = mul{}, .src = immediate{ long_value }, .dst = data{ "foo" }, .type = quad_word{} };
        auto b5_result = fixing_up_instructions_binary(b5);
        REQUIRE(b5_result.has_value());
        result += pretty_print(b5_result.value());

        result += "\n--src = long immediate; dst = stack; type = quad_word--\n";
        binary b6{ .op = mul{}, .src = immediate{ long_value }, .dst = stack{ 16 }, .type = quad_word{} };
        auto b6_result = fixing_up_instructions_binary(b6);
        REQUIRE(b6_result.has_value());
        result += pretty_print(b6_result.value());
        result += "\n";

        ApprovalTests::Approvals::verify(result);
    }

    SECTION("mov")
    {
        using wccff::assembly_generation::fixing_up_instructions11;
        using wccff::assembly_generation::mov_instruction;

        int64_t long_value = std::numeric_limits<int32_t>::max();
        long_value++;

        SECTION("No Fixing needed")
        {
            mov_instruction b1{ .src = immediate{ 42 }, .dst = cx{}, .type = long_word{} };
            REQUIRE(fixing_up_instructions11(b1).has_value() == false);

            mov_instruction b2{ .src = immediate{ 42 }, .dst = data{ "data_dst" }, .type = quad_word{} };
            REQUIRE(fixing_up_instructions11(b2).has_value() == false);

            mov_instruction b3{ .src = immediate{ 42 }, .dst = stack{ 16 }, .type = long_word{} };
            REQUIRE(fixing_up_instructions11(b3).has_value() == false);

            mov_instruction b4{ .src = immediate{ long_value }, .dst = cx{}, .type = quad_word{} };
            REQUIRE(fixing_up_instructions11(b4).has_value() == false);

            mov_instruction b5{ .src = ax{}, .dst = cx{}, .type = quad_word{} };
            REQUIRE(fixing_up_instructions11(b5).has_value() == false);

            mov_instruction b6{ .src = ax{}, .dst = data{ "data_dst" }, .type = long_word{} };
            REQUIRE(fixing_up_instructions11(b6).has_value() == false);

            mov_instruction b7{ .src = ax{}, .dst = stack{ 16 }, .type = quad_word{} };
            REQUIRE(fixing_up_instructions11(b7).has_value() == false);

            mov_instruction b8{ .src = data{ "data_src" }, .dst = cx{}, .type = long_word{} };
            REQUIRE(fixing_up_instructions11(b8).has_value() == false);

            mov_instruction b9{ .src = stack{ 8 }, .dst = cx{}, .type = quad_word{} };
            REQUIRE(fixing_up_instructions11(b9).has_value() == false);
        }

        std::string result;

        result += "--src = data; dst = data; type = long_word--\n";
        mov_instruction b1{ .src = data{ "data_src" }, .dst = data{ "data_dst" }, .type = long_word{} };
        auto b1_result = fixing_up_instructions11(b1);
        REQUIRE(b1_result.has_value());
        result += pretty_print(b1_result.value());

        result += "\n--src = data; dst = stack; type = quad_word--\n";
        mov_instruction b2{ .src = data{ "data_src" }, .dst = stack{ 16 }, .type = quad_word{} };
        auto b2_result = fixing_up_instructions11(b2);
        REQUIRE(b2_result.has_value());
        result += pretty_print(b2_result.value());

        result += "\n--src = stack; dst = data; type = long_word--\n";
        mov_instruction b3{ .src = stack{ 8 }, .dst = data{ "data_dst" }, .type = long_word{} };
        auto b3_result = fixing_up_instructions11(b3);
        REQUIRE(b3_result.has_value());
        result += pretty_print(b3_result.value());

        result += "\n--src = stack; dst = stack; type = quad_word--\n";
        mov_instruction b4{ .src = stack{ 8 }, .dst = stack{ 16 }, .type = quad_word{} };
        auto b4_result = fixing_up_instructions11(b4);
        REQUIRE(b4_result.has_value());
        result += pretty_print(b4_result.value());

        result += "\n--src = large immediate; dst = stack; type = quad_word--\n";
        mov_instruction b5{ .src = immediate{ long_value }, .dst = stack{ 16 }, .type = quad_word{} };
        auto b5_result = fixing_up_instructions11(b5);
        REQUIRE(b5_result.has_value());
        result += pretty_print(b5_result.value());

        result += "\n--src = large immediate; dst = data; type = quad_word--\n";
        mov_instruction b6{ .src = immediate{ long_value }, .dst = data{ "data_dst" }, .type = quad_word{} };
        auto b6_result = fixing_up_instructions11(b6);
        REQUIRE(b6_result.has_value());
        result += pretty_print(b6_result.value());

        result += "\n--src = large immediate; dst = stack; type = long_word--\n";
        mov_instruction b7{ .src = immediate{ long_value }, .dst = stack{ 16 }, .type = long_word{} };
        auto b7_result = fixing_up_instructions11(b7);
        REQUIRE(b7_result.has_value());
        result += pretty_print(b7_result.value());

        result += "\n--src = large immediate; dst = data; type = long_word--\n";
        mov_instruction b8{ .src = immediate{ long_value }, .dst = data{ "data_dst" }, .type = long_word{} };
        auto b8_result = fixing_up_instructions11(b8);
        REQUIRE(b8_result.has_value());
        result += pretty_print(b8_result.value());

        result += "\n--src = large immediate; dst = reg; type = long_word--\n";
        mov_instruction b9{ .src = immediate{ long_value }, .dst = cx{}, .type = long_word{} };
        auto b9_result = fixing_up_instructions11(b9);
        REQUIRE(b9_result.has_value());
        result += pretty_print(b9_result.value());

        ApprovalTests::Approvals::verify(result);
    }

    SECTION("mov_zero_extend")
    {
        using wccff::assembly_generation::fixing_up_instructions11;
        using wccff::assembly_generation::mov_zero_extend;

        std::string result;
        result += "--src = immediate; dst = reg--\n";
        mov_zero_extend b1{ .src = immediate{ 42 }, .dst = cx{} };
        auto b1_result = fixing_up_instructions11(b1);
        REQUIRE(b1_result.has_value());
        result += pretty_print(b1_result.value());

        result += "\n--src = reg; dst = reg--\n";
        mov_zero_extend b2{ .src = ax{}, .dst = cx{} };
        auto b2_result = fixing_up_instructions11(b2);
        REQUIRE(b2_result.has_value());
        result += pretty_print(b2_result.value());

        result += "\n--src = pseudo; dst = reg--\n";
        mov_zero_extend b3{ .src = stack{ 8 }, .dst = cx{} };
        auto b3_result = fixing_up_instructions11(b3);
        REQUIRE(b3_result.has_value());
        result += pretty_print(b3_result.value());

        result += "\n--src = data; dst = reg--\n";
        mov_zero_extend b4{ .src = data{ "data_src" }, .dst = cx{} };
        auto b4_result = fixing_up_instructions11(b4);
        REQUIRE(b4_result.has_value());
        result += pretty_print(b4_result.value());

        result += "\n--src = immediate; dst = data--\n";
        mov_zero_extend b5{ .src = immediate{ 42 }, .dst = data{ "data_dst" } };
        auto b5_result = fixing_up_instructions11(b5);
        REQUIRE(b5_result.has_value());
        result += pretty_print(b5_result.value());

        result += "\n--src = immediate; dst = stack--\n";
        mov_zero_extend b6{ .src = immediate{ 42 }, .dst = stack{ 16 } };
        auto b6_result = fixing_up_instructions11(b6);
        REQUIRE(b6_result.has_value());
        result += pretty_print(b6_result.value());

        result += "\n--src = reg; dst = data--\n";
        mov_zero_extend b7{ .src = ax{}, .dst = data{ "data_dst" } };
        auto b7_result = fixing_up_instructions11(b7);
        REQUIRE(b7_result.has_value());
        result += pretty_print(b7_result.value());

        result += "\n--src = reg; dst = stack--\n";
        mov_zero_extend b8{ .src = ax{}, .dst = stack{ 16 } };
        auto b8_result = fixing_up_instructions11(b8);
        REQUIRE(b8_result.has_value());
        result += pretty_print(b8_result.value());

        result += "\n--src = stack; dst = data--\n";
        mov_zero_extend b9{ .src = stack{ 8 }, .dst = data{ "data_dst" } };
        auto b9_result = fixing_up_instructions11(b9);
        REQUIRE(b9_result.has_value());
        result += pretty_print(b9_result.value());

        result += "\n--src = stack; dst = stack--\n";
        mov_zero_extend b10{ .src = stack{ 8 }, .dst = stack{ 16 } };
        auto b10_result = fixing_up_instructions11(b10);
        REQUIRE(b10_result.has_value());
        result += pretty_print(b10_result.value());

        result += "\n--src = data; dst = stack--\n";
        mov_zero_extend b11{ .src = data{ "data_src" }, .dst = stack{ 16 } };
        auto b11_result = fixing_up_instructions11(b11);
        REQUIRE(b11_result.has_value());
        result += pretty_print(b11_result.value());

        result += "\n--src = data; dst = data--\n";
        mov_zero_extend b12{ .src = data{ "data_src" }, .dst = data{ "data_dst" } };
        auto b12_result = fixing_up_instructions11(b12);
        REQUIRE(b12_result.has_value());
        result += pretty_print(b12_result.value());

        ApprovalTests::Approvals::verify(result);
    }

    SECTION("movx")
    {
        using wccff::assembly_generation::fixing_up_instructions11;
        using wccff::assembly_generation::movx;

        SECTION("No Fixing needed")
        {
            movx b1{ .src = ax{}, .dst = cx{} };
            REQUIRE(fixing_up_instructions11(b1).has_value() == false);

            movx b2{ .src = data{ "data_src" }, .dst = cx{} };
            REQUIRE(fixing_up_instructions11(b2).has_value() == false);

            movx b3{ .src = stack{ 8 }, .dst = cx{} };
            REQUIRE(fixing_up_instructions11(b3).has_value() == false);
        }

        std::string result;
        result += "--src = immediate; dst = reg--\n";
        movx b1{ .src = immediate{ 42 }, .dst = cx{} };
        auto b1_result = fixing_up_instructions11(b1);
        REQUIRE(b1_result.has_value());
        result += pretty_print(b1_result.value());

        result += "\n--src = immediate; dst = data--\n";
        movx b2{ .src = immediate{ 42 }, .dst = data{ "data_dst" } };
        auto b2_result = fixing_up_instructions11(b2);
        REQUIRE(b2_result.has_value());
        result += pretty_print(b2_result.value());

        result += "\n--src = immediate; dst = stack--\n";
        movx b3{ .src = immediate{ 42 }, .dst = stack{ 16 } };
        auto b3_result = fixing_up_instructions11(b3);
        REQUIRE(b3_result.has_value());
        result += pretty_print(b3_result.value());

        result += "\n--src = reg; dst = data--\n";
        movx b4{ .src = ax{}, .dst = data{ "data_dst" } };
        auto b4_result = fixing_up_instructions11(b4);
        REQUIRE(b4_result.has_value());
        result += pretty_print(b4_result.value());

        result += "\n--src = reg; dst = stack--\n";
        movx b5{ .src = ax{}, .dst = stack{ 16 } };
        auto b5_result = fixing_up_instructions11(b5);
        REQUIRE(b5_result.has_value());
        result += pretty_print(b5_result.value());

        ApprovalTests::Approvals::verify(result);
    }

    SECTION("push")
    {
        using wccff::assembly_generation::fixing_up_instructions11;
        using wccff::assembly_generation::push;
        SECTION("No Fixing needed")
        {
            push p1{ .src = cx{} };
            REQUIRE(fixing_up_instructions11(p1).has_value() == false);

            push p2{ .src = immediate{ 42 } };
            REQUIRE(fixing_up_instructions11(p2).has_value() == false);

            push p3{ .src = stack{ 8 } };
            REQUIRE(fixing_up_instructions11(p3).has_value() == false);

            push p4{ .src = data{ "data_src" } };
            REQUIRE(fixing_up_instructions11(p4).has_value() == false);
        }

        int64_t long_value = std::numeric_limits<int32_t>::max();
        long_value++;

        std::string result;
        result += "--src = large immediate--\n";
        push p{ .src = immediate{ long_value } };
        auto p_result = fixing_up_instructions11(p);
        REQUIRE(p_result.has_value());
        result += pretty_print(p_result.value());

        ApprovalTests::Approvals::verify(result);
    }

    SECTION("sub")
    {
        using wccff::assembly_generation::binary;
        using wccff::assembly_generation::fixing_up_instructions_binary;
        using wccff::assembly_generation::sub;
        SECTION("No Fixing needed")
        {
            binary b{ .op = sub{}, .src = immediate{ 42 }, .dst = cx{}, .type = long_word{} };
            REQUIRE(fixing_up_instructions_binary(b).has_value() == false);

            binary b1{ .op = sub{}, .src = immediate{ 42 }, .dst = data{ "data" }, .type = quad_word{} };
            REQUIRE(fixing_up_instructions_binary(b1).has_value() == false);

            binary b2{ .op = sub{}, .src = immediate{ 42 }, .dst = stack{ 16 }, .type = long_word{} };
            REQUIRE(fixing_up_instructions_binary(b2).has_value() == false);

            binary b3{ .op = sub{}, .src = ax{}, .dst = cx{}, .type = long_word{} };
            REQUIRE(fixing_up_instructions_binary(b3).has_value() == false);

            binary b4{ .op = sub{}, .src = ax{}, .dst = data{ "data" }, .type = quad_word{} };
            REQUIRE(fixing_up_instructions_binary(b4).has_value() == false);

            binary b5{ .op = sub{}, .src = ax{}, .dst = stack{ 16 }, .type = long_word{} };
            REQUIRE(fixing_up_instructions_binary(b5).has_value() == false);

            binary b6{ .op = sub{}, .src = data{ "data" }, .dst = cx{}, .type = quad_word{} };
            REQUIRE(fixing_up_instructions_binary(b6).has_value() == false);

            binary b7{ .op = sub{}, .src = stack{ 16 }, .dst = cx{}, .type = long_word{} };
            REQUIRE(fixing_up_instructions_binary(b7).has_value() == false);
        }

        std::string result;
        result += "--src = stack; dst = stack; type = long_word--\n";
        binary b{ .op = sub{}, .src = stack{ 8 }, .dst = stack{ 16 }, .type = long_word{} };
        auto b_result = fixing_up_instructions_binary(b);
        REQUIRE(b_result.has_value());
        result += pretty_print(b_result.value());

        result += "\n--src = stack; dst = data; type = long_word--\n";
        binary b1{ .op = sub{}, .src = stack{ 8 }, .dst = data{ "foo_dst" }, .type = quad_word{} };
        auto b1_result = fixing_up_instructions_binary(b1);
        REQUIRE(b1_result.has_value());
        result += pretty_print(b1_result.value());

        result += "\n--src = data; dst = stack; type = long_word--\n";
        binary b2{ .op = sub{}, .src = data{ "foo_src" }, .dst = stack{ 16 }, .type = long_word{} };
        auto b2_result = fixing_up_instructions_binary(b2);
        REQUIRE(b2_result.has_value());
        result += pretty_print(b2_result.value());

        result += "\n--src = data; dst = data; type = long_word--\n";
        binary b3{ .op = sub{}, .src = data{ "foo_src" }, .dst = data{ "foo_dst" }, .type = quad_word{} };
        auto b3_result = fixing_up_instructions_binary(b3);
        REQUIRE(b3_result.has_value());
        result += pretty_print(b3_result.value());

        ApprovalTests::Approvals::verify(result);
    }
}

TEST_CASE("pretty_print", "[assembly_generation]")
{
    using wccff::identifier;
    using wccff::long_word;
    using wccff::quad_word;
    using wccff::assembly_generation::A;
    using wccff::assembly_generation::AE;
    using wccff::assembly_generation::B;
    using wccff::assembly_generation::BE;
    using wccff::assembly_generation::binary;
    using wccff::assembly_generation::call;
    using wccff::assembly_generation::cmp;
    using wccff::assembly_generation::cvtsi2sd;
    using wccff::assembly_generation::cvttsd2si;
    using wccff::assembly_generation::data;
    using wccff::assembly_generation::E;
    using wccff::assembly_generation::function;
    using wccff::assembly_generation::G;
    using wccff::assembly_generation::GE;
    using wccff::assembly_generation::immediate;
    using wccff::assembly_generation::L;
    using wccff::assembly_generation::LE;
    using wccff::assembly_generation::mov_instruction;
    using wccff::assembly_generation::mov_zero_extend;
    using wccff::assembly_generation::movx;
    using wccff::assembly_generation::NE;
    using wccff::assembly_generation::neg_op;
    using wccff::assembly_generation::not_op;
    using wccff::assembly_generation::pretty_print;
    using wccff::assembly_generation::pseudo;
    using wccff::assembly_generation::R10;
    using wccff::assembly_generation::R11;
    using wccff::assembly_generation::stack;
    using wccff::assembly_generation::static_variable;
    using wccff::assembly_generation::unary;
    auto directoryDisposer = ApprovalTests::Approvals::useApprovalsSubdirectory("assembly_generation_results");

    std::string result;

    // assembly_type
    result += "--assembly_type--\n";
    result += pretty_print(wccff::long_word{});
    result += '\n';
    result += pretty_print(wccff::quad_word{});
    result += '\n';

    // binary
    result += "--binary--\n";
    result += pretty_print(binary{ .op = wccff::assembly_generation::add{},
                                   .src = immediate{ 42 },
                                   .dst = immediate{ 44 },
                                   .type = wccff::long_word{} });
    result += '\n';
    result += pretty_print(binary{ .op = wccff::assembly_generation::sub{},
                                   .src = immediate{ 42 },
                                   .dst = R10{},
                                   .type = wccff::quad_word{} });
    result += '\n';
    result += pretty_print(binary{ .op = wccff::assembly_generation::mul{},
                                   .src = immediate{ 42 },
                                   .dst = pseudo{ "xpto" },
                                   .type = wccff::long_word{} });
    result += '\n';
    result += pretty_print(binary{ .op = wccff::assembly_generation::binary_and{},
                                   .src = immediate{ 42 },
                                   .dst = stack{ 50 },
                                   .type = wccff::quad_word{} });
    result += '\n';
    result += pretty_print(binary{ .op = wccff::assembly_generation::binary_or{},
                                   .src = immediate{ 42 },
                                   .dst = data{ "bar" },
                                   .type = wccff::long_word{} });
    result += '\n';
    result += pretty_print(binary{ .op = wccff::assembly_generation::binary_xor{},
                                   .src = R10{},
                                   .dst = immediate{ 42 },
                                   .type = wccff::quad_word{} });
    result += '\n';
    result += pretty_print(
      binary{ .op = wccff::assembly_generation::left_shift{}, .src = R10{}, .dst = R11{}, .type = wccff::long_word{} });
    result += '\n';
    result += pretty_print(binary{ .op = wccff::assembly_generation::right_shift{},
                                   .src = R10{},
                                   .dst = pseudo{ "bar" },
                                   .type = wccff::quad_word{} });
    result += '\n';
    result += pretty_print(
      binary{ .op = wccff::assembly_generation::add{}, .src = R10{}, .dst = stack{ 50 }, .type = wccff::quad_word{} });
    result += '\n';
    result += pretty_print(binary{ .op = wccff::assembly_generation::add{},
                                   .src = R10{},
                                   .dst = data{ "bar" },
                                   .type = wccff::quad_word{} });
    result += '\n';

    result += pretty_print(binary{ .op = wccff::assembly_generation::sub{},
                                   .src = pseudo{ "bar" },
                                   .dst = immediate{ 42 },
                                   .type = wccff::quad_word{} });
    result += '\n';
    result += pretty_print(binary{ .op = wccff::assembly_generation::mul{},
                                   .src = pseudo{ "bar" },
                                   .dst = R11{},
                                   .type = wccff::long_word{} });
    result += '\n';
    result += pretty_print(binary{ .op = wccff::assembly_generation::binary_and{},
                                   .src = pseudo{ "bar" },
                                   .dst = pseudo{ "bar" },
                                   .type = wccff::quad_word{} });
    result += '\n';
    result += pretty_print(binary{ .op = wccff::assembly_generation::binary_or{},
                                   .src = pseudo{ "bar" },
                                   .dst = stack{ 50 },
                                   .type = wccff::quad_word{} });
    result += '\n';
    result += pretty_print(binary{ .op = wccff::assembly_generation::binary_xor{},
                                   .src = pseudo{ "bar" },
                                   .dst = data{ "bar" },
                                   .type = wccff::quad_word{} });
    result += '\n';

    result += pretty_print(binary{ .op = wccff::assembly_generation::left_shift{},
                                   .src = stack{ 50 },
                                   .dst = immediate{ 42 },
                                   .type = wccff::quad_word{} });
    result += '\n';
    result += pretty_print(binary{ .op = wccff::assembly_generation::right_shift{},
                                   .src = stack{ 50 },
                                   .dst = R11{},
                                   .type = wccff::long_word{} });
    result += '\n';
    result += pretty_print(binary{ .op = wccff::assembly_generation::add{},
                                   .src = stack{ 50 },
                                   .dst = pseudo{ "bar" },
                                   .type = wccff::quad_word{} });
    result += '\n';
    result += pretty_print(binary{ .op = wccff::assembly_generation::sub{},
                                   .src = stack{ 50 },
                                   .dst = stack{ 50 },
                                   .type = wccff::quad_word{} });
    result += '\n';
    result += pretty_print(binary{ .op = wccff::assembly_generation::mul{},
                                   .src = stack{ 50 },
                                   .dst = data{ "bar" },
                                   .type = wccff::quad_word{} });
    result += '\n';

    result += pretty_print(binary{ .op = wccff::assembly_generation::binary_and{},
                                   .src = data{ "bar" },
                                   .dst = immediate{ 42 },
                                   .type = wccff::quad_word{} });
    result += '\n';
    result += pretty_print(binary{ .op = wccff::assembly_generation::binary_or{},
                                   .src = data{ "bar" },
                                   .dst = R11{},
                                   .type = wccff::long_word{} });
    result += '\n';
    result += pretty_print(binary{ .op = wccff::assembly_generation::binary_xor{},
                                   .src = data{ "bar" },
                                   .dst = pseudo{ "bar" },
                                   .type = wccff::quad_word{} });
    result += '\n';
    result += pretty_print(binary{ .op = wccff::assembly_generation::left_shift{},
                                   .src = data{ "bar" },
                                   .dst = stack{ 50 },
                                   .type = wccff::quad_word{} });
    result += '\n';
    result += pretty_print(binary{ .op = wccff::assembly_generation::right_shift{},
                                   .src = data{ "bar" },
                                   .dst = data{ "bar" },
                                   .type = wccff::quad_word{} });
    result += '\n';

    // binary_operator
    result += "--binary_operator--\n";
    result += pretty_print(wccff::assembly_generation::binary_operator{ wccff::assembly_generation::add{} });
    result += '\n';
    result += pretty_print(wccff::assembly_generation::binary_operator{ wccff::assembly_generation::sub{} });
    result += '\n';
    result += pretty_print(wccff::assembly_generation::binary_operator{ wccff::assembly_generation::mul{} });
    result += '\n';
    result += pretty_print(wccff::assembly_generation::binary_operator{ wccff::assembly_generation::binary_and{} });
    result += '\n';
    result += pretty_print(wccff::assembly_generation::binary_operator{ wccff::assembly_generation::binary_or{} });
    result += '\n';
    result += pretty_print(wccff::assembly_generation::binary_operator{ wccff::assembly_generation::binary_xor{} });
    result += '\n';
    result += pretty_print(wccff::assembly_generation::binary_operator{ wccff::assembly_generation::left_shift{} });
    result += '\n';
    result += pretty_print(wccff::assembly_generation::binary_operator{ wccff::assembly_generation::right_shift{} });
    result += '\n';

    // call
    result += "--call--\n";
    result += pretty_print(call{ identifier{ "foo" } });
    result += '\n';

    // cdq
    result += "--cdq--\n";
    result += pretty_print(wccff::assembly_generation::cdq{ long_word{} });
    result += '\n';
    result += pretty_print(wccff::assembly_generation::cdq{ quad_word{} });
    result += '\n';

    // cmp
    result += "--cmp--\n";
    result += pretty_print(cmp{ .lhs = immediate{ 42 }, .rhs = immediate{ 44 }, .type = wccff::long_word{} });
    result += '\n';
    result += pretty_print(cmp{ .lhs = immediate{ 42 }, .rhs = R10{}, .type = wccff::quad_word{} });
    result += '\n';
    result += pretty_print(cmp{ .lhs = immediate{ 42 }, .rhs = pseudo{ "xpto" }, .type = wccff::long_word{} });
    result += '\n';
    result += pretty_print(cmp{ .lhs = immediate{ 42 }, .rhs = stack{ 50 }, .type = wccff::quad_word{} });
    result += '\n';
    result += pretty_print(cmp{ .lhs = immediate{ 42 }, .rhs = data{ "bar" }, .type = wccff::long_word{} });
    result += '\n';
    result += pretty_print(cmp{ .lhs = R10{}, .rhs = immediate{ 42 }, .type = wccff::quad_word{} });
    result += '\n';
    result += pretty_print(cmp{ .lhs = R10{}, .rhs = R11{}, .type = wccff::long_word{} });
    result += '\n';
    result += pretty_print(cmp{ .lhs = R10{}, .rhs = pseudo{ "bar" }, .type = wccff::quad_word{} });
    result += '\n';
    result += pretty_print(cmp{ .lhs = R10{}, .rhs = stack{ 50 }, .type = wccff::quad_word{} });
    result += '\n';
    result += pretty_print(cmp{ .lhs = R10{}, .rhs = data{ "bar" }, .type = wccff::quad_word{} });
    result += '\n';

    result += pretty_print(cmp{ .lhs = pseudo{ "bar" }, .rhs = immediate{ 42 }, .type = wccff::quad_word{} });
    result += '\n';
    result += pretty_print(cmp{ .lhs = pseudo{ "bar" }, .rhs = R11{}, .type = wccff::long_word{} });
    result += '\n';
    result += pretty_print(cmp{ .lhs = pseudo{ "bar" }, .rhs = pseudo{ "bar" }, .type = wccff::quad_word{} });
    result += '\n';
    result += pretty_print(cmp{ .lhs = pseudo{ "bar" }, .rhs = stack{ 50 }, .type = wccff::quad_word{} });
    result += '\n';
    result += pretty_print(cmp{ .lhs = pseudo{ "bar" }, .rhs = data{ "bar" }, .type = wccff::quad_word{} });
    result += '\n';

    result += pretty_print(cmp{ .lhs = stack{ 50 }, .rhs = immediate{ 42 }, .type = wccff::quad_word{} });
    result += '\n';
    result += pretty_print(cmp{ .lhs = stack{ 50 }, .rhs = R11{}, .type = wccff::long_word{} });
    result += '\n';
    result += pretty_print(cmp{ .lhs = stack{ 50 }, .rhs = pseudo{ "bar" }, .type = wccff::quad_word{} });
    result += '\n';
    result += pretty_print(cmp{ .lhs = stack{ 50 }, .rhs = stack{ 50 }, .type = wccff::quad_word{} });
    result += '\n';
    result += pretty_print(cmp{ .lhs = stack{ 50 }, .rhs = data{ "bar" }, .type = wccff::quad_word{} });
    result += '\n';

    result += pretty_print(cmp{ .lhs = data{ "bar" }, .rhs = immediate{ 42 }, .type = wccff::quad_word{} });
    result += '\n';
    result += pretty_print(cmp{ .lhs = data{ "bar" }, .rhs = R11{}, .type = wccff::long_word{} });
    result += '\n';
    result += pretty_print(cmp{ .lhs = data{ "bar" }, .rhs = pseudo{ "bar" }, .type = wccff::quad_word{} });
    result += '\n';
    result += pretty_print(cmp{ .lhs = data{ "bar" }, .rhs = stack{ 50 }, .type = wccff::quad_word{} });
    result += '\n';
    result += pretty_print(cmp{ .lhs = data{ "bar" }, .rhs = data{ "bar" }, .type = wccff::quad_word{} });
    result += '\n';

    // cond_code
    result += "--cond_code--\n";
    result += pretty_print(wccff::assembly_generation::E{});
    result += '\n';
    result += pretty_print(wccff::assembly_generation::NE{});
    result += '\n';
    result += pretty_print(wccff::assembly_generation::G{});
    result += '\n';
    result += pretty_print(wccff::assembly_generation::GE{});
    result += '\n';
    result += pretty_print(wccff::assembly_generation::L{});
    result += '\n';
    result += pretty_print(wccff::assembly_generation::LE{});
    result += '\n';

    // cvtsi2sd
    result += "--cvtsi2sd--\n";
    result += pretty_print(cvtsi2sd{ .src = immediate{ 42 }, .dst = R11{}, .src_type = long_word{} });
    result += '\n';
    result += pretty_print(cvtsi2sd{ .src = immediate{ 42 }, .dst = pseudo{ "bar" }, .src_type = quad_word{} });
    result += '\n';
    result += pretty_print(cvtsi2sd{ .src = immediate{ 42 }, .dst = stack{ 50 }, .src_type = long_word{} });
    result += '\n';
    result += pretty_print(cvtsi2sd{ .src = immediate{ 42 }, .dst = data{ "bar" }, .src_type = quad_word{} });
    result += '\n';

    result += pretty_print(cvtsi2sd{ .src = R10{}, .dst = R11{}, .src_type = long_word{} });
    result += '\n';
    result += pretty_print(cvtsi2sd{ .src = R10{}, .dst = pseudo{ "bar" }, .src_type = quad_word{} });
    result += '\n';
    result += pretty_print(cvtsi2sd{ .src = R10{}, .dst = stack{ 50 }, .src_type = long_word{} });
    result += '\n';
    result += pretty_print(cvtsi2sd{ .src = R10{}, .dst = data{ "bar" }, .src_type = quad_word{} });
    result += '\n';

    result += pretty_print(cvtsi2sd{ .src = pseudo{ "bar" }, .dst = R11{}, .src_type = long_word{} });
    result += '\n';
    result += pretty_print(cvtsi2sd{ .src = pseudo{ "bar" }, .dst = pseudo{ "bar" }, .src_type = quad_word{} });
    result += '\n';
    result += pretty_print(cvtsi2sd{ .src = pseudo{ "bar" }, .dst = stack{ 50 }, .src_type = long_word{} });
    result += '\n';
    result += pretty_print(cvtsi2sd{ .src = pseudo{ "bar" }, .dst = data{ "bar" }, .src_type = quad_word{} });
    result += '\n';

    result += pretty_print(cvtsi2sd{ .src = stack{ 50 }, .dst = R11{}, .src_type = long_word{} });
    result += '\n';
    result += pretty_print(cvtsi2sd{ .src = stack{ 50 }, .dst = pseudo{ "bar" }, .src_type = quad_word{} });
    result += '\n';
    result += pretty_print(cvtsi2sd{ .src = stack{ 50 }, .dst = stack{ 50 }, .src_type = long_word{} });
    result += '\n';
    result += pretty_print(cvtsi2sd{ .src = stack{ 50 }, .dst = data{ "bar" }, .src_type = quad_word{} });
    result += '\n';

    result += pretty_print(cvtsi2sd{ .src = data{ "bar" }, .dst = R11{}, .src_type = long_word{} });
    result += '\n';
    result += pretty_print(cvtsi2sd{ .src = data{ "bar" }, .dst = pseudo{ "bar" }, .src_type = quad_word{} });
    result += '\n';
    result += pretty_print(cvtsi2sd{ .src = data{ "bar" }, .dst = stack{ 50 }, .src_type = long_word{} });
    result += '\n';
    result += pretty_print(cvtsi2sd{ .src = data{ "bar" }, .dst = data{ "bar" }, .src_type = quad_word{} });
    result += '\n';

    // cvttsd2si
    result += "--cvttsd2si--\n";
    result += pretty_print(cvttsd2si{ .src = immediate{ 42 }, .dst = R11{}, .dst_type = long_word{} });
    result += '\n';
    result += pretty_print(cvttsd2si{ .src = immediate{ 42 }, .dst = pseudo{ "bar" }, .dst_type = quad_word{} });
    result += '\n';
    result += pretty_print(cvttsd2si{ .src = immediate{ 42 }, .dst = stack{ 50 }, .dst_type = long_word{} });
    result += '\n';
    result += pretty_print(cvttsd2si{ .src = immediate{ 42 }, .dst = data{ "bar" }, .dst_type = quad_word{} });
    result += '\n';

    result += pretty_print(cvttsd2si{ .src = R10{}, .dst = R11{}, .dst_type = long_word{} });
    result += '\n';
    result += pretty_print(cvttsd2si{ .src = R10{}, .dst = pseudo{ "bar" }, .dst_type = quad_word{} });
    result += '\n';
    result += pretty_print(cvttsd2si{ .src = R10{}, .dst = stack{ 50 }, .dst_type = long_word{} });
    result += '\n';
    result += pretty_print(cvttsd2si{ .src = R10{}, .dst = data{ "bar" }, .dst_type = quad_word{} });
    result += '\n';

    result += pretty_print(cvttsd2si{ .src = pseudo{ "bar" }, .dst = R11{}, .dst_type = long_word{} });
    result += '\n';
    result += pretty_print(cvttsd2si{ .src = pseudo{ "bar" }, .dst = pseudo{ "bar" }, .dst_type = quad_word{} });
    result += '\n';
    result += pretty_print(cvttsd2si{ .src = pseudo{ "bar" }, .dst = stack{ 50 }, .dst_type = long_word{} });
    result += '\n';
    result += pretty_print(cvttsd2si{ .src = pseudo{ "bar" }, .dst = data{ "bar" }, .dst_type = quad_word{} });
    result += '\n';

    result += pretty_print(cvttsd2si{ .src = stack{ 50 }, .dst = R11{}, .dst_type = long_word{} });
    result += '\n';
    result += pretty_print(cvttsd2si{ .src = stack{ 50 }, .dst = pseudo{ "bar" }, .dst_type = quad_word{} });
    result += '\n';
    result += pretty_print(cvttsd2si{ .src = stack{ 50 }, .dst = stack{ 50 }, .dst_type = long_word{} });
    result += '\n';
    result += pretty_print(cvttsd2si{ .src = stack{ 50 }, .dst = data{ "bar" }, .dst_type = quad_word{} });
    result += '\n';

    result += pretty_print(cvttsd2si{ .src = data{ "bar" }, .dst = R11{}, .dst_type = long_word{} });
    result += '\n';
    result += pretty_print(cvttsd2si{ .src = data{ "bar" }, .dst = pseudo{ "bar" }, .dst_type = quad_word{} });
    result += '\n';
    result += pretty_print(cvttsd2si{ .src = data{ "bar" }, .dst = stack{ 50 }, .dst_type = long_word{} });
    result += '\n';
    result += pretty_print(cvttsd2si{ .src = data{ "bar" }, .dst = data{ "bar" }, .dst_type = quad_word{} });
    result += '\n';

    // data
    result += "--data--\n";
    result += pretty_print(data{ "data_name" });
    result += '\n';

    // div
    result += "--div--\n";
    result += pretty_print(wccff::assembly_generation::div{ .src = immediate{ 42 }, .type = long_word{} });
    result += '\n';
    result += pretty_print(wccff::assembly_generation::div{ .src = R11{}, .type = quad_word{} });
    result += '\n';
    result += pretty_print(wccff::assembly_generation::div{ .src = pseudo{ "bar" }, .type = quad_word{} });
    result += '\n';
    result += pretty_print(wccff::assembly_generation::div{ .src = stack{ 50 }, .type = quad_word{} });
    result += '\n';
    result += pretty_print(wccff::assembly_generation::div{ .src = data{ "bar" }, .type = quad_word{} });
    result += '\n';

    // function
    auto generate_func_body = []() -> std::vector<wccff::assembly_generation::instruction> {
        std::vector<wccff::assembly_generation::instruction> result;
        result.emplace_back(wccff::assembly_generation::ret_instruction{});
        return result;
    };
    result += "--function--\n";
    result += pretty_print(function{ .name = "identifier_name", .is_global = true, .stack_size = 16 });
    result += '\n';
    result += pretty_print(function{ .name = "identifier_name", .is_global = false, .stack_size = 56 });
    result += '\n';
    result += pretty_print(function{ .name = "identifier_name",
                                     .is_global = false,
                                     .stack_size = 56,
                                     .instructions = generate_func_body() });
    result += '\n';

    // identifier
    result += "--identifier--\n";
    result += pretty_print(identifier{ "identifier_name" });
    result += '\n';

    // idiv
    result += "--idiv--\n";
    result += pretty_print(wccff::assembly_generation::idiv{ .src = immediate{ 42 }, .type = long_word{} });
    result += '\n';
    result += pretty_print(wccff::assembly_generation::idiv{ .src = R11{}, .type = quad_word{} });
    result += '\n';
    result += pretty_print(wccff::assembly_generation::idiv{ .src = pseudo{ "bar" }, .type = quad_word{} });
    result += '\n';
    result += pretty_print(wccff::assembly_generation::idiv{ .src = stack{ 50 }, .type = quad_word{} });
    result += '\n';
    result += pretty_print(wccff::assembly_generation::idiv{ .src = data{ "bar" }, .type = quad_word{} });
    result += '\n';

    // immediate
    result += "--immediate--\n";
    result += pretty_print(immediate{ 666 });
    result += '\n';
    result += pretty_print(immediate{ -555 });
    result += '\n';

    // jmp
    result += "--jmp--\n";
    result += pretty_print(wccff::assembly_generation::jmp{ "jmp_target" });
    result += '\n';

    // jmpcc
    result += "--jmpcc--\n";
    result += pretty_print(wccff::assembly_generation::jmpcc{ .name = "jmp_target", .cond = E{} });
    result += '\n';

    // label
    result += "--label--\n";
    result += pretty_print(wccff::assembly_generation::label{ "label_name" });
    result += '\n';

    // mov
    result += "--mov--\n";
    result += pretty_print(mov_instruction{ .src = immediate{ 42 }, .dst = immediate{ 55 }, .type = long_word{} });
    result += '\n';
    result += pretty_print(mov_instruction{ .src = immediate{ 42 }, .dst = R10{}, .type = quad_word{} });
    result += '\n';
    result += pretty_print(mov_instruction{ .src = immediate{ 42 }, .dst = pseudo{ "pseu_dst" }, .type = long_word{} });
    result += '\n';
    result += pretty_print(mov_instruction{ .src = immediate{ 42 }, .dst = stack{ -32 }, .type = long_word{} });
    result += '\n';
    result += pretty_print(mov_instruction{ .src = immediate{ 42 }, .dst = data{ "data_dst" }, .type = long_word{} });
    result += '\n';

    result += pretty_print(mov_instruction{ .src = R10{}, .dst = immediate{ 55 }, .type = long_word{} });
    result += '\n';
    result += pretty_print(mov_instruction{ .src = R10{}, .dst = R10{}, .type = quad_word{} });
    result += '\n';
    result += pretty_print(mov_instruction{ .src = R10{}, .dst = pseudo{ "pseu_dst" }, .type = long_word{} });
    result += '\n';
    result += pretty_print(mov_instruction{ .src = R10{}, .dst = stack{ -32 }, .type = long_word{} });
    result += '\n';
    result += pretty_print(mov_instruction{ .src = R10{}, .dst = data{ "data_dst" }, .type = long_word{} });
    result += '\n';

    result += pretty_print(mov_instruction{ .src = pseudo{ "src" }, .dst = immediate{ 55 }, .type = long_word{} });
    result += '\n';
    result += pretty_print(mov_instruction{ .src = pseudo{ "src" }, .dst = R10{}, .type = quad_word{} });
    result += '\n';
    result += pretty_print(mov_instruction{ .src = pseudo{ "src" }, .dst = pseudo{ "pseu_dst" }, .type = long_word{} });
    result += '\n';
    result += pretty_print(mov_instruction{ .src = pseudo{ "src" }, .dst = stack{ -32 }, .type = long_word{} });
    result += '\n';
    result += pretty_print(mov_instruction{ .src = pseudo{ "src" }, .dst = data{ "data_dst" }, .type = long_word{} });
    result += '\n';

    result += pretty_print(mov_instruction{ .src = stack{ -40 }, .dst = immediate{ 55 }, .type = long_word{} });
    result += '\n';
    result += pretty_print(mov_instruction{ .src = stack{ -40 }, .dst = R10{}, .type = quad_word{} });
    result += '\n';
    result += pretty_print(mov_instruction{ .src = stack{ -40 }, .dst = pseudo{ "pseu_dst" }, .type = long_word{} });
    result += '\n';
    result += pretty_print(mov_instruction{ .src = stack{ -40 }, .dst = stack{ -32 }, .type = long_word{} });
    result += '\n';
    result += pretty_print(mov_instruction{ .src = stack{ -40 }, .dst = data{ "data_dst" }, .type = long_word{} });
    result += '\n';

    result += pretty_print(mov_instruction{ .src = data{ "data_src" }, .dst = immediate{ 55 }, .type = long_word{} });
    result += '\n';
    result += pretty_print(mov_instruction{ .src = data{ "data_src" }, .dst = R10{}, .type = quad_word{} });
    result += '\n';
    result += pretty_print(mov_instruction{ .src = data{ "data_src" }, .dst = pseudo{ "dst" }, .type = long_word{} });
    result += '\n';
    result += pretty_print(mov_instruction{ .src = data{ "data_src" }, .dst = stack{ -32 }, .type = long_word{} });
    result += '\n';
    result += pretty_print(
      mov_instruction{ .src = data{ "data_src" }, .dst = data{ "data_dst" }, .type = long_word{} });
    result += '\n';

    // mov_zero_extend
    result += "--mov_zero_extend-\n";
    result += pretty_print(mov_zero_extend{ .src = immediate{ 42 }, .dst = immediate{ 55 } });
    result += '\n';
    result += pretty_print(mov_zero_extend{ .src = immediate{ 42 }, .dst = R10{} });
    result += '\n';
    result += pretty_print(mov_zero_extend{ .src = immediate{ 42 }, .dst = pseudo{ "pseu_dst" } });
    result += '\n';
    result += pretty_print(mov_zero_extend{ .src = immediate{ 42 }, .dst = stack{ -32 } });
    result += '\n';
    result += pretty_print(mov_zero_extend{ .src = immediate{ 42 }, .dst = data{ "data_dst" } });
    result += '\n';

    result += pretty_print(mov_zero_extend{ .src = R10{}, .dst = immediate{ 55 } });
    result += '\n';
    result += pretty_print(mov_zero_extend{ .src = R10{}, .dst = R10{} });
    result += '\n';
    result += pretty_print(mov_zero_extend{ .src = R10{}, .dst = pseudo{ "pseu_dst" } });
    result += '\n';
    result += pretty_print(mov_zero_extend{ .src = R10{}, .dst = stack{ -32 } });
    result += '\n';
    result += pretty_print(mov_zero_extend{ .src = R10{}, .dst = data{ "data_dst" } });
    result += '\n';

    result += pretty_print(mov_zero_extend{ .src = pseudo{ "src" }, .dst = immediate{ 55 } });
    result += '\n';
    result += pretty_print(mov_zero_extend{ .src = pseudo{ "src" }, .dst = R10{} });
    result += '\n';
    result += pretty_print(mov_zero_extend{ .src = pseudo{ "src" }, .dst = pseudo{ "pseu_dst" } });
    result += '\n';
    result += pretty_print(mov_zero_extend{ .src = pseudo{ "src" }, .dst = stack{ -32 } });
    result += '\n';
    result += pretty_print(mov_zero_extend{ .src = pseudo{ "src" }, .dst = data{ "data_dst" } });
    result += '\n';

    result += pretty_print(mov_zero_extend{ .src = stack{ -40 }, .dst = immediate{ 55 } });
    result += '\n';
    result += pretty_print(mov_zero_extend{ .src = stack{ -40 }, .dst = R10{} });
    result += '\n';
    result += pretty_print(mov_zero_extend{ .src = stack{ -40 }, .dst = pseudo{ "pseu_dst" } });
    result += '\n';
    result += pretty_print(mov_zero_extend{ .src = stack{ -40 }, .dst = stack{ -32 } });
    result += '\n';
    result += pretty_print(mov_zero_extend{ .src = stack{ -40 }, .dst = data{ "data_dst" } });
    result += '\n';

    result += pretty_print(mov_zero_extend{ .src = data{ "data_src" }, .dst = immediate{ 55 } });
    result += '\n';
    result += pretty_print(mov_zero_extend{ .src = data{ "data_src" }, .dst = R10{} });
    result += '\n';
    result += pretty_print(mov_zero_extend{ .src = data{ "data_src" }, .dst = pseudo{ "dst" } });
    result += '\n';
    result += pretty_print(mov_zero_extend{ .src = data{ "data_src" }, .dst = stack{ -32 } });
    result += '\n';
    result += pretty_print(mov_zero_extend{ .src = data{ "data_src" }, .dst = data{ "data_dst" } });
    result += '\n';

    // movx
    result += "--movx-\n";
    result += pretty_print(movx{ .src = immediate{ 42 }, .dst = immediate{ 55 } });
    result += '\n';
    result += pretty_print(movx{ .src = immediate{ 42 }, .dst = R10{} });
    result += '\n';
    result += pretty_print(movx{ .src = immediate{ 42 }, .dst = pseudo{ "pseu_dst" } });
    result += '\n';
    result += pretty_print(movx{ .src = immediate{ 42 }, .dst = stack{ -32 } });
    result += '\n';
    result += pretty_print(movx{ .src = immediate{ 42 }, .dst = data{ "data_dst" } });
    result += '\n';

    result += pretty_print(movx{ .src = R10{}, .dst = immediate{ 55 } });
    result += '\n';
    result += pretty_print(movx{ .src = R10{}, .dst = R10{} });
    result += '\n';
    result += pretty_print(movx{ .src = R10{}, .dst = pseudo{ "pseu_dst" } });
    result += '\n';
    result += pretty_print(movx{ .src = R10{}, .dst = stack{ -32 } });
    result += '\n';
    result += pretty_print(movx{ .src = R10{}, .dst = data{ "data_dst" } });
    result += '\n';

    result += pretty_print(movx{ .src = pseudo{ "src" }, .dst = immediate{ 55 } });
    result += '\n';
    result += pretty_print(movx{ .src = pseudo{ "src" }, .dst = R10{} });
    result += '\n';
    result += pretty_print(movx{ .src = pseudo{ "src" }, .dst = pseudo{ "pseu_dst" } });
    result += '\n';
    result += pretty_print(movx{ .src = pseudo{ "src" }, .dst = stack{ -32 } });
    result += '\n';
    result += pretty_print(movx{ .src = pseudo{ "src" }, .dst = data{ "data_dst" } });
    result += '\n';

    result += pretty_print(movx{ .src = stack{ -40 }, .dst = immediate{ 55 } });
    result += '\n';
    result += pretty_print(movx{ .src = stack{ -40 }, .dst = R10{} });
    result += '\n';
    result += pretty_print(movx{ .src = stack{ -40 }, .dst = pseudo{ "pseu_dst" } });
    result += '\n';
    result += pretty_print(movx{ .src = stack{ -40 }, .dst = stack{ -32 } });
    result += '\n';
    result += pretty_print(movx{ .src = stack{ -40 }, .dst = data{ "data_dst" } });
    result += '\n';

    result += pretty_print(movx{ .src = data{ "data_src" }, .dst = immediate{ 55 } });
    result += '\n';
    result += pretty_print(movx{ .src = data{ "data_src" }, .dst = R10{} });
    result += '\n';
    result += pretty_print(movx{ .src = data{ "data_src" }, .dst = pseudo{ "dst" } });
    result += '\n';
    result += pretty_print(movx{ .src = data{ "data_src" }, .dst = stack{ -32 } });
    result += '\n';
    result += pretty_print(movx{ .src = data{ "data_src" }, .dst = data{ "data_dst" } });
    result += '\n';

    // operand
    result += "--operand--\n";
    result += pretty_print(wccff::assembly_generation::operand{ immediate{ 42 } });
    result += '\n';
    result += pretty_print(wccff::assembly_generation::operand{ R10{} });
    result += '\n';
    result += pretty_print(wccff::assembly_generation::operand{ pseudo{ "pseudo_name" } });
    result += '\n';
    result += pretty_print(wccff::assembly_generation::operand{ stack{ 42 } });
    result += '\n';
    result += pretty_print(wccff::assembly_generation::operand{ data{ "data_name" } });
    result += '\n';

    // pseudo
    result += "--pseudo--\n";
    result += pretty_print(pseudo{ "pseudo_name" });
    result += '\n';

    // push
    result += "--push--\n";
    result += pretty_print(wccff::assembly_generation::push{ immediate{ 42 } });
    result += '\n';
    result += pretty_print(wccff::assembly_generation::push{ R10{} });
    result += '\n';
    result += pretty_print(wccff::assembly_generation::push{ pseudo{ "pseudo_name" } });
    result += '\n';
    result += pretty_print(wccff::assembly_generation::push{ stack{ 42 } });
    result += '\n';
    result += pretty_print(wccff::assembly_generation::push{ data{ "data_name" } });
    result += '\n';

    // reg
    result += "--reg--\n";
    result += pretty_print(wccff::assembly_generation::reg{ wccff::assembly_generation::ax{} });
    result += '\n';
    result += pretty_print(wccff::assembly_generation::reg{ wccff::assembly_generation::cx{} });
    result += '\n';
    result += pretty_print(wccff::assembly_generation::reg{ wccff::assembly_generation::dx{} });
    result += '\n';
    result += pretty_print(wccff::assembly_generation::reg{ wccff::assembly_generation::di{} });
    result += '\n';
    result += pretty_print(wccff::assembly_generation::reg{ wccff::assembly_generation::si{} });
    result += '\n';
    result += pretty_print(wccff::assembly_generation::reg{ wccff::assembly_generation::R8{} });
    result += '\n';
    result += pretty_print(wccff::assembly_generation::reg{ wccff::assembly_generation::R9{} });
    result += '\n';
    result += pretty_print(wccff::assembly_generation::reg{ wccff::assembly_generation::R10{} });
    result += '\n';
    result += pretty_print(wccff::assembly_generation::reg{ wccff::assembly_generation::R11{} });
    result += '\n';
    result += pretty_print(wccff::assembly_generation::reg{ wccff::assembly_generation::SP{} });
    result += '\n';

    // ret
    result += "--ret--\n";
    result += pretty_print(wccff::assembly_generation::ret_instruction{});
    result += '\n';

    // top_level
    result += "--top_level--\n";
    result += pretty_print(wccff::assembly_generation::top_level{ function{ .name = "identifier_name",
                                                                            .is_global = false,
                                                                            .stack_size = 56,
                                                                            .instructions = generate_func_body() } });
    result += '\n';
    result += pretty_print(static_variable{ .name = identifier{ "foo1" },
                                            .is_global = false,
                                            .alignment = 4,
                                            .init = wccff::int_initial{ 43 } });
    result += '\n';

    // setcc
    result += "--setcc--\n";
    result += pretty_print(wccff::assembly_generation::setcc{ .cond = E{}, .dst = immediate{ 16 } });
    result += '\n';
    result += pretty_print(wccff::assembly_generation::setcc{ .cond = NE{}, .dst = R10{} });
    result += '\n';
    result += pretty_print(wccff::assembly_generation::setcc{ .cond = G{}, .dst = pseudo{ "pseudo_name" } });
    result += '\n';
    result += pretty_print(wccff::assembly_generation::setcc{ .cond = GE{}, .dst = stack{ 42 } });
    result += '\n';
    result += pretty_print(wccff::assembly_generation::setcc{ .cond = L{}, .dst = data{ "data_name" } });
    result += '\n';
    result += pretty_print(wccff::assembly_generation::setcc{ .cond = LE{}, .dst = data{ "data_name" } });
    result += '\n';
    result += pretty_print(wccff::assembly_generation::setcc{ .cond = A{}, .dst = pseudo{ "pseudo_name" } });
    result += '\n';
    result += pretty_print(wccff::assembly_generation::setcc{ .cond = AE{}, .dst = stack{ 42 } });
    result += '\n';
    result += pretty_print(wccff::assembly_generation::setcc{ .cond = B{}, .dst = data{ "data_name" } });
    result += '\n';
    result += pretty_print(wccff::assembly_generation::setcc{ .cond = BE{}, .dst = data{ "data_name" } });
    result += '\n';

    // stack
    result += "--stack--\n";
    result += pretty_print(stack{ immediate{ 16 } });
    result += '\n';

    // static_variable
    result += "--static_variable--\n";
    result += pretty_print(static_variable{ .name = identifier{ "foo1" },
                                            .is_global = false,
                                            .alignment = 4,
                                            .init = wccff::int_initial{ 43 } });
    result += '\n';
    result += pretty_print(static_variable{ .name = identifier{ "foo2" },
                                            .is_global = true,
                                            .alignment = 8,
                                            .init = wccff::long_initial{ 55 } });
    result += '\n';

    // std::vector<instruction>
    result += "--std::vector<instruction>--\n";
    std::vector<wccff::assembly_generation::instruction> instructions;
    instructions.emplace_back(wccff::assembly_generation::ret_instruction{});
    instructions.emplace_back(wccff::assembly_generation::ret_instruction{});
    result += pretty_print(instructions);
    result += '\n';

    // unary
    result += "--unary--\n";
    result += pretty_print(unary{ .op = not_op{}, .dst = immediate{ 16 }, .type = long_word{} });
    result += '\n';
    result += pretty_print(unary{ .op = neg_op{}, .dst = R10{}, .type = quad_word{} });
    result += '\n';
    result += pretty_print(unary{ .op = not_op{}, .dst = pseudo{ "pseudo_name" }, .type = long_word{} });
    result += '\n';
    result += pretty_print(unary{ .op = neg_op{}, .dst = stack{ 16 }, .type = quad_word{} });
    result += '\n';
    result += pretty_print(unary{ .op = not_op{}, .dst = data{ "pseudo_name" }, .type = long_word{} });
    result += '\n';

    // unary_operator
    result += "--unary_operator--\n";
    result += pretty_print(wccff::assembly_generation::unary_operator{ neg_op{} });
    result += '\n';
    result += pretty_print(wccff::assembly_generation::unary_operator{ not_op{} });
    result += '\n';

    ApprovalTests::Approvals::verify(result);
}
