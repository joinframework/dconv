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

Double to string conversion uses the **Grisu2** algorithm by default, described by **Florian Loitsch** in its publication [Printing Floating-Point Numbers Quickly and Accurately with Integers](https://florian.loitsch.com/publications).

The **Grisu** and **Grisu3** algorithms from the same publication are also available. Grisu prints up to 18 digits without looking for the shortest output, Grisu2 prints the shortest output for about 99.9% of the doubles, and Grisu3 detects the remaining cases and falls back to `std::to_chars`.

String to double conversion uses the **Eisel-Lemire** algorithm on the first 19 significant digits. Longer mantissas are bracketed between two Eisel-Lemire results; only when they disagree is the number rebuilt from the digits already read and passed to strtod. A stream is read once and never rewound; a buffer is only read again for mantissas longer than 19 digits.

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

Each algorithm has its own header and namespace:

```cpp
#include <dconv/grisu3.hpp>

char value[25];
char* end = dconv::grisu3::dtoa (value, -2.22507e-308);
```

| Header | Function |
| :--- | :--- |
| `dconv/grisu3.hpp` | `dconv::grisu3::dtoa` |
| `dconv/grisu2.hpp` | `dconv::grisu2::dtoa` |
| `dconv/grisu.hpp` | `dconv::grisu::dtoa` |

The parsing API can be used this way:

```cpp
#include <dconv/atod.hpp>

double value;
const char* end = dconv::atod ("-2.22507e-308", value);
```

`atod` also accepts a `std::string` or a `std::string_view`, and a `(first, length)` or `(first, last)` range.
A temporary `std::string` is rejected, since the returned position would point into it.

It can also read from a stream, without buffering it:

```cpp
#include <dconv/atod.hpp>
#include <sstream>

double value;
std::istringstream in ("-2.22507e-308");
bool ok = dconv::atod (in, value);
```

A variant reads from a probe and stops on integers, so that integers can be kept as integers:

```cpp
#include <dconv/atod.hpp>

double value;
uint64_t significand;
int64_t exponent;
bool negative;
std::error_code ec;

dconv::StringProbe probe ("18446744073709551615");
if (dconv::atod (probe, value, significand, exponent, negative, ec) == -1 && ec == dconv::Errc::NotDouble)
{
    // significand holds the integer, negative its sign.
}
```

It returns `0` and the converted value for a real number, or `-1` with `Errc::NotDouble` for an integer without fraction nor exponent whose magnitude fits in 64 bits, or `-1` with `Errc::InvalidNumber` when parsing fails.
`significand`, `exponent` and `negative` hold what was read until the end or the error.

`dtoa` needs a buffer of at least 25 bytes (26 bytes for `grisu::dtoa`) and returns the end of the written characters.
`atod` returns the end of the parsed characters, or `nullptr` on failure.
The probe and stream overloads return `false` on failure and stop right after the number on success.
They read the stream buffer directly: whitespaces are not skipped, the stream state is left unchanged and the characters read are consumed even on failure.
`atod` never changes `errno`.

## License

[MIT](https://choosealicense.com/licenses/mit/)
