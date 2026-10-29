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

#ifndef __DCONV_GRISU_HPP__
#define __DCONV_GRISU_HPP__

// dconv.
#include <dconv/grisupow.hpp>
#include <dconv/format.hpp>

// C.
#include <cassert>
#include <cstring>
#include <cstdint>

namespace dconv
{
    namespace grisu
    {
        namespace details
        {
            /**
             * @brief double to string conversion with the given digits generator.
             * @param buffer buffer to write the string representation to.
             * @param value value to convert.
             * @param generate digits generator.
             * @return end position.
             */
            inline char* dtoa (char* buffer, double value,
                               void (*generate) (char*, int&, int&, double) noexcept) noexcept
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

                int length = 0, k = 0;
                generate (buffer, length, k, value);
                return dconv::details::prettify (buffer, length, k);
            }
        }

        /**
         * @brief get the cached power of ten that scales a binary exponent into the digit generation range.
         * @param exponent binary exponent of the normalized value.
         * @param k decimal exponent of the scaled value.
         * @return cached power of ten.
         */
        inline const DiyFp& cachedPower (int exponent, int& k) noexcept
        {
            const int expr = -59 - (exponent + 64) + 63;
            const int mk = (expr * 30103 + 99999) / 100000;

            assert (mk + 343 >= 0 && mk + 343 < 687);
            k = -mk;
            return dconv::details::grisupow[mk + 343];
        }

        /**
         * @brief write the decimal digits of the scaled value.
         * @param buffer buffer to write the digits to.
         * @param length number of digits written.
         * @param k decimal exponent.
         * @param value positive value to convert.
         */
        inline void generate (char* buffer, int& length, int& k, double value) noexcept
        {
            DiyFp w (value);
            w.normalize ();
            w *= cachedPower (w._exponent, k);

            const DiyFp one (static_cast<uint64_t> (1) << -w._exponent, w._exponent);
            uint32_t part1 = static_cast<uint32_t> (w._mantissa >> -one._exponent);
            uint64_t part2 = w._mantissa & (one._mantissa - 1);

            length = 0;
            for (int kappa = dconv::details::countDigits (part1); kappa > 0; --kappa)
            {
                buffer[length++] = static_cast<char> ('0' + part1 / dconv::details::pow10[kappa - 1]);
                part1 %= dconv::details::pow10[kappa - 1];
            }

            while ((length < 18) && (part2 != 0))
            {
                part2 *= 10;
                buffer[length++] = static_cast<char> ('0' + (part2 >> -one._exponent));
                part2 &= one._mantissa - 1;
                --k;
            }
        }

        /**
         * @brief double to string conversion using Grisu.
         * @param buffer buffer to write the string representation to (must point to at least 26 writable bytes).
         * @param value value to convert.
         * @return end position.
         */
        inline char* dtoa (char* buffer, double value) noexcept
        {
            return details::dtoa (buffer, value, generate);
        }
    }
}

#endif
