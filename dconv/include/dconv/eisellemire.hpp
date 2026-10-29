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

#ifndef __DCONV_EISELLEMIRE_HPP__
#define __DCONV_EISELLEMIRE_HPP__

// dconv.
#include <dconv/eisellemirepow.hpp>
#include <dconv/probe.hpp>

// C++.
#include <limits>

// C.
#include <cstring>
#include <cstdint>

namespace dconv
{
    namespace eisellemire
    {
        /**
         * @brief multiply two 64 bits unsigned integers.
         * @param a multiplicand.
         * @param b multiplier.
         * @param high high 64 bits of the 128-bit result.
         * @param low low 64 bits of the 128-bit result.
         */
        inline void umul128 (uint64_t a, uint64_t b, uint64_t& high, uint64_t& low) noexcept
        {
#if defined(__SIZEOF_INT128__)
            __uint128_t p = static_cast<__uint128_t> (a) * b;

            high = static_cast<uint64_t> (p >> 64);
            low = static_cast<uint64_t> (p);
#else
            uint64_t b_lo = static_cast<uint32_t> (b);
            uint64_t b_hi = b >> 32;
            uint64_t p0 = (a & 0xFFFFFFFF) * b_lo;
            uint64_t p1 = (a >> 32) * b_lo;
            uint64_t p2 = (a & 0xFFFFFFFF) * b_hi;
            uint64_t p3 = (a >> 32) * b_hi;
            uint64_t carry = (p0 >> 32) + (p1 & 0xFFFFFFFF) + (p2 & 0xFFFFFFFF);

            low = (carry << 32) | (p0 & 0xFFFFFFFF);
            high = (carry >> 32) + (p1 >> 32) + (p2 >> 32) + p3;
#endif
        }

        /**
         * @brief convert double using fast path.
         * @param significand significand digits (at most 19).
         * @param exponent exponent.
         * @return converted value.
         */
        inline double compute (uint64_t significand, int64_t exponent) noexcept
        {
            static constexpr double pow10[] = {1e0,  1e1,  1e2,  1e3,  1e4,  1e5,  1e6,  1e7,  1e8,  1e9,  1e10, 1e11,
                                               1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19, 1e20, 1e21, 1e22};

            double value = static_cast<double> (significand);

            if (DCONV_LIKELY ((exponent >= -22) && (exponent <= 22) && (value <= 9007199254740991.0)))
            {
                return (exponent < 0) ? (value / pow10[-exponent]) : (value * pow10[exponent]);
            }

            if (DCONV_UNLIKELY ((exponent > 22) && (exponent < (22 + 16))))
            {
                const double scaled = value * pow10[exponent - 22];
                if (scaled <= 9007199254740991.0)
                {
                    return scaled * pow10[22];
                }
            }

            if (DCONV_UNLIKELY (significand == 0 || exponent < -342))
            {
                return 0.0;
            }

            if (DCONV_UNLIKELY (exponent > 308))
            {
                return std::numeric_limits<double>::infinity ();
            }

            const int lz = __builtin_clzll (significand);
            significand <<= lz;

            const details::Power& power = details::eisellemirepow[exponent + 342];
            uint64_t high, low;
            umul128 (power.hi, significand, high, low);
            if ((high & 0x1FF) == 0x1FF)
            {
                uint64_t secondHigh, secondLow;
                umul128 (power.lo, significand, secondHigh, secondLow);
                low += secondHigh;
                high += (secondHigh > low);
            }

            const int upperbit = static_cast<int> (high >> 63);
            const int shift = upperbit + 9;
            uint64_t mantissa = high >> shift;
            int64_t power2 = (((152170 + 65536) * exponent) >> 16) + 63 + upperbit - lz + 1023;

            if (DCONV_UNLIKELY (power2 <= 0))
            {
                if (-power2 + 1 >= 64)
                {
                    return 0.0;
                }
                mantissa >>= -power2 + 1;
                mantissa += (mantissa & 1);
                mantissa >>= 1;
                power2 = (mantissa < (uint64_t (1) << 52)) ? 0 : 1;
            }
            else
            {
                if ((low <= 1) && (exponent >= -4) && (exponent <= 23) && ((mantissa & 3) == 1) &&
                    ((mantissa << shift) == high))
                {
                    mantissa &= ~uint64_t (1);
                }

                mantissa += (mantissa & 1);
                mantissa >>= 1;

                if (DCONV_UNLIKELY (mantissa >= (uint64_t (2) << 52)))
                {
                    mantissa = uint64_t (1) << 52;
                    ++power2;
                }

                if (DCONV_UNLIKELY (power2 >= 0x7FF))
                {
                    return std::numeric_limits<double>::infinity ();
                }
            }

            const uint64_t bits = (static_cast<uint64_t> (power2) << 52) | (mantissa & ((uint64_t (1) << 52) - 1));
            memcpy (&value, &bits, sizeof (double));
            return value;
        }
    }
}

#endif
