# tengrad-c: Progress

## Current status

Step 3 (storage) complete: a refcounted float32 block, private to the
library, with tests under ASan and valgrind. No tensor code exists yet.

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

### Step 2: Tensor design (no code)

Decided how a tensor is laid out in memory and who owns that memory.

Concepts learned:

- A tensor is a flat contiguous block of numbers plus a shape and strides.
  Rank (ndim) = number of axes. Element count = product of the shape.
- Row-major strides: last axis is 1, each earlier axis is the next stride
  times the next size. `offset = sum(i_k * stride_k)` counts elements;
  byte offset = element offset * sizeof(element).
- A view is a new shape and strides over the same memory. Each axis carries
  its own (size, stride) pair. Transpose reorders the pairs; reshape of a
  contiguous tensor uses fresh row-major strides. The strides decide what
  the view shows.
- Contiguous = strides equal the row-major strides of the shape. Views can
  be non-contiguous, which makes reads jump around memory and is slower.
  (Axes of size 1 are an edge case to handle when we write the check.)
- Views need shared ownership, so the refcount belongs to the memory block,
  not to any tensor.
- Unsigned arithmetic wraps silently (`2 - 3 == SIZE_MAX`). Signed overflow
  is undefined behavior.

### Step 3: Storage

Built: `src/storage.h` and `src/storage.c` (`tg_storage` with `create`,
`retain`, `release`), `include/tg/status.h` (`tg_status`), and
`tests/test_storage.c` (success, invalid arguments, retain/release). The
valgrind CI job now runs `test_storage` too.

Concepts learned:

- Refcount lifecycle: create gives count 1, each extra holder retains, each
  holder releases exactly once, and the last release frees. Allocation
  happens only in create.
- Double release or retain after free is a dangling-pointer bug. A NULL
  check cannot see it, and the refcount cannot detect it. It is undefined
  behavior, caught only by ASan and valgrind in tests.
- An object whose lifetime is decided by a refcount must be heap-allocated
  by the code that manages the count. A stack struct dies with its
  function while other holders still point at it.
- Out-parameter rule: build the result in a local variable and write
  `*out` once, on the success path, as the last step. Failure paths then
  need no undo, and the caller's pointer is never clobbered.
- Release only what a successful create returned.
- Overflow check before multiplying: `n > SIZE_MAX / sizeof(float)`.
- Reading sanitizer reports: UBSan names the cause, ASan the consequence,
  stack traces read top-down from the crash site. LeakSanitizer runs at
  exit, so tests can pass while the process fails. Valgrind runs an
  unmodified binary, shows the full call chain per leak, and counts
  allocs against frees.
- A leaked block is still readable, so no CHECK can detect it. Only an
  allocator-watching tool can.
- Include what you use. Relying on another header's includes breaks
  silently when that header changes.

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
- dtype: float32 only for the first version. "General-purpose" means many
  model types, not many number formats, and every extra dtype multiplies
  the code and tests of every operation. Cost: no native float64 or uint8
  (gradient checks will use float32 tolerances), and adding dtypes later is
  a real refactor. Data access is type-suffixed (`tg_tensor_data_f32`) so a
  new dtype adds functions instead of breaking existing ones.
- Ownership: tensors share a `storage` that owns the data block and a
  refcount. A tensor holds shape, strides and a storage pointer. Freeing a
  tensor decrements the count; at 0 it frees the data block, then the
  storage struct; the tensor struct itself is always freed.
- Shape and strides live inline in the tensor struct, `TG_MAX_DIMS` = 8.
  More axes is an error at creation, never an overflow. Rejected:
  separately malloc'd arrays (no cap, but up to 3 allocations per tensor
  and harder failure handling). PyTorch uses a hybrid (inline up to 5, heap
  beyond); revisit only if the cap hurts in real use.
- `tg_tensor` is an opaque type: the public header only declares it, the
  struct lives in a private header, and users go through functions. This
  lets us change the layout (e.g. to the hybrid) without breaking users or
  contributors. Cost: a function call per access, no stack-allocated tensors.
