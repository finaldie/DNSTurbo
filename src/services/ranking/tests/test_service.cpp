#include <stdlib.h>
#include <string.h>

#include <skullcpp/unittest.h>
#include "skull_protos.h"

/**
 * Basic Unit Test Rules for skull service:
 * 1. Given the api request and test the response data
 * 2. Test the important algorithm
 * 3. DO NOT Test the log content, since it's inconstant and FT may covered it
 * 4. DO NOT Test metrics, since FT may covered it
 * 5. DO NOT strive for 100% test coverage, set a meaningful goal, like 80%
 */

/**
 * The following is an example test case, we assume that there is a existing
 *  API 'get', then its message proto would be 'get_req' and 'get_resp', then
 *  we run this API, and expect the response contain the content what we want.
 *
 *  More information here: https://github.com/finaldie/skull/wiki/How-To-Test
 *
 * static
 * void test_example()
 * {
 *     // 1. Create a ut service env
 *     skullcpp::UTService utSvc("ranking", "tests/test_config.yaml");
 *
 *     // 2. Construct api request message
 *     skull::service::ranking::get_req  apiReq;
 *     skull::service::ranking::get_resp apiResp;
 *     apiReq.set_name("hello api");
 *
 *     // 3. Run service
 *     utSvc.run("get", apiReq, apiResp);
 *
 *     // 4. Validate api response data
 *     SKULL_CUNIT_ASSERT(apiResp.response() == "Hi new bie");
 * }
 */

static
void test_example() {
    // Write the test case logic here
}

int main(int argc, char** argv)
{
    SKULL_CUNIT_RUN(test_example);
    return 0;
}
