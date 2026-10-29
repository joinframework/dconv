/**
 * MIT License
 *
 * Copyright (c) 2026 Mathieu Rabine
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "version.hpp"
#include "bigint.hpp"

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

#include <unistd.h>
#include <getopt.h>
#include <cstdint>
#include <cmath>

struct CachedPower
{
    uint64_t mantissa;
    int exponent;
};

// =========================================================================
//   CLASS     :
//   METHOD    : version
// =========================================================================
void version ()
{
    std::cout << "grisupowgen version " << VERSION_MAJOR << "." << VERSION_MINOR << "." << VERSION_PATCH << std::endl;
}

// =========================================================================
//   CLASS     :
//   METHOD    : usage
// =========================================================================
void usage ()
{
    std::cout << "Usage" << std::endl;
    std::cout << "  grisupowgen [options]" << std::endl;
    std::cout << std::endl;
    std::cout << "Options" << std::endl;
    std::cout << "  -h                show available options" << std::endl;
    std::cout << "  -l lower          lower exponent (default: -343)" << std::endl;
    std::cout << "  -u upper          upper exponent (default: 343)" << std::endl;
    std::cout << "  -v                print version" << std::endl;
}

// =========================================================================
//   CLASS     :
//   METHOD    : compute
// =========================================================================
CachedPower compute (int exponent)
{
    BigInt val (1);
    int shift = 0;

    if (exponent > 0)
    {
        for (int i = 0; i < exponent; ++i)
        {
            val.multiply (10);
        }
    }
    else if (exponent < 0)
    {
        shift = static_cast<int> (-exponent * std::log2 (10.0)) + 128;
        val = BigInt::powerOfTwo (shift);
        for (int i = 0; i < -exponent; ++i)
        {
            val.divide (10);
        }
    }

    Power top = val.getTop128 ();
    CachedPower power = {top.hi + (top.lo >> 63), val.bitLength () - 64 - shift};
    if (power.mantissa == 0)
    {
        power.mantissa = 1ULL << 63;
        ++power.exponent;
    }
    return power;
}

// =========================================================================
//   CLASS     :
//   METHOD    : main
// =========================================================================
int main (int argc, char* argv[])
{
    int minExp = -343, maxExp = 343;

    int opt;
    while ((opt = getopt (argc, argv, "hl:u:v")) != -1)
    {
        switch (opt)
        {
            case 'h':
                usage ();
                _exit (EXIT_SUCCESS);
            case 'l':
                minExp = std::stoi (optarg);
                break;
            case 'u':
                maxExp = std::stoi (optarg);
                break;
            case 'v':
                version ();
                _exit (EXIT_SUCCESS);
            default:
                usage ();
                _exit (EXIT_FAILURE);
        }
    }

    if (minExp > maxExp)
    {
        std::cerr << "min exponent must be less than max exponent" << std::endl;
        _exit (EXIT_FAILURE);
    }

    std::vector<CachedPower> powers;
    for (int exp = minExp; exp <= maxExp; ++exp)
    {
        powers.push_back (compute (exp));
    }

    std::cout << "#ifndef __CACHED_POWER_HPP__" << std::endl;
    std::cout << "#define __CACHED_POWER_HPP__" << std::endl;
    std::cout << std::endl;

    std::cout << "#include <cstdint>" << std::endl;
    std::cout << std::endl;

    std::cout << "#define MIN_EXPONENT " << minExp << std::endl;
    std::cout << "#define MAX_EXPONENT " << maxExp << std::endl;
    std::cout << std::endl;

    std::cout << "struct CachedPower {" << std::endl;
    std::cout << "    uint64_t mantissa;" << std::endl;
    std::cout << "    int exponent;" << std::endl;
    std::cout << "};" << std::endl;
    std::cout << std::endl;

    std::cout << "constexpr CachedPower powers[] = {" << std::endl;
    for (size_t i = 0; i < powers.size (); ++i)
    {
        std::cout << std::hex << "    {0x" << std::setw (16) << std::setfill ('0') << powers[i].mantissa << ", "
                  << std::dec << powers[i].exponent << "}," << std::endl;
    }
    std::cout << "};" << std::endl;
    std::cout << std::endl;

    std::cout << "#endif" << std::endl;
    std::cout << std::endl;

    _exit (EXIT_SUCCESS);
}
