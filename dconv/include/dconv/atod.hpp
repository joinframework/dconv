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
#include <dconv/view.hpp>

// C++.
#include <limits>

// C.
#include <cstdlib>

#define likely(x)   __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x), 0)

namespace dconv
{
    namespace details
    {
        struct fp
        {
            uint64_t significand = 0;
            uint64_t digits = 0;
            int64_t exponent = 0;
            int64_t frac = 0;
            bool negative = false;
        };

        inline bool strtodFast (uint64_t significand, int64_t exponent, double& value)
        {
            static const double pow10[] = {
                1e0,  1e1,  1e2,  1e3,  1e4,  1e5,  1e6,  1e7,  1e8,  1e9,  1e10,
                1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19, 1e20, 1e21,
                1e22
            };

            value = static_cast <double> (significand);

            if ((exponent > 22) && (exponent < (22 + 16)))
            {
                value *= pow10[exponent - 22];
                exponent = 22;
            }

            if ((exponent >= -22) && (exponent <= 22) && (value <= 9007199254740991.0))
            {
                if (exponent < 0)
                {
                    value /= pow10[-exponent];
                }
                else
                {
                    value *= pow10[exponent];
                }
                return true;
            }

            if (value == 0)
            {
                value = 0.0;
                return true;
            }

            return false;
        }

        inline const char * strtodSlow (const char * beg, double& value)
        {
            char* end = nullptr;
            static locale_t locale = newlocale (LC_ALL_MASK, "C", nullptr);
            value = strtod_l (beg, &end, locale);
            return end;
        }

        inline bool isDigit (char c)
        {
            return (c >= '0') && (c <= '9');
        }

        inline bool isSign (char c)
        {
            return (c == '+') || (c == '-');
        }

        inline bool isExponent (char c)
        {
            return (c == 'e') || (c == 'E');
        }

        inline bool isDot (char c)
        {
            return (c == '.');
        }
    }

    /**
     * @brief string to double conversion.
     * @param view string view.
     * @param value converted value.
     * @return end position on success, nullptr on failure.
     */
    inline const char * atod (View& view, double& value)
    {
        details::fp f;

        auto beg = view.data ();
        f.negative = view.getIf ('-');

        if (unlikely (view.getIf ('0')))
        {
            if (unlikely (details::isDigit (view.peek ())))
            {
                return nullptr;
            }
        }
        else if (likely (details::isDigit (view.peek ())))
        {
            f.significand = (view.get () - '0');
            ++f.digits;

            while (likely (details::isDigit (view.peek ())))
            {
                f.significand = (f.significand * 10) + (view.get () - '0');
                ++f.digits;
            }
        }
        else if (likely (view.getIfNoCase ('i') && view.getIfNoCase ('n') && view.getIfNoCase ('f')))
        {
            if (unlikely (view.getIfNoCase ('i') && !(view.getIfNoCase ('n') && view.getIfNoCase ('i') && view.getIfNoCase ('t') && view.getIfNoCase ('y'))))
            {
                return nullptr;
            }

            value = f.negative ? -std::numeric_limits <double>::infinity () : std::numeric_limits <double>::infinity ();

            return view.data ();
        }
        else if (likely (view.getIfNoCase ('n') && view.getIfNoCase ('a') && view.getIfNoCase ('n')))
        {
            value = f.negative ? -std::numeric_limits <double>::quiet_NaN () : std::numeric_limits <double>::quiet_NaN ();

            return view.data ();
        }
        else
        {
            return nullptr;
        }

        if (view.getIf ('.'))
        {
            while (likely (details::isDigit (view.peek ())))
            {
                if (f.significand || f.digits)
                {
                    ++f.digits;
                }
                --f.frac;

                f.significand = (f.significand * 10) + (view.get () - '0');
            }
        }

        if (view.getIf ('e') || view.getIf ('E'))
        {
            bool negExp = false;

            if (details::isSign (view.peek ()))
            {
                negExp = (view.get () == '-');
            }
            
            if (likely (details::isDigit (view.peek ())))
            {
                f.exponent = (view.get () - '0');

                while (likely (details::isDigit (view.peek ())))
                {
                    int digit = view.peek () - '0';

                    if (likely (f.exponent <= ((std::numeric_limits <int>::max () - digit) / 10)))
                    {
                        f.exponent = (f.exponent * 10) + digit;
                    }

                    view.get ();
                }
            }
            else
            {
                return nullptr;
            }

            if (negExp)
            {
                f.exponent = -f.exponent;
            }
        }

        f.exponent += f.frac;

        if (unlikely ((f.exponent < -325) || (f.exponent > 308) || (f.digits > 19)))
        {
            return details::strtodSlow (beg, value);
        }

        if (details::strtodFast (f.significand, f.exponent, value))
        {
            if (f.negative)
            {
                value = -value;
            }

            return view.data ();
        }

        return details::strtodSlow (beg, value);
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
        return atod (view, value);
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
        return atod (view, value);
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
        return atod (view, value);
    }
}

#endif
