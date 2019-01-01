#include <stdlib.h>

#include <skull/unittest.h>

/**
 * Basic Unit Test Rules for skull project:
 * 1. Test the IDL transaction data
 * 2. Test the important algorithm
 * 3. DO NOT Test log content, since it's inconstant and FT may covered it
 * 4. DO NOT Test metrics, since FT may covered it
 */

static
void test_case1()
{
}

int main(int argc, char** argv)
{
    SKULL_CUNIT_RUN(test_case1);
    return 0;
}
