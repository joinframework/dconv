---
name: compile
description: Configure and build dconv with the CMake presets, as CI does. Use to compile a change or fix build errors.
---

# Compile

Dev loop vs closing: see `AGENTS.md` "Definition of done".

```bash
cmake --preset <preset>
cmake --build --preset <preset> [--target <name>.gtest]
```

Presets: `gcc-` / `clang-` + `debug` (coverage), `release`, `asan` (ASan + UBSan); trees in `build/<compiler>/<kind>`. Fix warnings (`-Werror`), never silence them.

## What CI builds on every PR

1. `clang-format` at the version pinned in `build.yml` over the tree (gates everything). Check `clang-format --version`: if it differs, ask the user to install it; a format diff seen only in CI points to the version gap.
2. gcc and clang x amd64 and arm64, in `joinframework/join-ci`: `release` then `debug`; `asan` on amd64.
3. CodeQL on `gcc-debug`.
4. `gcovr` coverage on amd64 (gcc and clang, kept as artifacts); only gcc's is uploaded to Coveralls/Codecov/Codacy by `deploy-coverage`, which can fail the run.

arm64 is not reproducible locally: flag endianness and `char` signedness assumptions. `__uint128_t` must stay behind `#if defined(__SIZEOF_INT128__)` with a portable `#else`; no CI job compiles that `#else` (gcc and clang define it on amd64 and arm64), so check it by reading the code. `__builtin_expect` via `DCONV_LIKELY` is accepted.

## Report

Presets built, with the first error or warning in full on failure.
