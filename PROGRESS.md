# tengrad-c: Progress

## Current status
Step 2 (tensor design) complete. Design only: no tensor code exists yet.
CI green on first run (commit 2c8f059).

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

## API snapshot
- `<tg/version.h>`: `const char *tg_version(void)` returns "0.1.0"
  (static string, never freed or modified by the caller).

## Known issues / tech debt
- CI does not set `TENGRAD_WERROR=ON`, so warnings do not fail CI.
- The valgrind job checks nothing yet (0 allocations). It becomes useful
  once storage allocates.
- Harness limit: one .c file per test executable (static counter).
  The `REQUIRE` failure path has not been exercised.
- No `tengrad.h` umbrella header yet.
- The version string is hard-coded in `src/version.c` and the test.
- `.clangd` and `.clang-format` were never reviewed by the mentor.
- clang-format is not checked in CI.
- Stale numbered comments in the sanitizer block of `CMakeLists.txt`.
- Rank is capped at 8 axes (deliberate, see Decisions).
- The refcount will start as a plain non-atomic integer, so sharing tensors
  across threads is not safe. Needs a plan before we add threading.

## Next step
Step 3: storage. A float32 block with a refcount: create, retain, release,
plus tests (and a check you have seen fail). Valgrind finally has
allocations to watch.

## Open questions
- Error handling convention for allocation failure and invalid arguments
  (NULL return, error codes, something else).
- Alignment of the data block (matters for SIMD later).
- Is storage public API or private to the library?
