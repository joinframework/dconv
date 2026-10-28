# dconv

[![Test Status](https://github.com/joinframework/dconv/actions/workflows/cd.yml/badge.svg?branch=main)](https://github.com/joinframework/dconv/actions?query=workflow%3Acd+branch%3Amain)
[![Security Status](https://github.com/joinframework/dconv/actions/workflows/audit.yml/badge.svg?branch=main)](https://github.com/joinframework/dconv/actions?query=workflow%3Aaudit+branch%3Amain)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/44b789b2a04c4f1c9720c6b3020dd769)](https://app.codacy.com/gh/joinframework/dconv/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)
[![Codacy Badge](https://app.codacy.com/project/badge/Coverage/44b789b2a04c4f1c9720c6b3020dd769)](https://app.codacy.com/gh/joinframework/dconv/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_coverage)
[![Codecov](https://codecov.io/gh/joinframework/dconv/branch/main/graph/badge.svg)](https://codecov.io/gh/joinframework/dconv)
[![Coveralls](https://coveralls.io/repos/github/joinframework/dconv/badge.svg?branch=main)](https://coveralls.io/github/joinframework/dconv?branch=main)
[![Doxygen](https://img.shields.io/badge/docs-doxygen-blue.svg)](https://joinframework.github.io/dconv/index.html)
[![GitHub Releases](https://img.shields.io/github/release/joinframework/dconv.svg)](https://github.com/joinframework/dconv/releases/latest)
[![GitHub License](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/joinframework/dconv/blob/main/LICENSE)

**dconv** is a header-only C++20 library for printing and parsing floating point numbers.

Double to string conversion is done using the **Grisu2** algorithm, described by **Florian Loitsch** in its publication [Printing Floating-Point Numbers Quickly and Accurately with Integers](https://florian.loitsch.com/publications).

String to double conversion uses the **Eisel-Lemire** algorithm, without fallback for mantissas of up to 19 digits; longer mantissas fall back to strtod.

Any help to improve speed, accuracy or code quality is welcome.

## Build

### Prerequisites

Install build tools, compilers and test dependencies (every preset builds the test suite):
```bash
sudo apt install gcc g++ clang clang-tools libclang-rt-dev cmake ninja-build libgtest-dev libgmock-dev
```

> **Compilers:** GCC 11+ or Clang 16+ (C++20). Clang requires `libclang-rt-dev` for coverage instrumentation (`--coverage`).

### Build from Source

```bash
git clone https://github.com/joinframework/dconv.git
cd dconv
cmake --preset gcc-release
cmake --build --preset gcc-release
```

Available presets are `gcc-debug`, `gcc-release`, `gcc-asan`, `clang-debug`, `clang-release` and `clang-asan`.
Debug presets enable coverage, asan presets enable AddressSanitizer and UndefinedBehaviorSanitizer.

dconv is header-only: to install the headers without building the tests, configure without a preset:
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --install build
```

### Build Options

| Option | Default | Description |
| :--- | :---: | :--- |
| `DCONV_ENABLE_TESTS` | `OFF` | Build the test suite. |
| `DCONV_ENABLE_COVERAGE` | `OFF` | Enable code coverage instrumentation (requires Debug build). |
| `DCONV_ENABLE_ASAN` | `OFF` | Enable AddressSanitizer and UndefinedBehaviorSanitizer. |

### Run Tests

```bash
ctest --preset gcc-release
```

## Usage

The printing API can be used this way:

```cpp
#include <dconv/dtoa.hpp>

char value[25];
char* end = dconv::dtoa (value, -2.22507e-308);
```

The parsing API can be used this way:

```cpp
#include <dconv/atod.hpp>

double value;
const char* end = dconv::atod ("-2.22507e-308", value);
```

`dtoa` needs a buffer of at least 25 bytes and returns the end of the written characters.
`atod` returns the end of the parsed characters, or `nullptr` on failure.

## License

[MIT](https://choosealicense.com/licenses/mit/)
