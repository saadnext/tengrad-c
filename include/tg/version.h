#ifndef TG_VERSION_H
#define TG_VERSION_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Retrieves the current version identifier string.
 *
 * @return A non-NULL, null-terminated string representing the version.
 *
 * @note Lifetime & Ownership:
 * - The returned pointer has static lifetime and remains valid for the
 *   entire duration of the program.
 * - The caller MUST NOT attempt to modify the string or free the pointer.
 * - Thread-safe: Safe to call concurrently from multiple threads.
 */
const char *tg_version(void);

#ifdef __cplusplus
}
#endif

#endif /* TG_VERSION_H */
