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
 * LIABILITY, WHETHER IN AN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __DCONV_POWER_HPP__
#define __DCONV_POWER_HPP__

// C.
#include <cstdint>

namespace dconv
{
    namespace details
    {
        /// 128 bits power.
        struct Power
        {
            /// high 64 bits.
            uint64_t hi;

            /// low 64 bits.
            uint64_t lo;
        };

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
    }
}

#endif
