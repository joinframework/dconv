/**
 * MIT License
 *
 * Copyright (c) 2021 Mathieu Rabine
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

#ifndef __DCONV_ATOD_HPP__
#define __DCONV_ATOD_HPP__

// dconv.
#include <dconv/atodpow.hpp>
#include <dconv/view.hpp>

// C++.
#include <limits>
#include <memory>

// C.
#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <cmath>

namespace dconv
{
    namespace details
    {
        struct LocaleDelete
        {
            constexpr LocaleDelete () noexcept = default;

            void operator () (locale_t loc) noexcept
            {
                freelocale (loc);
            }
        };

        using LocalePtr = std::unique_ptr <std::remove_pointer_t <locale_t>, LocaleDelete>;

        inline const char * strtodSlow (const char * beg, double& value)
        {
            static LocalePtr locale (newlocale (LC_ALL_MASK, "C", nullptr));
            char* end = nullptr;
            value = strtod_l (beg, &end, locale.get ());
            return end;
        }

        inline void umul192 (uint64_t hi, uint64_t lo, uint64_t mantissa, uint64_t& high, uint64_t& middle, uint64_t& low) noexcept
        {
        #if defined(__SIZEOF_INT128__)
            __uint128_t h = static_cast <__uint128_t> (hi) * mantissa;
            __uint128_t l = static_cast <__uint128_t> (lo) * mantissa;
            __uint128_t s = h + (l >> 64);

            high = static_cast <uint64_t> (s >> 64);
            middle = static_cast <uint64_t> (s);
            low = static_cast <uint64_t> (l);
        #else
            uint64_t hi_hi, hi_lo, lo_hi, lo_lo;

            uint64_t m_lo = static_cast <uint32_t> (mantissa);
            uint64_t m_hi = mantissa >> 32;
            uint64_t p0 = (hi & 0xFFFFFFFF) * m_lo;
            uint64_t p1 = (hi >> 32) * m_lo;
            uint64_t p2 = (hi & 0xFFFFFFFF) * m_hi;
            uint64_t p3 = (hi >> 32) * m_hi;
            uint64_t carry = (p0 >> 32) + (p1 & 0xFFFFFFFF) + (p2 & 0xFFFFFFFF);
            hi_lo = (carry << 32) | (p0 & 0xFFFFFFFF);
            hi_hi = (carry >> 32) + (p1 >> 32) + (p2 >> 32) + p3;

            p0 = (lo & 0xFFFFFFFF) * m_lo;
            p1 = (lo >> 32) * m_lo;
            p2 = (lo & 0xFFFFFFFF) * m_hi;
            p3 = (lo >> 32) * m_hi;
            carry = (p0 >> 32) + (p1 & 0xFFFFFFFF) + (p2 & 0xFFFFFFFF);
            lo_lo = (carry << 32) | (p0 & 0xFFFFFFFF);
            lo_hi = (carry >> 32) + (p1 >> 32) + (p2 >> 32) + p3;

            low = lo_lo;
            middle = hi_lo + lo_hi;
            high = hi_hi + (middle < hi_lo ? 1 : 0);
        #endif
        }

        inline bool strtodFast (bool negative, uint64_t mantissa, int64_t exponent, double& value) noexcept
        {
            static constexpr double pow10[] = {
                1e0,  1e1,  1e2,  1e3,  1e4,  1e5,  1e6,  1e7,  1e8,  1e9,  1e10,
                1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19, 1e20, 1e21,
                1e22
            };

            if (unlikely (mantissa == 0))
            {
                value = negative ? -0.0 : 0.0;
                return true;
            }

            if (likely ((exponent >= -22) && (exponent <= 22) && (mantissa <= 9007199254740991)))
            {
                value = static_cast <double> (mantissa);
                value = (exponent < 0) ? (value / pow10[-exponent]) : (value * pow10[exponent]);
                value = negative ? -value : value;
                return true;
            }

            if (unlikely (exponent < -325 || exponent > 308))
            {
                return false;
            }

            uint64_t high, middle, low;
            const Power& power = atodpow[exponent + 325];
            umul192 (power.hi, power.lo, mantissa, high, middle, low);
            int64_t exp = ((exponent * 217706) >> 16) + 1087;

            int lz;
            if (high != 0)
            {
                lz = __builtin_clzll (high);
                exp -= lz;
            }
            else if (middle != 0)
            {
                lz = __builtin_clzll (middle);
                exp -= lz + 64;
            }
            else
            {
                return false;
            }

            if (unlikely (exp <= 0 || exp >= 2047))
            {
                return false;
            }

            if (high == 0)
            {
                high = middle << lz;
                middle = 0;
            }
            else if (lz != 0)
            {
                high = (high << lz) | (middle >> (64 - lz));
                middle <<= lz;
            }

            middle |= (low != 0);

            uint64_t mant = (high >> 11) & 0xFFFFFFFFFFFFF;
            uint64_t bits = (static_cast <uint64_t> (exp) << 52) | mant;
            uint64_t frac = high & 0x7FF;

            bool roundUp = ((frac >  0x400) |
                           ((frac == 0x400) && ((middle != 0) || (mant & 1))) |
                           ((frac == 0x3FF) && ((middle != 0))));

            bits += roundUp;
            bits |= (static_cast <uint64_t> (negative) << 63);
            std::memcpy (&value, &bits, sizeof (double));

            return true;
        }

        constexpr inline bool isDigit (char c) noexcept
        {
            return static_cast <unsigned char> (c - '0') <= 9u;
        }

        constexpr inline bool isSign (char c) noexcept
        {
            return (c == '+') || (c == '-');
        }

        inline const char * atod (View& view, double& value)
        {
            auto beg = view.data ();

            uint64_t mantissa = 0;
            uint64_t digits = 0;
            bool neg = view.getIf ('-');

            if (view.getIf ('0'))
            {
                if (unlikely (isDigit (view.peek ())))
                {
                    return nullptr;
                }
            }
            else if (likely (isDigit (view.peek ())))
            {
                mantissa = view.get () - '0';
                ++digits;

                while (isDigit (view.peek ()))
                {
                    uint64_t next = (10 * mantissa) + (view.get () - '0');
                    if (unlikely (next < mantissa)) // overflow
                    {
                        return strtodSlow (beg, value);
                    }
                    mantissa = next;
                    ++digits;
                }
            }
            else if (view.getIfNoCase ('i') && view.getIfNoCase ('n') && view.getIfNoCase ('f'))
            {
                if (view.getIfNoCase ('i'))
                {
                    if (!(view.getIfNoCase ('n') && view.getIfNoCase ('i') && 
                          view.getIfNoCase ('t') && view.getIfNoCase ('y')))
                    {
                        return nullptr;
                    }
                }
                value = neg ? -std::numeric_limits <double>::infinity () : std::numeric_limits <double>::infinity ();
                return view.data ();
            }
            else if (view.getIfNoCase ('n') && view.getIfNoCase ('a') && view.getIfNoCase ('n'))
            {
                value = neg ? -std::numeric_limits <double>::quiet_NaN () : std::numeric_limits <double>::quiet_NaN ();
                return view.data ();
            }
            else
            {
                return nullptr;
            }

            int64_t exponent = 0;

            if (view.getIf ('.'))
            {
                if (unlikely (!isDigit (view.peek ())))
                {
                    return nullptr;
                }

                mantissa = (10 * mantissa) + (view.get () - '0');
                if (mantissa || digits) ++digits;
                --exponent;

                while (isDigit (view.peek ()))
                {
                    uint64_t next = (10 * mantissa) + (view.get () - '0');
                    if (unlikely (next < mantissa)) // overflow
                    {
                        return strtodSlow (beg, value);
                    }
                    mantissa = next;
                    if (mantissa || digits) ++digits;
                    --exponent;
                }
            }

            if (view.getIf ('e') || view.getIf ('E'))
            {
                bool negExp = false;

                if (isSign (view.peek ()))
                {
                    negExp = (view.get () == '-');
                }

                if (unlikely (!isDigit (view.peek ())))
                {
                    return nullptr;
                }

                int64_t exp = view.get () - '0';

                while (isDigit (view.peek ()))
                {
                    if (likely (exp < 1000))
                    {
                        exp = (10 * exp) + (view.get () - '0');
                    }
                    else
                    {
                        view.get ();
                    }
                }

                exponent += (negExp ? -exp : exp);
            }

            if (likely (digits <= 19))
            {
                if (strtodFast (neg, mantissa, exponent, value))
                {
                    return view.data ();
                }
            }

            return strtodSlow (beg, value);
        }
    }

    /**
     * @brief string to double conversion.
     * @param str string to parse.
     * @param value converted value.
     * @return end position on success, nullptr on failure.
     */
    inline const char* atod (const char* str, double& value)
    {
        View view (str);
        return details::atod (view, value);
    }

    /**
     * @brief string to double conversion.
     * @param str string to parse.
     * @param length string length.
     * @param value converted value.
     * @return end position on success, nullptr on failure.
     */
    inline const char* atod (const char* str, size_t length, double& value)
    {
        View view (str, length);
        return details::atod (view, value);
    }

    /**
     * @brief string to double conversion.
     * @param first string first position.
     * @param last string last position.
     * @param value converted value.
     * @return end position on success, nullptr on failure.
     */
    inline const char* atod (const char* first, const char* last, double& value)
    {
        View view (first, last);
        return details::atod (view, value);
    }
}

#endif
