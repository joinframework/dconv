/**
 * MIT License
 *
 * Copyright (c) 2025 Mathieu Rabine
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

#include <unistd.h>
#include <getopt.h>
#include <cstring>
#include <cstdint>
#include <cmath>

// =========================================================================
//   CLASS     :
//   METHOD    : version
// =========================================================================
void version ()
{
    std::cout << "powgen version " << VERSION_MAJOR << "." << VERSION_MINOR << "." << VERSION_PATCH << std::endl;
}

// =========================================================================
//   CLASS     :
//   METHOD    : usage
// =========================================================================
void usage ()
{
    std::cout << "Usage" << std::endl;
    std::cout << "  powgen [options]" << std::endl;
    std::cout << std::endl;
    std::cout << "Options" << std::endl;
    std::cout << "  -h                show available options" << std::endl;
    std::cout << "  -l lower          lower exponent (default: -325)" << std::endl;
    std::cout << "  -u upper          upper exponent (default: 308)" << std::endl;
    std::cout << "  -v                print version" << std::endl;
}

// =========================================================================
//   CLASS     :
//   METHOD    : compute
// =========================================================================
Power compute (int exponent, int base = 5)
{
    if (exponent == 0)
    {
        return { 1ULL << 63, 0 };
    }

    if (exponent > 0)
    {
        BigInt val (1);
        for (int i = 0; i < exponent; ++i)
        {
            val.multiply (base);
        }
        return val.getTop128 ();
    }
    else
    {
        int absExp = -exponent;
        int bitsNeeded = static_cast <int> (absExp * 2.33) + 128 + 64; 
        BigInt val = BigInt::powerOfTwo (bitsNeeded);
        for (int i = 0; i < absExp; ++i)
        {
            val.divide (base);
        }
        return val.getTop128 ();
    }
}

// =========================================================================
//   CLASS     :
//   METHOD    : main
// =========================================================================
int main (int argc, char* argv[])
{
    int minExp = -325, maxExp = 308;

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

    std::vector <Power> powers;
    for (int exp = minExp; exp <= maxExp; ++exp)
    {
        powers.push_back (compute (exp));
    }

    std::cout << "#ifndef __POWER_HPP__" << std::endl;
    std::cout << "#define __POWER_HPP__" << std::endl;
    std::cout << std::endl;

    std::cout << "#include <cstdint>" << std::endl;
    std::cout << std::endl;

    std::cout << "#define MIN_EXPONENT " << minExp << std::endl;
    std::cout << "#define MAX_EXPONENT " << maxExp << std::endl;
    std::cout << std::endl;

    std::cout << "struct Power {" << std::endl;
    std::cout << "    uint64_t hi;" << std::endl;
    std::cout << "    uint64_t lo;" << std::endl;
    std::cout << "};" << std::endl;
    std::cout << std::endl;

    std::cout << "constexpr Power powers[] = {" << std::endl;
    for (size_t i = 0; i < powers.size (); ++i)
    {
        std::cout << std::hex
                  << "    {0x" << std::setw (16) << std::setfill ('0') << powers[i].hi << ", "
                  << "0x" << std::setw (16) << std::setfill ('0') << powers[i].lo << "},"
                  << std::dec << std::endl;
    }
    std::cout << "};" << std::endl;
    std::cout << std::endl;

    std::cout << "#endif" << std::endl;
    std::cout << std::endl;

    _exit (EXIT_SUCCESS);
}
