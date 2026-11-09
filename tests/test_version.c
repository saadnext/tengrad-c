#include "test_harness.h"
#include <tg/version.h>

static void test_version_string(void) {
    const char *version_call1 = tg_version();
    const char *expected_version = "0.1.0";

    REQUIRE(version_call1 != NULL);

    CHECK(strlen(version_call1) != 0);
    CHECK_STR_EQ(expected_version, version_call1);

    const char *version_call2 = tg_version();
    CHECK(version_call1 == version_call2);
}

int main(void) {
    test_version_string();
    return TEST_STATUS;
}
