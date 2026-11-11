---
paths:
  - "dconv/**/*.hpp"
  - "dconv/**/*.cpp"
---

# C++ style

`.clang-format` is enforced (hook, or `clang-format` 19); never hand-format.

## Layout
- C++20, header-only, no language extensions; GNU builtins and attributes already in use (`__builtin_expect`, `__attribute__ ((cold, noinline))`) are accepted. Space before every `(` (`foo (a)`, `TEST (s, n)`); Allman braces on every control statement.
- Header: MIT license, `#ifndef __DCONV_<NAME>_HPP__` (no `#pragma once`), content in indented `namespace dconv`; internals in `details` or the algorithm namespace (`schubfach`, `grisu2`...). New public headers go in `PUBLIC_HEADERS` (`dconv/CMakeLists.txt`).
- Include groups: `// dconv.`, `// libraries.` (tests), `// C++.`, `// C.`; `<dconv/...>` only.

## Naming
- `PascalCase` types, enum values and template parameters (`Probe`, `Rewind`); `camelCase` functions; class members `_camelCase` (plain aggregates such as `details::Power` keep `hi`/`lo`); macros `DCONV_UPPER`; fixed-width types unprefixed (`uint64_t`). Std-mandated names keep their spelling (`make_error_code`).

## Classes
- Model: `StringProbe`/`StreamProbe`. Order: `public` then `private`; constructors, special members, destructor, API, then `static constexpr` members, then default-initialised data members. `DiyFp` (public data read by Grisu) and `ErrorCategory` are older exceptions: do not imitate them, do not change them unasked.
- New classes declare every special member (`= default` or `= delete`, e.g. `StreamProbe` non-copyable); `explicit` single-argument constructors; `override` on virtuals; constructor initializers one per line, leading comma.
- Overloads that would return a pointer into a temporary are `= delete` (`atod (std::string&&, ...)`). Compound operators are members, binary operators free functions after the class.

## Code
- Free and out-of-class header functions `inline` (`constexpr`, in-class and template definitions may omit it); `noexcept` when it cannot throw; `constexpr` where possible. Named casts only; `memcpy` for type punning.
- Errors: return `nullptr` / `false` / `-1` and `Errc` (`error.hpp`); no exceptions. Never change `errno` (save and restore around `strtod`).
- Performance first: no allocation, no locale, bounded loops, tables computed at build time; `DCONV_LIKELY`/`DCONV_UNLIKELY` on branches. Works on aarch64; `__uint128_t` only behind `#if defined(__SIZEOF_INT128__)` with a portable `#else` (never compiled by CI: review it by reading).

## Doxygen
- New header declarations (tests follow `tests.md`): `/**`, ` * @brief ...` (lowercase), then `@param`/`@return` lines, ` */`; never `///` for functions or types; state buffer size requirements.
- Wording: constructors `default constructor.` (the probes, also with arguments) or `create <what> using <args>.` (`DiyFp`); `copy constructor.`, `copy assignment.`, `move constructor.`, `move assignment.`, `destroy instance.`; `@param other object to copy.` / `object to move.`; `@return a reference of the current object.`. Members and constants: `/// one line.`; enum values: trailing `/**< ... */`.
- No comments inside function bodies except `LCOV_EXCL_*` markers.
