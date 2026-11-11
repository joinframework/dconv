---
name: coverage
description: Check dconv coverage is not degraded versus main (gcc and clang). Use at closing.
---

# Coverage

Coverage must not drop on **gcc and clang**. Use the `debug` presets (only ones with coverage) and CI's `gcovr` options:

```bash
C=gcc    # repeat with C=clang
GCOV=gcov; [ "$C" = clang ] && GCOV='llvm-cov gcov'
cmake --preset $C-debug && cmake --build --preset $C-debug
find build/$C/debug -name '*.gcda' -delete
ctest --preset $C-debug
gcovr --root . --gcov-executable "$GCOV" --exclude-pattern-prefix LCOV --exclude-branches-by-pattern '.*' \
      --exclude '.*/tests/.*' --exclude 'tools/.*' --json-summary <scratchpad>/$C-after.json --print-summary build/$C/debug
```

Baseline: same steps on `main` in a worktree (`git worktree add --detach <scratchpad>/base main`) into `$C-before.json`; remove it at the end (`git worktree remove --force`), even on failure.

Compare the total and every changed header. Any drop fails: add tests (or `LCOV_EXCL_*` for truly unreachable code, sparingly) and re-measure.

Needs `gcovr` and `llvm-cov` matching clang. Without them locally, run the steps above in CI's image (`docker pull joinframework/join-ci:latest` first if it lacks `gcovr`), on copies so the host `build/` is untouched:

```bash
rsync -a --exclude build ./ <scratchpad>/head/
docker run --rm --user "$(id -u):$(id -g)" -v <scratchpad>:<scratchpad> -w <scratchpad>/head \
       joinframework/join-ci:latest bash -c '<steps above>'
```

Same for the baseline with `-w <scratchpad>/base`. If neither works, report coverage as **not verified**, never "not degraded".
