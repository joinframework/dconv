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

#ifndef __DCONV_GRISU3_HPP__
#define __DCONV_GRISU3_HPP__

// dconv.
#include <dconv/grisu2.hpp>

// C++.
#include <charconv>

// C.
#include <cstdint>

namespace dconv
{
    namespace grisu3
    {
        /**
         * @brief round the last digit to the closest value and check it is safe.
         * @param buffer generated digits.
         * @param length number of generated digits.
         * @param wpW distance between the liberal upper boundary and the value.
         * @param delta liberal interval size.
         * @param rest distance between the liberal upper boundary and the generated number.
         * @param tenKappa scaled power of ten of the last digit.
         * @param ulp scaled unit of the approximation error.
         * @return true if the generated number is the shortest and closest, false otherwise.
         */
        inline bool roundWeed (char* buffer, int length, uint64_t wpW, uint64_t delta, uint64_t rest, uint64_t tenKappa,
                               uint64_t ulp) noexcept
        {
            const uint64_t wpWup = wpW - ulp;
            const uint64_t wpWdown = wpW + ulp;

            while ((rest < wpWup) && (delta - rest >= tenKappa) &&
                   ((rest + tenKappa < wpWup) || (wpWup - rest >= rest + tenKappa - wpWup)))
            {
                --buffer[length - 1];
                rest += tenKappa;
            }

            if ((rest < wpWdown) && (delta - rest >= tenKappa) &&
                ((rest + tenKappa < wpWdown) || (wpWdown - rest > rest + tenKappa - wpWdown)))
            {
                return false;
            }

            return (2 * ulp <= rest) && (rest <= delta - 4 * ulp);
        }

        /**
         * @brief generate the shortest digits within the liberal boundaries.
         * @param low lower boundary.
         * @param w value.
         * @param high upper boundary.
         * @param buffer buffer to write the digits to.
         * @param length number of digits written.
         * @param k decimal exponent.
         * @return true if the digits are the shortest and closest, false otherwise.
         */
        inline bool digitsGen (const DiyFp& low, const DiyFp& w, const DiyFp& high, char* buffer, int& length,
                               int& k) noexcept
        {
            uint64_t unit = 1;
            const uint64_t tooLow = low._mantissa - unit;
            const uint64_t tooHigh = high._mantissa + unit;
            uint64_t unsafe = tooHigh - tooLow;

            const DiyFp one (static_cast<uint64_t> (1) << -w._exponent, w._exponent);
            uint32_t p1 = static_cast<uint32_t> (tooHigh >> -one._exponent);
            uint64_t p2 = tooHigh & (one._mantissa - 1);
            int kappa = details::countDigits (p1);
            length = 0;

            while (kappa > 0)
            {
                buffer[length++] = static_cast<char> ('0' + p1 / details::pow10[kappa - 1]);
                p1 %= details::pow10[kappa - 1];
                --kappa;

                const uint64_t rest = (static_cast<uint64_t> (p1) << -one._exponent) + p2;
                if (rest < unsafe)
                {
                    k += kappa;
                    return roundWeed (buffer, length, tooHigh - w._mantissa, unsafe, rest,
                                      static_cast<uint64_t> (details::pow10[kappa]) << -one._exponent, unit);
                }
            }

            for (;;)
            {
                p2 *= 10;
                unit *= 10;
                unsafe *= 10;
                buffer[length++] = static_cast<char> ('0' + (p2 >> -one._exponent));
                p2 &= one._mantissa - 1;
                --kappa;

                if (p2 < unsafe)
                {
                    k += kappa;
                    return roundWeed (buffer, length, (tooHigh - w._mantissa) * unit, unsafe, p2, one._mantissa, unit);
                }
            }
        }

        /**
         * @brief write the shortest and closest decimal digits using the standard library.
         * @param buffer buffer to write the digits to.
         * @param length number of digits written.
         * @param k decimal exponent.
         * @param value positive value to convert.
         */
        inline void fallback (char* buffer, int& length, int& k, double value) noexcept
        {
            char text[32];
            const char* end = std::to_chars (text, text + sizeof (text), value, std::chars_format::scientific).ptr;
            const char* cur = text;

            length = 0;
            for (; *cur != 'e'; ++cur)
            {
                if (*cur != '.')
                {
                    buffer[length++] = *cur;
                }
            }

            int exponent = 0;
            std::from_chars (cur + 2, end, exponent);
            k = ((cur[1] == '-') ? -exponent : exponent) - (length - 1);
        }

        /**
         * @brief write the shortest and closest decimal digits.
         * @param buffer buffer to write the digits to.
         * @param length number of digits written.
         * @param k decimal exponent.
         * @param value positive value to convert.
         */
        inline void generate (char* buffer, int& length, int& k, double value) noexcept
        {
            DiyFp w, minus, plus;
            grisu2::scaledBoundaries (value, w, minus, plus, k);

            if (!digitsGen (minus, w, plus, buffer, length, k))
            {
                fallback (buffer, length, k, value);
            }
        }

        /**
         * @brief double to string conversion using Grisu3.
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
