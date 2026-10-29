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

#ifndef __DCONV_FORMAT_HPP__
#define __DCONV_FORMAT_HPP__

// dconv.
#include <dconv/digits.hpp>

// C.
#include <cstring>
#include <cassert>
#include <cstdint>

namespace dconv
{
    namespace details
    {
        /**
         * @brief write a decimal exponent.
         * @param buffer buffer to write the exponent to.
         * @param k exponent.
         * @return end position.
         */
        inline char* writeExponent (char* buffer, int k) noexcept
        {
            *buffer = '-';
            buffer += (k < 0);
            k = (k < 0) ? -k : k;
            if (k >= 100)
            {
                *buffer++ = static_cast<char> ('0' + k / 100);
                __builtin_memcpy (buffer, &digitPairs[(k % 100) * 2], 2);
                buffer += 2;
            }
            else if (k >= 10)
            {
                __builtin_memcpy (buffer, &digitPairs[k * 2], 2);
                buffer += 2;
            }
            else
            {
                *buffer++ = static_cast<char> ('0' + k);
            }
            return buffer;
        }

        /**
         * @brief lay out decimal digits as a fixed or scientific number.
         * @param buffer buffer holding the digits.
         * @param length number of digits.
         * @param k decimal exponent.
         * @return end position.
         */
        inline char* prettify (char* buffer, int length, int k) noexcept
        {
            assert (length > 0);
            const int kk = length + k;

            if ((length <= kk) && (kk <= 21))
            {
                memset (buffer + length, '0', kk - length);
                buffer[kk] = '.';
                buffer[kk + 1] = '0';
                return &buffer[kk + 2];
            }
            else if ((0 < kk) && (kk <= 21))
            {
                if ((length - kk <= 16) && (kk <= 7))
                {
                    char tmp[16];
                    memcpy (tmp, &buffer[kk], 16);
                    memcpy (&buffer[kk + 1], tmp, 16);
                }
                else
                {
                    memmove (&buffer[kk + 1], &buffer[kk], length - kk);
                }
                buffer[kk] = '.';
                return &buffer[length + 1];
            }
            else if ((-6 < kk) && (kk <= 0))
            {
                int offset = 2 - kk;
                memmove (&buffer[offset], &buffer[0], length);
                buffer[0] = '0';
                buffer[1] = '.';
                memset (&buffer[2], '0', offset - 2);
                return &buffer[length + offset];
            }
            else if (length == 1)
            {
                buffer[1] = 'e';
                return writeExponent (&buffer[2], kk - 1);
            }
            else
            {
                if (length - 1 <= 16)
                {
                    char tmp[16];
                    memcpy (tmp, &buffer[1], 16);
                    memcpy (&buffer[2], tmp, 16);
                }
                else
                {
                    memmove (&buffer[2], &buffer[1], length - 1);
                }
                buffer[1] = '.';
                buffer[length + 1] = 'e';
                return writeExponent (&buffer[length + 2], kk - 1);
            }
        }
    }
}

#endif
