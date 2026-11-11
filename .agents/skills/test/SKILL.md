---
name: test
description: Run dconv tests (ctest or one gtest), including sanitizers. Use to validate a change or reproduce a failure.
---

# Test

Build first with skill `compile`. Dev loop vs closing: see `AGENTS.md` "Definition of done".

```bash
ctest --preset <preset> --no-tests=error [-R '^<Suite>\.']   # Suite = first TEST argument, e.g. StreamProbe, schubfach
build/<compiler>/<kind>/dconv/tests/<name>.gtest --gtest_filter='suite.test'
```

- `debug` and `release` on gcc and clang, like CI. `release` (`-O3`) can expose what `debug` hides.
- `asan` (ASan + UBSan, `detect_leaks=1`) for buffer, range and UB issues; the `asan` test presets set `ASAN_OPTIONS`/`UBSAN_OPTIONS` (export the same values when running a binary directly).
- A failing conversion: print the input and the bits of the double (`%a`) before changing code.

## Report

Presets run and pass/fail counts, with the output of any failure. Never claim success you did not observe.
