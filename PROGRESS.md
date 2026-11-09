# tengrad-c: Progress

## Current status
Step 1 (project skeleton) complete. Nothing tensor-related exists yet.
CI green on first run (commit 2c8f059)

## Completed steps
### Step 1: Skeleton
Built: CMake + Ninja build, static library `tengrad`, `tg_version()`, smoke
example, own test harness, CI (gcc and clang, Debug+sanitizers and Release,
plus a separate valgrind job), `.clang-format` and `.clangd`.

Concepts learned:
- CMake targets, and INTERFACE libraries as shared bundles of flags.
- CMake treats an undefined variable as false, silently. A typo in an
  `if()` disables a feature with no error, so check the real compiler
  lines with `cmake --build <dir> --clean-first -- -v`.
- The default C standard depends on the compiler (GCC 15: gnu23).
  `CMAKE_C_EXTENSIONS OFF` does nothing without `CMAKE_C_STANDARD`.
- `static` at file scope gives internal linkage (a private copy per .c file).
- A green test proves nothing until you have seen the same check fail.

## Decisions and why
- C17 with extensions off (`-std=c17`): it is C11 plus bug fixes, and every
  compiler implements it fully. C23 rejected: uneven compiler support.
- Warnings always on (`-Wall -Wextra -Wpedantic -Wconversion -Wshadow`).
  `-Werror` is opt-in via `TENGRAD_WERROR`. No option to disable warnings.
- Sanitizers: the `tengrad_sanitizers` target always exists and is empty
  unless `TENGRAD_SANITIZE=ON`. Every target links it unconditionally, so
  no per-target `if()` can be mistyped or forgotten.
- Test harness: one static failure counter, tests are `void` functions,
  `REQUIRE` does a plain `return;`. Alternatives rejected: `exit()` in
  `REQUIRE`, and passing `int *failed` into every helper.
- Public headers live in `include/tg/`. Build dirs: build-deb, build-rel,
  build-asan.

## API snapshot
- `<tg/version.h>`: `const char *tg_version(void)` returns "0.1.0"
  (static string, never freed or modified by the caller).

## Known issues / tech debt
- CI does not set `TENGRAD_WERROR=ON`, so warnings do not fail CI.
- The valgrind job checks nothing yet (0 allocations). It becomes useful
  once tensors allocate.
- Harness limit: one .c file per test executable (static counter).
  The `REQUIRE` failure path has not been exercised.
- No `tengrad.h` umbrella header yet.
- The version string is hard-coded in `src/version.c` and the test.
- `.clangd` and `.clang-format` were never reviewed by the mentor.
- clang-format is not checked in CI.
- Stale numbered comments in the sanitizer block of `CMakeLists.txt`.

## Next step
Step 2: what a tensor is (n-dimensional array, shape, strides), then decide
the dtype strategy and memory ownership model before writing tensor code.

## Open questions
- dtype strategy: float32 only at first, or several dtypes?
- Ownership model for tensor memory.
