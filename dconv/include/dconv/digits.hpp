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

    }
}

#endif
