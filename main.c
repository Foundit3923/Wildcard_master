#include <stdio.h>
#include <string.h>
#include "unit_tests/legacy_tests.h"

int main (int arg_count, char *arg_values[])
{
    arg_count--; arg_values++; // Ignore first argument (./a.out)
    const char *UNIT_TEST_FLAG = "--run_tests";
    const char *BENCHMARK_FLAG = "--run_benchmarks";
    int return_code = 0;

    printf("Running Wildfilter tooling\n");
    if (arg_count <= 0)
    {
        printf("Not implemented: Default behavior with no arguments\n");
    }
    else if (strcomp(arg_values[0], UNIT_TEST_FLAG) == 0)
    {
        arg_count--; arg_values++;
        printf("Running unit tests. Remaining output managed by munit.\n");
        return_code = munit_suite_main(&legacy_test_suite, NULL, arg_count, arg_values);
    }
    else if (strcomp(arg_values[0], BENCHMARK_FLAG) == 0)
    {
        if (arg_count == 1)
        {
            printf("Not implemented: Default behavior for benchmarks\n");
        }
        else
        {
            printf("Not implemented: Benchmarks with algorithms specified\n");
        }
    }
    return return_code;
}