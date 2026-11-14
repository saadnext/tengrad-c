#include "storage.h"
#include "tg/status.h"
#include <stdint.h>
#include <stdlib.h>

tg_status tg_storage_create(tg_storage **storage, size_t element_count) {
    if (!storage || element_count == 0)
        return TG_ERR_INVALID_ARG;
    if (element_count > SIZE_MAX / sizeof(float))
        return TG_ERR_NO_MEMORY;
    tg_storage *s = malloc(sizeof(*s));
    if (!s)
        return TG_ERR_NO_MEMORY;
    s->data = malloc(element_count * sizeof(*s->data));
    if (!s->data) {
        free(s);
        return TG_ERR_NO_MEMORY;
    }
    s->element_count = element_count;
    s->refcount = 1;
    *storage = s;
    return TG_OK;
}

void tg_storage_retain(tg_storage *storage) {
    if (!storage)
        return;
    storage->refcount++;
}

void tg_storage_release(tg_storage *storage) {
    if (!storage)
        return;
    storage->refcount--;
    if (storage->refcount == 0) {
        free(storage->data);
        free(storage);
    }
}
