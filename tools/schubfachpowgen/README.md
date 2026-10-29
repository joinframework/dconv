# schubfachpowgen

**schubfachpowgen** is a command-line tool that generates the **lookup
table** of powers of 10 used by the Schubfach algorithm.

Each power of 10 is stored on 126 bits, rounded up, as a high and a
low 64-bit word.

The tool outputs a fully self-contained C++ header containing:

- `MIN_EXPONENT`, `MAX_EXPONENT`
- a `Power { uint64_t hi; uint64_t lo; }` structure
- a `constexpr Power powers[]` lookup table

## Usage

| Option          | Description                           |
|-----------------|---------------------------------------|
| `-h`            | Show help message                     |
| `-l <lower>`    | Set lower exponent (default: `-292`)  |
| `-u <upper>`    | Set upper exponent (default: `324`)   |
| `-v`            | Show program version                  |

## Example

Generate powers for exponents −200 through +200 and write them to
`power.hpp`:

``` bash
schubfachpowgen -l -200 -u 200 > power.hpp
```

The generated header will contain definitions like:

``` cpp
#ifndef __POWER_HPP__
#define __POWER_HPP__

#include <cstdint>

#define MIN_EXPONENT -200
#define MAX_EXPONENT 200

struct Power {
    uint64_t hi;
    uint64_t lo;
};

constexpr Power powers[] = {
    {0x1234567890abcdef, 0xfedcba0987654321},
    ...
    ...
    ...
};

#endif
```

## License

[MIT](https://choosealicense.com/licenses/mit/)
