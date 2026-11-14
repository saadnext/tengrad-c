#include "storage.h"
#include "test_harness.h"
#include "tg/status.h"
#include <stdint.h>

static void test_create_success(void) {
    tg_storage *storage = NULL;
    REQUIRE(tg_storage_create(&storage, 16) == TG_OK);
    REQUIRE(storage != NULL);
    CHECK(storage->element_count == 16);
    CHECK(storage->refcount == 1);
    CHECK(storage->data != NULL);
    for (int i = 0; i < 16; ++i)
        storage->data[i] = (float)i;
    for (int i = 0; i < 16; ++i)
        CHECK(storage->data[i] == (float)i);
    tg_storage_release(storage);
}

static void test_create_invalid_args(void) {
    tg_storage dummy1;
    tg_storage *s1 = &dummy1;
    CHECK(tg_storage_create(&s1, 0) == TG_ERR_INVALID_ARG);
    CHECK(s1 == &dummy1);
    tg_storage dummy2;
    tg_storage *s2 = &dummy2;
    CHECK(tg_storage_create(&s2, SIZE_MAX / sizeof(float) + 1) == TG_ERR_NO_MEMORY);
    CHECK(s2 == &dummy2);
    CHECK(tg_storage_create(NULL, 16) == TG_ERR_INVALID_ARG);
}

static void test_retain_release(void) {
    tg_storage *storage = NULL;
    REQUIRE(tg_storage_create(&storage, 16) == TG_OK);
    REQUIRE(storage != NULL);
    for (int i = 0; i < 16; ++i)
        storage->data[i] = (float)i;
    tg_storage_retain(storage);
    CHECK(storage->refcount == 2);
    tg_storage_release(storage);
    CHECK(storage->refcount == 1);
    for (int i = 0; i < 16; ++i)
        CHECK(storage->data[i] == (float)i);
    tg_storage_release(storage);
    tg_storage_retain(NULL);
    tg_storage_release(NULL);
}

int main(void) {
    test_create_success();
    test_create_invalid_args();
    test_retain_release();
    return TEST_STATUS;
}
