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

#ifndef __DCONV_DIGITS_HPP__
#define __DCONV_DIGITS_HPP__

// C.
#include <cstdint>

namespace dconv
{
    namespace details
    {
        /// powers of ten that fit in 32 bits.
        constexpr uint32_t pow10[] = {1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000};

        /// two digits decimal representations of 0 to 99.
        alignas (64) constexpr char digitPairs[] =
            "0001020304050607080910111213141516171819202122232425262728293031323334353637383940414243444546474849505152"
            "5354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979899";

        /// trailing zeros of the two digits decimal representations of 0 to 99.
        alignas (64) constexpr uint8_t pairZeros[] = {
            2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0};

        /**
         * @brief count the trailing zeros of a four digits group.
         * @param group four digits group.
         * @return number of trailing zeros.
         */
        inline int countTrailingZeros (uint32_t group) noexcept
        {
            return pairZeros[group % 100] + ((group % 100 == 0) ? pairZeros[group / 100] : 0);
        }

        /**
         * @brief count the digits of a decimal significand.
         * @param digits decimal significand.
         * @return number of digits.
         */
        inline int countDigits (uint64_t digits) noexcept
        {
            if (digits < 100000000)
            {
                if (digits < 10000)
                {
                    if (digits < 100)
                    {
                        return (digits < 10) ? 1 : 2;
                    }
                    return (digits < 1000) ? 3 : 4;
                }
                if (digits < 1000000)
                {
                    return (digits < 100000) ? 5 : 6;
                }
                return (digits < 10000000) ? 7 : 8;
            }
            if (digits >= 10000000000000000)
            {
                return 17;
            }
            if (digits >= 1000000000000000)
            {
                return 16;
            }
            if (digits >= 1000000000000)
            {
                return (digits >= 100000000000000) ? 15 : ((digits >= 10000000000000) ? 14 : 13);
            }
            if (digits >= 10000000000)
            {
                return (digits >= 100000000000) ? 12 : 11;
            }
            return (digits >= 1000000000) ? 10 : 9;
        }

        /**
         * @brief write the digits of a decimal significand.
         * @param buffer buffer to write the digits to.
         * @param digits decimal significand.
         * @param length number of digits.
         * @return number of trailing zeros written.
         */
        inline int writeDigits (char* buffer, uint64_t digits, int length) noexcept
        {
            uint32_t high = static_cast<uint32_t> (digits);
            uint32_t low = 0;
            if (digits >= 100000000)
            {
                high = static_cast<uint32_t> (digits / 100000000);
                low = static_cast<uint32_t> (digits % 100000000);
            }

            char* const end = buffer + length;
            char* cur = end;
            int zeros = 0;
            if (digits >= 100000000)
            {
                cur -= 8;
                if (low == 0)
                {
                    __builtin_memcpy (cur, "00000000", 8);
                    zeros = 8;
                }
                else
                {
                    const uint32_t upper = low / 10000;
                    const uint32_t lower = low % 10000;
                    __builtin_memcpy (cur, &digitPairs[(upper / 100) * 2], 2);
                    __builtin_memcpy (cur + 2, &digitPairs[(upper % 100) * 2], 2);
                    __builtin_memcpy (cur + 4, &digitPairs[(lower / 100) * 2], 2);
                    __builtin_memcpy (cur + 6, &digitPairs[(lower % 100) * 2], 2);
                    zeros = (lower != 0) ? countTrailingZeros (lower) : 4 + countTrailingZeros (upper);
                }
            }

            if (high >= 10000)
            {
                const uint32_t lower = high % 10000;
                high /= 10000;
                cur -= 4;
                __builtin_memcpy (cur, &digitPairs[(lower / 100) * 2], 2);
                __builtin_memcpy (cur + 2, &digitPairs[(lower % 100) * 2], 2);
                if (zeros == end - cur - 4)
                {
                    zeros += countTrailingZeros (lower);
                }
            }

            while (high >= 100)
            {
                cur -= 2;
                __builtin_memcpy (cur, &digitPairs[(high % 100) * 2], 2);
                if (zeros == end - cur - 2)
                {
                    zeros += pairZeros[high % 100];
                }
                high /= 100;
            }

            if (high >= 10)
            {
                cur -= 2;
                __builtin_memcpy (cur, &digitPairs[high * 2], 2);
                if (zeros == end - cur - 2)
                {
                    zeros += pairZeros[high];
                }
            }
            else
            {
                *--cur = static_cast<char> ('0' + high);
            }

            return zeros;
        }
    }
}

#endif
