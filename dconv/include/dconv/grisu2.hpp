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

#ifndef __DCONV_GRISU2_HPP__
#define __DCONV_GRISU2_HPP__

// dconv.
#include <dconv/grisu.hpp>

// C.
#include <cassert>
#include <cstdint>

namespace dconv
{
    namespace grisu2
    {
        /**
         * @brief compute the scaled value and its scaled boundaries.
         * @param value positive value to convert.
         * @param w scaled value.
         * @param minus scaled lower boundary.
         * @param plus scaled upper boundary.
         * @param k decimal exponent of the scaled values.
         */
        inline void scaledBoundaries (double value, DiyFp& w, DiyFp& minus, DiyFp& plus, int& k) noexcept
        {
            w = DiyFp (value);
            w.normalizedBoundaries (minus, plus);

            const DiyFp& power = grisu::cachedPower (plus._exponent, k);
            minus *= power;
            plus *= power;
            w.normalize ();
            w *= power;
        }

        /**
         * @brief move the last digit closer to the value while it stays within the boundaries.
         * @param buffer generated digits.
         * @param length number of generated digits.
         * @param delta distance between the boundaries.
         * @param rest distance between the upper boundary and the generated number.
         * @param tenKappa scaled power of ten of the last digit.
         * @param wpW distance between the upper boundary and the value.
         */
        inline void round (char* buffer, int length, uint64_t delta, uint64_t rest, uint64_t tenKappa,
                           uint64_t wpW) noexcept
        {
            while ((rest < wpW && delta - rest >= tenKappa) &&
                   (rest + tenKappa < wpW || wpW - rest > rest + tenKappa - wpW))
            {
                --buffer[length - 1];
                rest += tenKappa;
            }
        }

        /**
         * @brief generate the shortest digits within the conservative boundaries.
         * @param w scaled value.
         * @param high conservative upper boundary.
         * @param delta distance between the conservative boundaries.
         * @param buffer buffer to write the digits to.
         * @param length number of digits written.
         * @param k decimal exponent.
         */
        inline void digitsGen (const DiyFp& w, const DiyFp& high, uint64_t delta, char* buffer, int& length,
                               int& k) noexcept
        {
            const DiyFp one (static_cast<uint64_t> (1) << -high._exponent, high._exponent);
            const DiyFp wpW = high - w;
            uint32_t p1 = static_cast<uint32_t> (high._mantissa >> -one._exponent);
            uint64_t p2 = high._mantissa & (one._mantissa - 1);
            int kappa = details::countDigits (p1);
            length = 0;

            while (kappa > 0)
            {
                assert (kappa >= 1 && kappa <= 10);
                uint32_t d = p1 / details::pow10[kappa - 1];
                p1 %= details::pow10[kappa - 1];
                if (d || length)
                {
                    buffer[length++] = '0' + d;
                }
                --kappa;
                uint64_t rest = (static_cast<uint64_t> (p1) << -one._exponent) + p2;
                if (rest <= delta)
                {
                    k += kappa;
                    round (buffer, length, delta, rest, static_cast<uint64_t> (details::pow10[kappa]) << -one._exponent,
                           wpW._mantissa);
                    return;
                }
            }

            uint64_t unit = 1;
            for (;;)
            {
                assert (unit <= UINT64_MAX / 10);
                p2 *= 10;
                delta *= 10;
                unit *= 10;
                char d = static_cast<char> (p2 >> -one._exponent);
                buffer[length] = '0' + d;
                length += (d || length);
                p2 &= one._mantissa - 1;
                --kappa;
                if (p2 < delta)
                {
                    k += kappa;
                    round (buffer, length, delta, p2, one._mantissa, wpW._mantissa * unit);
                    return;
                }
            }
        }

        /**
         * @brief write the shortest decimal digits within the conservative boundaries.
         * @param buffer buffer to write the digits to.
         * @param length number of digits written.
         * @param k decimal exponent.
         * @param value positive value to convert.
         */
        inline void generate (char* buffer, int& length, int& k, double value) noexcept
        {
            DiyFp w, minus, plus;
            scaledBoundaries (value, w, minus, plus, k);

            ++minus._mantissa;
            --plus._mantissa;

            digitsGen (w, plus, plus._mantissa - minus._mantissa, buffer, length, k);
        }

        /**
         * @brief double to string conversion using Grisu2.
         * @param buffer buffer to write the string representation to (must point to at least 25 writable bytes).
         * @param value value to convert.
         * @return end position.
         */
        inline char* dtoa (char* buffer, double value) noexcept
        {
            return grisu::details::dtoa (buffer, value, generate);
        }
    }
}

#endif