- Shape, stride and element-count type: `int64_t`. Rejected `int` (overflows
  past ~2.1 billion elements) and `size_t` (unsigned wraparound, cannot
  represent -1). Cast to `size_t` deliberately in one place for allocation
  sizes, and check the shape product for overflow at creation.
- Error handling: every fallible function returns `tg_status`
  (`TG_OK`, `TG_ERR_INVALID_ARG`, `TG_ERR_NO_MEMORY`) and hands results back
  through an out-parameter. Rejected: NULL return (cannot say why) and a
  global or thread-local last-error (hidden state, can be overwritten,
  easy to ignore). Cost: call sites cannot be chained as expressions;
  revisit the ergonomics when we reach tensor operations.
- Storage is private to the library (`src/storage.h`), not public API.
  Users only see tensors. Anything public must stay stable forever, and
  promoting it later is easy. Its struct is not opaque: the header is
  private, so there is nobody to hide the layout from, and hot paths need
  direct field access. Tests reach it through
  `target_include_directories(test_storage PRIVATE src/)`, not by widening
  the library's public include path.
- `tg_storage_create(tg_storage **out, size_t element_count)` allocates the
  struct and the data block. Count 0 or NULL `out` is `TG_ERR_INVALID_ARG`.
  A byte size that overflows is `TG_ERR_NO_MEMORY` (no separate code). On
  failure `*out` is untouched and nothing leaks. The data is uninitialized.
- `retain` and `release` return `void` and treat NULL as a no-op, like
  `free(NULL)`: nothing can fail inside them, and it keeps cleanup paths
  simple. They do not null the caller's pointer: with shared ownership
  other variables would still dangle.
- `refcount` is `int64_t` (signed, so an over-release can be asserted
  against), `element_count` is `size_t`.
- The refcount is a plain integer for now. Atomics are deferred until
  there is threading to justify their cost.

## API snapshot

- `<tg/version.h>`: `const char *tg_version(void)` returns "0.1.0"
  (static string, never freed or modified by the caller).
- `<tg/status.h>`: `tg_status` enum: `TG_OK`, `TG_ERR_INVALID_ARG`,
  `TG_ERR_NO_MEMORY`.
- Private, `src/storage.h`: `tg_storage` (data, element_count, refcount),
  `tg_storage_create`, `tg_storage_retain`, `tg_storage_release`.

## Known issues / tech debt

- CI does not set `TENGRAD_WERROR=ON`, so warnings do not fail CI.
- Harness limit: one .c file per test executable (static counter), and a
  crash in one test hides every test after it. The `REQUIRE` failure path
  has not been exercised.
- The data-allocation failure path in `create` (free the struct, return
  `TG_ERR_NO_MEMORY`) is untested: it needs a way to make `malloc` fail on
  demand.
- `release` has no debug `assert(refcount > 0)`, so an over-release is
  silent until it corrupts something.
- `storage.c` uses `SIZE_MAX` but does not include `<stdint.h>` itself; it
  only compiles because `storage.h` happens to include it.
- No `tengrad.h` umbrella header yet.
- The version string is hard-coded in `src/version.c` and the test.
- `.clangd` and `.clang-format` were never reviewed by the mentor.
- clang-format is not checked in CI.
- Stale numbered comments in the sanitizer block of `CMakeLists.txt`.
- Rank is capped at 8 axes (deliberate, see Decisions).
- The refcount is not atomic, so sharing tensors across threads is not
  safe. Plan: C11 `<stdatomic.h>`, and `release` must decrement and test
  for 0 as one atomic operation. Needs doing before we add threading.

## Next step

Step 4: the tensor struct and creation. A private `tg_tensor` (shape,
strides, storage pointer), creation from a shape with validation (rank
cap, no negative sizes, overflow-checked element count), row-major
strides, and free. Design questions first, code after.

## Open questions

- Alignment of the data block (matters for SIMD later).
- Whether the public tensor API keeps out-parameters or gets a friendlier
  shape once we feel the call-site cost.
