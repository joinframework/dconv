# dconv

[![Coverage Status](https://github.com/joinframework/dconv/workflows/coverage/badge.svg)](https://github.com/joinframework/dconv/actions?query=workflow%3Acoverage)
[![Security Status](https://github.com/joinframework/dconv/actions/workflows/security.yml/badge.svg)](https://github.com/joinframework/dconv/security/code-scanning)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/44b789b2a04c4f1c9720c6b3020dd769)](https://app.codacy.com/gh/joinframework/dconv/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)
[![Codacy Badge](https://app.codacy.com/project/badge/Coverage/44b789b2a04c4f1c9720c6b3020dd769)](https://app.codacy.com/gh/joinframework/dconv/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_coverage)
[![codecov](https://codecov.io/gh/joinframework/dconv/branch/main/graph/badge.svg)](https://codecov.io/gh/joinframework/dconv)
[![Coverage Status](https://coveralls.io/repos/github/joinframework/dconv/badge.svg?branch=main)](https://coveralls.io/github/joinframework/dconv?branch=main)
[![Doxygen](https://img.shields.io/badge/docs-doxygen-blue.svg)](https://joinframework.github.io/dconv/index.html)
[![GitHub Releases](https://img.shields.io/github/release/joinframework/dconv.svg)](https://github.com/joinframework/dconv/releases/latest)
[![GitHub License](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/joinframework/dconv/blob/main/LICENSE)

**dconv** is a C++14 library for printing and parsing floating point numbers.

Double to string conversion is done using the **Grisu2** algorithm, described by **Florian Loitsch** in its publication [Printing Floating-Point Numbers Quickly and Accurately with Integers](https://florian.loitsch.com/publications).

String to double conversion uses a fast-path implementation based on the **Eisel-Lemire** algorithm, with automatic fallback to strtod when inputs exceed the range or precision safely handled by the fast algorithm.

The code is far from being perfect so any help to improve speed, accuratie, code quality etc... is welcome.

## Dependencies

To install **dconv** dependencies do this:
```bash
sudo apt update && sudo apt install libgtest-dev libgmock-dev
```

## Download

To download the latest source do this:
```bash
git clone https://github.com/mrabine/dconv.git
```

## Configuration

To configure **dconv** do this:
```bash
cmake -B build -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug -DDCONV_ENABLE_TESTS=ON -DDCONV_ENABLE_COVERAGE=ON
```

## Build

To build **dconv** do this:
```bash
cmake --build build --config Debug
```

## Usage

The printing API can be used this way:

```cpp
#include <dconv/dtoa.hpp>

char value [25];
char* end = dconv::dtoa (value, -2.22507e-308);
```

The parsing API can be used this way:

```cpp
#include <dconv/atod.hpp>

double value;
char* end = dconv::atod ("-2.22507e-308", value);
```

## License

[MIT](https://choosealicense.com/licenses/mit/)
