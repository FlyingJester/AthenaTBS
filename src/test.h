#pragma once
#include <stdio.h>

/* A naively simple testing framework.
 *    I just want some tests for my program. Why should that be so difficult?
 */

typedef int(*Athena_TestFunction)(void);

struct Athena_Test{
    Athena_TestFunction function;
    const char * const name;
    unsigned enabled;
};

#define ATHENA_TEST(FUNC)\
{FUNC, #FUNC, 1}

#define ATHENA_DISABLED_TEST(FUNC)\
{FUNC, #FUNC, 0}

#define ATHENA_RUN_TEST(T, SUCESSES)\
if(!T.enabled){\
    puts("[Test]Disabled test:");\
    putchar('\t');\
    puts(T.name);\
}\
else if(!T.function()){\
    puts("[Test]Failed test:");\
    putchar('\t');\
    puts(T.name);\
}\
else do{\
    fputs("[Test]Passed:\t", stdout);\
    puts(T.name);\
    SUCESSES++;\
}while(0)

#define ATHENA_TEST_FUNCTION(FUNC_NAME, TEST_ARRAY)\
int FUNC_NAME(){\
    int i;\
    union {\
        unsigned u;\
        unsigned short s[2];\
    }results;\
    results.s[0] = 0;\
    results.s[1] = sizeof(TEST_ARRAY) / sizeof(TEST_ARRAY[0]);\
    for(i = 0; i<sizeof(TEST_ARRAY) / sizeof(TEST_ARRAY[0]); i++)\
        ATHENA_RUN_TEST(TEST_ARRAY[i], results.s[0]);\
    return results.u;\
}

#define ATHENA_RUN_TEST_SUITE(R)\
{\
    union {\
        unsigned u;\
        unsigned short s[2];\
    }results;\
    puts("========== " #R " ==========\n[Test]Beginnning " #R);\
    results.u = R();\
    printf("[Test]" #R " Results: %i/%i\n========== " #R\
        " ==========\n", results.s[0], results.s[1]);\
}

/*

An example of how to use these tests:

*//* Write a few tests. Returning 1 is a success, 0 is a failure. *//*
int Athena_TestWorks(){
    return 1;
}

int Athena_TestFails(){
    return 0;
}

*//* Create an array of the tests. *//*

static struct Athena_Test athena_tests[] = {
    ATHENA_TEST(Athena_TestWorks),
    ATHENA_TEST(Athena_TestFails)
};

*//* This creates the test function for this module. *//*
ATHENA_TEST_FUNCTION(Athena_Test, athena_tests)

*//* Call that function somewhere (The first argument to 
    ATHENA_TEST_FUNCTION is the name of the function). That runs the tests.*//*

#define ENABLE_TESTS
int main(){

#ifdef ENABLE_TESTS
*//* This should show one working test and one successful test. *//*
    Athena_Test();
#endif

}

*/
