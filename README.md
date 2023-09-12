# dconv

[![Coverage Status](https://github.com/joinframework/dconv/workflows/coverage/badge.svg)](https://github.com/joinframework/dconv/actions?query=workflow%3Acoverage)
[![Security Status](https://github.com/joinframework/dconv/workflows/security/badge.svg)](https://github.com/joinframework/dconv/security/code-scanning)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/44b789b2a04c4f1c9720c6b3020dd769)](https://app.codacy.com/gh/joinframework/dconv/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)
[![Codacy Badge](https://app.codacy.com/project/badge/Coverage/44b789b2a04c4f1c9720c6b3020dd769)](https://app.codacy.com/gh/joinframework/dconv/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_coverage)
[![codecov](https://codecov.io/gh/joinframework/dconv/branch/main/graph/badge.svg)](https://codecov.io/gh/joinframework/dconv)
[![Coverage Status](https://coveralls.io/repos/github/joinframework/dconv/badge.svg?branch=main)](https://coveralls.io/github/joinframework/dconv?branch=main)
[![Doxygen](https://img.shields.io/badge/docs-doxygen-blue.svg)](https://joinframework.github.io/dconv/index.html)
[![GitHub Releases](https://img.shields.io/github/release/joinframework/dconv.svg)](https://github.com/joinframework/dconv/releases/latest)
[![GitHub License](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/joinframework/dconv/blob/main/LICENSE)

**dconv** is a C++14 library for printing and parsing floating point numbers.

Double to string conversion is done using the **Grisu2** algorithm, described by **Florian Loitsch** in its publication [Printing Floating-Point Numbers Quickly and Accurately with Integers](https://florian.loitsch.com/publications).

String to double conversion is done using a fast and simple (but not accurate!) approach and fallback to **strtod** if conversion can't be done the simplified way.

The code is far from being perfect so any help to improve speed, accuratie, code quality etc... is welcome.

## Download

To download the latest source do this:
```bash
git clone https://github.com/mrabine/dconv.git
```

## Configuration

To configure dconv do this:
```bash
cd dconv
mkdir build && cd build
cmake ..
```

## Installation

To install dconv do this:
```bash
make && sudo make install
```

Note that building and installing the library is not required as it is a header only library.

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
