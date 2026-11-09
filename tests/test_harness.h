#ifndef TG_TEST_HARNESS_H
#define TG_TEST_HARNESS_H

#include <stdio.h>  /* IWYU pragma: keep */
#include <string.h> /* IWYU pragma: keep */

static int tg_test_failures = 0;

/**
 * @brief Evaluates the `tg_test_failures` counter and returns the appropriate process exit code.
 * @note Returns 0 (Success) if no failures occurred, or 1 (Failure) if any check failed.
 */
#define TEST_STATUS (tg_test_failures > 0 ? 1 : 0)

/**
 * @brief Asserts that a condition is true.
 * @note This variable `tg_test_failures` will be incremented by 1 if the assertion fails.
 *
 * @param cond The boolean expression to evaluate.
 */
#define CHECK(cond)                                                                                \
    do {                                                                                           \
        if (!(cond)) {                                                                             \
            fprintf(stderr, "CHECK failed at %s:%d\n", __FILE__, __LINE__);                        \
            fprintf(stderr, "  Expression: %s\n", #cond);                                          \
            tg_test_failures++;                                                                    \
        }                                                                                          \
    } while (0)

/**
 * @brief Asserts a critical condition is true. Aborts the current function on failure.
 */
#define REQUIRE(cond)                                                                              \
    do {                                                                                           \
        if (!(cond)) {                                                                             \
            fprintf(stderr, "REQUIRE failed (FATAL) at %s:%d\n", __FILE__, __LINE__);              \
            fprintf(stderr, "  Expression: %s\n", #cond);                                          \
            tg_test_failures++;                                                                    \
            return; /* Bails out immediately, preserving the failure state */                      \
        }                                                                                          \
    } while (0)

/**
 * @brief Asserts that two C-strings are structurally equal.
 * @note This variable `tg_test_failures` will be incremented by 1 if the assertion fails.
 * @note Uses ultra-unique suffixes to entirely eliminate the risk of -Wshadow variable collision.
 *
 * @param a The expected string (handles NULL safely).
 * @param b The actual string (handles NULL safely).
 */
#define CHECK_STR_EQ(a, b)                                                                         \
    do {                                                                                           \
        const char *str_a_cseq_7x8 = (a);                                                          \
        const char *str_b_cseq_7x8 = (b);                                                          \
        if (str_a_cseq_7x8 == NULL || str_b_cseq_7x8 == NULL) {                                    \
            if (str_a_cseq_7x8 != str_b_cseq_7x8) {                                                \
                fprintf(stderr, "CHECK_STR_EQ failed at %s:%d\n", __FILE__, __LINE__);             \
                fprintf(stderr, "  Expected: %s\n", str_a_cseq_7x8 ? str_a_cseq_7x8 : "NULL");     \
                fprintf(stderr, "  Actual:   %s\n", str_b_cseq_7x8 ? str_b_cseq_7x8 : "NULL");     \
                tg_test_failures++;                                                                \
            }                                                                                      \
        } else if (strcmp(str_a_cseq_7x8, str_b_cseq_7x8) != 0) {                                  \
            fprintf(stderr, "CHECK_STR_EQ failed at %s:%d\n", __FILE__, __LINE__);                 \
            fprintf(stderr, "  Expected: \"%s\"\n", str_a_cseq_7x8);                               \
            fprintf(stderr, "  Actual:   \"%s\"\n", str_b_cseq_7x8);                               \
            tg_test_failures++;                                                                    \
        }                                                                                          \
                                                                                                   \
    } while (0)

#endif /* TG_TEST_HARNESS_H */
