# grisupowgen

**grisupowgen** is a command-line tool that generates the **lookup
table** of cached powers of 10 used by the Grisu algorithms.

Each power of 10 is stored as a normalized 64-bit mantissa rounded
to nearest and a binary exponent, so that `10^k ≈ mantissa × 2^exponent`.

The tool outputs a fully self-contained C++ header containing:

- `MIN_EXPONENT`, `MAX_EXPONENT`
- a `CachedPower { uint64_t mantissa; int exponent; }` structure
- a `constexpr CachedPower powers[]` lookup table

## Usage

| Option          | Description                           |
|-----------------|---------------------------------------|
| `-h`            | Show help message                     |
| `-l <lower>`    | Set lower exponent (default: `-343`)  |
| `-u <upper>`    | Set upper exponent (default: `343`)   |
| `-v`            | Show program version                  |

## Example

Generate powers for exponents −200 through +200 and write them to
`cached_power.hpp`:

``` bash
grisupowgen -l -200 -u 200 > cached_power.hpp
```

The generated header will contain definitions like:

``` cpp
#ifndef __CACHED_POWER_HPP__
#define __CACHED_POWER_HPP__

#include <cstdint>

#define MIN_EXPONENT -200
#define MAX_EXPONENT 200

struct CachedPower {
    uint64_t mantissa;
    int exponent;
};

constexpr CachedPower powers[] = {
    {0xc3f490aa77bd60fd, -728},
    ...
    ...
    ...
};

#endif
```

## License

[MIT](https://choosealicense.com/licenses/mit/)
