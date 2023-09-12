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
        inline const char * strtodSlow (const char * beg, double& value)
        {
            char* end = nullptr;
            static locale_t locale = newlocale (LC_ALL_MASK, "C", nullptr);
            value = strtod_l (beg, &end, locale);
            return end;
        }

        inline bool strtodFast (bool negative, uint64_t i, int64_t exponent, double& value)
        {
            static const double pow10[] = {
                1e0,  1e1,  1e2,  1e3,  1e4,  1e5,  1e6,  1e7,  1e8,  1e9,  1e10,
                1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19, 1e20, 1e21,
                1e22
            };

            if ((exponent >= -22) && (exponent <= 22) && (i <= 9007199254740991)) 
            {
                value = static_cast <double> (i);
                if (exponent < 0)
                {
                    value /= pow10[-exponent];
                }
                else
                {
                    value *= pow10[exponent];
                }
                value = negative ? -value : value;
                return true;
            }

            if (i == 0) 
            {
                value = negative ? -0.0 : 0.0;
                return true;
            }

            return false;
        }

        inline bool isDigit (char c)
        {
            return (c >= '0') && (c <= '9');
        }

        inline bool isSign (char c)
        {
            return (c == '+') || (c == '-');
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
        uint64_t i = 0, digits = 0;

        auto beg = view.data ();
        bool neg = view.getIf ('-');

        if (view.getIf ('0'))
        {
            if (unlikely (details::isDigit (view.peek ())))
            {
                return nullptr;
            }
        }
        else if (details::isDigit (view.peek ()))
        {
            i = view.get () - '0';
            ++digits;

            while (details::isDigit (view.peek ()))
            {
                i = (10 * i) + (view.get () - '0');
                ++digits;
            }
        }
        else if (view.getIfNoCase ('i') && view.getIfNoCase ('n') && view.getIfNoCase ('f'))
        {
            if (view.getIfNoCase ('i') && !(view.getIfNoCase ('n') && view.getIfNoCase ('i') && view.getIfNoCase ('t') && view.getIfNoCase ('y')))
            {
                return nullptr;
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
            if (unlikely (!details::isDigit (view.peek ())))
            {
                return nullptr;
            }

            i = (10 * i) + (view.get () - '0');
            if (i) ++digits;
            --exponent;

            while (details::isDigit (view.peek ()))
            {
                i = (10 * i) + (view.get () - '0');
                if (i) ++digits;
                --exponent;
            }
        }

        if (view.getIf ('e') || view.getIf ('E'))
        {
            bool negexp = false;

            if (details::isSign (view.peek ()))
            {
                negexp = (view.get () == '-');
            }

            if (unlikely (!details::isDigit (view.peek ())))
            {
                return nullptr;
            }

            int64_t exp = view.get () - '0';

            while (details::isDigit (view.peek ()))
            {
                if (exp < 0x100000000)
                {
                    exp = (10 * exp) + (view.get () - '0');
                }
            }

            exponent += (negexp ? -exp : exp);
        }

        if (likely ((exponent >= -325) && (exponent <= 308) && (digits < 19)))
        {
            if (details::strtodFast (neg, i, exponent, value))
            {
                return view.data ();
            }
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
