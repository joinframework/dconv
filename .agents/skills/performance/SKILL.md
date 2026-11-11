---
name: performance
description: Check dtoa/atod speed versus main with dconv-benchmark on a real machine. Use at closing when conversion code changed.
---

# Performance

Required at closing when conversion code changes (`dconv/include/dconv/`). Done means no slowdown of the dconv entries versus `main` beyond the run-to-run spread. If no suitable machine is available, report it as **not verified** (WSL2 numbers are indicative only).

## 1. Ask first

Ask the user: **is a real machine available (ideally x86_64 and aarch64)?** Clone `dconv-benchmark` there as in skill `conformance`, never into the dconv tree.

## 2. Performance mode

The manual runs below bypass `run_bench.sh`'s checks: verify yourself that `cpu<N>/cpufreq/scaling_governor` is `performance` and boost is off (`cpufreq/boost` = 0, or `intel_pstate/no_turbo` = 1). Read and note the current values, then:

```bash
echo performance | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor
echo 0 | sudo tee /sys/devices/system/cpu/cpufreq/boost              # AMD / acpi-cpufreq
echo 1 | sudo tee /sys/devices/system/cpu/intel_pstate/no_turbo    # Intel intel_pstate
```

Use `sudo` only when justified; otherwise give the commands with the `! ` prefix. No `ALLOW_NOISY=1` unless the user accepts noisy numbers. Remind the user of the original values at the end.

## 3. Compare

Same `build/main` / `build/branch` setup as skill `conformance` (local checkouts via `FETCHCONTENT_SOURCE_DIR_DCONV`, `main` in a worktree). Build `bench_dtoa_dconv` and `bench_atod_dconv` (+ `bench_dtoa_dconvgrisu{,2,3}` if Grisu changed).

Binaries: `<build>/benchmarks/performance/bench_*`. Export the same `GLIBC_TUNABLES` as `scripts/run_bench.sh`. Run them alternately main/branch, several times, `taskset -c 2`, with `--benchmark_min_warmup_time=0.1 --benchmark_min_time=0.2s`; keep the minimum. Datasets: `Canada`, `Random`, `Unit`, `Integer`, `Digits01..17`. Remove the worktree at the end, even on failure.

## 4. Report

Machine (CPU, arch, governor/boost, native or WSL2), repetitions, and a before/after table per dataset with the relative change. A change is real only beyond the spread; never claim an unmeasured gain.
