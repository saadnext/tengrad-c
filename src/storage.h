#ifndef TG_STORAGE_H
#define TG_STORAGE_H

#include <stddef.h>
#include <stdint.h>
#include <tg/status.h>

typedef struct {
    float *data;
    size_t element_count;
    int64_t refcount;
} tg_storage;

/**
 * Creates a storage: a block of `element_count` float32 values with a
 * reference count of 1.
 *
 * @param storage       Out-parameter. On success, receives the new storage.
 *                      Must not be NULL.
 * @param element_count Number of float32 elements. Must be greater than 0.
 *
 * @return TG_OK on success.
 *         TG_ERR_INVALID_ARG if `storage` is NULL or `element_count` is 0.
 *         TG_ERR_NO_MEMORY if the allocation fails, or if the block's size
 *         in bytes is too large to represent.
 *
 * @note Ownership:
 * - The caller receives the first holder and must release it exactly once
 *   with tg_storage_release.
 * - On failure, `*storage` is not modified and nothing is leaked.
 *
 * @note Contents: the data block is uninitialized. The caller must write
 *       every element before reading it.
 */
tg_status tg_storage_create(tg_storage **storage, size_t element_count);

/**
 * Adds one holder to a live storage.
 *
 * @param storage The storage to share. If NULL, does nothing.
 *
 * @note Ownership:
 * - Every retain must be matched by exactly one tg_storage_release.
 * - `storage` must still be alive. Retaining storage whose count already
 *   reached 0 is undefined behavior.
 * - Not thread-safe: the refcount is a plain integer.
 */
void tg_storage_retain(tg_storage *storage);

/**
 * Removes one holder. When the last holder is removed, frees the data block
 * and the storage itself.
 *
 * @param storage The storage to release. If NULL, does nothing.
 *
 * @note Ownership:
 * - Call exactly once per holder (one for create, one for each retain).
 * - After calling, the caller must not use this pointer again, even if
 *   other holders keep the storage alive. Releasing more times than there
 *   are holders is undefined behavior.
 * - Not thread-safe: the refcount is a plain integer.
 */
void tg_storage_release(tg_storage *storage);

#endif /* TG_STORAGE_H */
