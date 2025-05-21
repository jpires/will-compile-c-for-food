long explicit_cast_of_variable(int a)
{
    return (long)a;
}

long explicit_cast_of_expression(int a, int b)
{
    return (long)(a + b);
}

long explicit_cast_of_expression1(int a, int b)
{
    return (long)a + b;
}

long explicit_cast_of_expression3(int a, int b)
{
    return a + (long)b;
}

long explicit_cast_of_function_call(int a)
{
    return (long)explicit_cast_of_variable(a);
}
