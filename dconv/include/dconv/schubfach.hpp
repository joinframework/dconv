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

#ifndef __DCONV_SCHUBFACH_HPP__
#define __DCONV_SCHUBFACH_HPP__

// dconv.
#include <dconv/schubfachpow.hpp>
#include <dconv/format.hpp>

// C.
#include <cstring>
#include <cstdint>

namespace dconv
{
    namespace schubfach
    {
        /**
         * @brief compute floor (log10 (2^q)) or floor (log10 (3/4 * 2^q)).
         * @param q binary exponent.
         * @param asymmetric compute floor (log10 (3/4 * 2^q)) if true.
         * @return decimal exponent.
         */
        constexpr int floorLog10Pow2 (int q, bool asymmetric) noexcept
        {
            return (q * 315653 - (asymmetric ? 131008 : 0)) >> 20;
        }

        /**
         * @brief compute floor (log2 (10^k)).
         * @param k decimal exponent.
         * @return binary exponent.
         */
        constexpr int floorLog2Pow10 (int k) noexcept
        {
            return (k * 3483294) >> 20;
        }

        /**
         * @brief multiply by a power of ten and round to odd.
         * @param g power of ten.
         * @param cp value to multiply.
         * @return product rounded to odd.
         */
        inline uint64_t roundToOdd (const details::Power& g, uint64_t cp) noexcept
        {
            uint64_t middle, unused;
            details::umul128 (g.lo, cp, middle, unused);

            uint64_t high, low;
            details::umul128 (g.hi, cp, high, low);

            const uint64_t z = low + middle;
            high += (z < low);
            return high | (z != 0);
        }

        /**
         * @brief compute the shortest decimal significand.
         * @param value positive value to convert.
         * @param k decimal exponent.
         * @return decimal significand, possibly with trailing zeros.
         */
        inline uint64_t shortest (double value, int& k) noexcept
        {
            uint64_t bits;
            memcpy (&bits, &value, sizeof (double));

            const uint64_t fraction = bits & 0x000FFFFFFFFFFFFF;
            const int biased = static_cast<int> (bits >> 52);
            const uint64_t c = (biased != 0) ? (fraction | 0x0010000000000000) : fraction;
            const int q = (biased != 0) ? biased - 1075 : -1074;

            if ((static_cast<unsigned> (-q) <= 52) && ((c & ((static_cast<uint64_t> (1) << -q) - 1)) == 0))
            {
                k = 0;
                return c >> -q;
            }

            const bool asymmetric = (fraction == 0) && (biased > 1);
            const uint64_t out = c & 1;

            const uint64_t cb = c << 2;
            const uint64_t cbl = asymmetric ? cb - 1 : cb - 2;
            const uint64_t cbr = cb + 2;

            k = floorLog10Pow2 (q, asymmetric);
            const int h = q + floorLog2Pow10 (-k) + 3;
            const details::Power& g = details::schubfachpow[292 - k];

            const uint64_t vb = roundToOdd (g, cb << h);
            const uint64_t vbl = roundToOdd (g, cbl << h);
            const uint64_t vbr = roundToOdd (g, cbr << h);

            const uint64_t s = vb >> 2;
            const uint64_t sp10 = (s / 10) * 10;
            const uint64_t tp10 = sp10 + 10;
            const bool upin = vbl + out <= (sp10 << 2);
            const bool wpin = (tp10 << 2) + out <= vbr;
            if (upin != wpin)
            {
                uint64_t digits = upin ? s / 10 : s / 10 + 1;
                ++k;
                if (digits < 1000000000000000)
                {
                    digits *= 10;
                    --k;
                }
                return digits;
            }

            const uint64_t t = s + 1;
            const bool uin = vbl + out <= (s << 2);
            const bool win = (t << 2) + out <= vbr;
            if (uin != win)
            {
                return uin ? s : t;
            }

            const int64_t cmp = static_cast<int64_t> (vb - ((s + t) << 1));
            return ((cmp < 0) || ((cmp == 0) && ((s & 1) == 0))) ? s : t;
        }

        /**
         * @brief write the shortest decimal digits.
         * @param buffer buffer to write the digits to.
         * @param length number of digits written.
         * @param k decimal exponent.
         * @param value positive value to convert.
         */
        inline void generate (char* buffer, int& length, int& k, double value) noexcept
        {
            const uint64_t digits = shortest (value, k);
            length = details::countDigits (digits);
            const int zeros = details::writeDigits (buffer, digits, length);
            length -= zeros;
            k += zeros;
        }

        /**
         * @brief double to string conversion using Schubfach.
         * @param buffer buffer to write the string representation to (must point to at least 25 writable bytes).
         * @param value value to convert.
         * @return end position.
         */
        inline char* dtoa (char* buffer, double value) noexcept
        {
            uint64_t bits;
            memcpy (&bits, &value, sizeof (double));
            bool is_negative = (bits >> 63) != 0;

            *buffer = '-';
            buffer += is_negative;
            value = is_negative ? -value : value;

            if (value == 0.0)
            {
                memcpy (buffer, "0.0", 3);
                return buffer + 3;
            }

            int k = 0;
            const uint64_t digits = shortest (value, k);
            return details::format (buffer, digits, details::countDigits (digits), k);
        }
    }
}

#endif
