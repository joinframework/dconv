---
name: conformance
description: Check dtoa/atod correctness versus main with the dconv-benchmark conformance suite. Use at closing when conversion code changed.
---

# Conformance

Required at closing when conversion code changes (`dconv/include/dconv/`). No timing: any machine works. For speed see skill `performance`.

Ask where `dconv-benchmark` is cloned (`https://github.com/joinframework/dconv-benchmark`); never clone it into the dconv tree. It pins dconv to a release tag, so point both builds at local checkouts, `main` in a worktree:

```bash
git -C <dconv> worktree add --detach <scratchpad>/base main
cmake -S <bench> -B <bench>/build/main   -DCMAKE_BUILD_TYPE=Release -DFETCHCONTENT_SOURCE_DIR_DCONV=<scratchpad>/base
cmake -S <bench> -B <bench>/build/branch -DCMAKE_BUILD_TYPE=Release -DFETCHCONTENT_SOURCE_DIR_DCONV=<dconv>
cmake --build <build> --target conformance
<build>/benchmarks/conformance/conformance --output <scratchpad>/conformance-<main|branch>.json
```

Remove the worktree at the end (`git worktree remove --force`), even on failure.

## Pass criteria

Compare the `dconv*` rows of both runs (the other libraries are context):

- **Dtoa Roundtrip**: 100% for every dconv algorithm.
- **Dtoa Shortest**: 100% for Schubfach and Grisu3; Grisu2 and Grisu must not drop.
- **Atod Exact**: 100%.

Any drop fails. Report both tables and, for a failure, a failing input with the expected and actual output.

The suite only calls `dconv::atod (first, last, value)` (`StringProbe`): `StreamProbe` and the `std::istream` overloads are covered only by the gtests; say so in the report.
