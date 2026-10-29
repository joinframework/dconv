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
#include <dconv/eisellemire.hpp>
#include <dconv/error.hpp>
#include <dconv/probe.hpp>

// C++.
#include <type_traits>
#include <string_view>
#include <charconv>
#include <string>
#include <limits>

// C.
#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <cerrno>

namespace dconv
{
    namespace details
    {
        inline constexpr bool isDigit (char c) noexcept
        {
            return static_cast<unsigned char> (c - '0') <= 9u;
        }

        inline constexpr bool isSign (char c) noexcept
        {
            return (c == '+') || (c == '-');
        }

        /**
         * @brief read an optional exponent.
         * @param probe probe to read from.
         * @param exponent read exponent.
         * @return true on success, false otherwise.
         */
        template <typename Probe>
        inline bool readExponent (Probe& probe, int64_t& exponent)
        {
            exponent = 0;
            if (!probe.getIf ('e') && !probe.getIf ('E'))
            {
                return true;
            }

            bool negExp = false;
            if (isSign (probe.peek ()))
            {
                negExp = (probe.get () == '-');
            }

            if (DCONV_UNLIKELY (!isDigit (probe.peek ())))
            {
                return false;
            }

            exponent = probe.get () - '0';

            while (isDigit (probe.peek ()))
            {
                int digit = probe.get () - '0';

                if (DCONV_LIKELY (exponent <= ((std::numeric_limits<int>::max () - digit) / 10)))
                {
                    exponent = (exponent * 10) + digit;
                }
            }

            if (negExp)
            {
                exponent = -exponent;
            }

            return true;
        }

        /**
         * @brief continue a conversion whose significand has more than 19 digits.
         * @param probe probe to read from, positioned on the first digit beyond the significand.
         * @param negative sign of the number.
         * @param significand first 19 significant digits.
         * @param scale power of ten applied to the significand, updated with the final exponent.
         * @param fraction true if the reading stopped in the fractional part.
         * @param value converted value.
         * @param pending integer digit already read beyond the significand, or -1.
         * @return true on success, false otherwise.
         */
        template <typename Probe>
        __attribute__ ((cold, noinline)) bool atodLong (Probe& probe, bool negative, uint64_t significand,
                                                        int64_t& scale, bool fraction, double& value, int pending = -1)
        {
            static constexpr int maxDigits = 800;
            char text[maxDigits + 32];
            char* const tail = std::to_chars (text, text + 20, significand).ptr;
            char* cur = tail;
            bool truncated = false;

            if (pending >= 0)
            {
                *cur++ = static_cast<char> ('0' + pending);
                ++scale;
            }

            if (!fraction)
            {
                while (isDigit (probe.peek ()))
                {
                    char digit = static_cast<char> (probe.get ());
                    if (cur < text + maxDigits)
                    {
                        *cur++ = digit;
                    }
                    else
                    {
                        truncated |= (digit != '0');
                    }
                    ++scale;
                }

                if (probe.getIf ('.'))
                {
                    if (DCONV_UNLIKELY (!isDigit (probe.peek ())))
                    {
                        return false;
                    }
                }
            }

            while (isDigit (probe.peek ()))
            {
                char digit = static_cast<char> (probe.get ());
                if (cur < text + maxDigits)
                {
                    *cur++ = digit;
                }
                else
                {
                    truncated |= (digit != '0');
                }
            }

            int64_t exponent = 0;
            if (!readExponent (probe, exponent))
            {
                return false;
            }

            scale += exponent;
            value = eisellemire::compute (significand, scale);
            if (value != eisellemire::compute (significand + 1, scale))
            {
                if (truncated)
                {
                    *cur++ = '1';
                }
                const int64_t power = scale - (cur - tail);
                *cur++ = 'e';
                *std::to_chars (cur, cur + 21, power).ptr = '\0';
                int error = errno;
                value = std::strtod (text, nullptr);
                errno = error;
            }

            value = negative ? -value : value;
            return true;
        }

        /**
         * @brief string to double conversion that stops on integers.
         * @param probe probe to read from.
         * @param value converted value.
         * @param significand integer value when the error is NotDouble, first significant digits read otherwise.
         * @param exponent decimal exponent read.
         * @param negative sign read.
         * @param ec error code set on failure.
         * @return 0 on success, -1 on failure.
         */
        template <typename Probe, bool Rewind = std::is_same_v<Probe, StringProbe>>
        inline int atod (Probe& probe, double& value, uint64_t& significand, int64_t& exponent, bool& negative,
                         std::error_code& ec)
        {
            const char* beg = nullptr;
            if constexpr (Rewind)
            {
                beg = probe.tell ();
            }

            significand = 0;
            exponent = 0;
            negative = probe.getIf ('-');
            size_t digits = 0;

            if (probe.getIf ('0'))
            {
                if (DCONV_UNLIKELY (isDigit (probe.peek ())))
                {
                    ec = make_error_code (Errc::InvalidNumber);
                    return -1;
                }
            }
            else if (DCONV_LIKELY (isDigit (probe.peek ())))
            {
                if constexpr (Rewind)
                {
                    digits = probe.readDigits (significand);
                }
                else
                {
                    digits = probe.readDigits (significand, 19);
                    if (DCONV_UNLIKELY (isDigit (probe.peek ())))
                    {
                        const int pending = probe.get () - '0';
                        const int next = probe.peek ();
                        if (!isDigit (next) && (next != '.') && (next != 'e') && (next != 'E') &&
                            (significand <= (std::numeric_limits<uint64_t>::max () - pending) / 10))
                        {
                            significand = (significand * 10) + pending;
                            ec = make_error_code (Errc::NotDouble);
                            return -1;
                        }
                        if (!atodLong (probe, negative, significand, exponent, false, value, pending))
                        {
                            ec = make_error_code (Errc::InvalidNumber);
                            return -1;
                        }
                        return 0;
                    }
                }
            }
            else if (probe.getIfNoCase ('i') && probe.getIfNoCase ('n') && probe.getIfNoCase ('f'))
            {
                if (probe.getIfNoCase ('i'))
                {
                    if (!(probe.getIfNoCase ('n') && probe.getIfNoCase ('i') && probe.getIfNoCase ('t') &&
                          probe.getIfNoCase ('y')))
                    {
                        ec = make_error_code (Errc::InvalidNumber);
                        return -1;
                    }
                }
                value = negative ? -std::numeric_limits<double>::infinity () : std::numeric_limits<double>::infinity ();
                return 0;
            }
            else if (probe.getIfNoCase ('n') && probe.getIfNoCase ('a') && probe.getIfNoCase ('n'))
            {
                value =
                    negative ? -std::numeric_limits<double>::quiet_NaN () : std::numeric_limits<double>::quiet_NaN ();
                return 0;
            }
            else
            {
                ec = make_error_code (Errc::InvalidNumber);
                return -1;
            }

            bool real = false;
            if (probe.getIf ('.'))
            {
                real = true;
                if (DCONV_UNLIKELY (!isDigit (probe.peek ())))
                {
                    ec = make_error_code (Errc::InvalidNumber);
                    return -1;
                }

                if (digits == 0)
                {
                    while (probe.getIf ('0'))
                    {
                        --exponent;
                    }
                }

                size_t count =
                    probe.readDigits (significand, Rewind ? std::numeric_limits<size_t>::max () : 19 - digits);
                digits += count;
                exponent -= static_cast<int64_t> (count);
                if constexpr (!Rewind)
                {
                    if (DCONV_UNLIKELY (isDigit (probe.peek ())))
                    {
                        if (!atodLong (probe, negative, significand, exponent, true, value))
                        {
                            ec = make_error_code (Errc::InvalidNumber);
                            return -1;
                        }
                        return 0;
                    }
                }
            }

            if constexpr (Rewind)
            {
                if (DCONV_UNLIKELY (digits > 19))
                {
                    probe.seek (beg);
                    return atod<Probe, false> (probe, value, significand, exponent, negative, ec);
                }
            }

            real |= (probe.peek () == 'e') || (probe.peek () == 'E');
            int64_t power = 0;
            if (!readExponent (probe, power))
            {
                ec = make_error_code (Errc::InvalidNumber);
                return -1;
            }

            if (!real)
            {
                ec = make_error_code (Errc::NotDouble);
                return -1;
            }

            exponent += power;
            value = eisellemire::compute (significand, exponent);
            value = negative ? -value : value;
            return 0;
        }

        /**
         * @brief string to double conversion.
         * @param probe probe to read from.
         * @param value converted value.
         * @return true on success, false otherwise.
         */
        template <typename Probe>
        inline bool atod (Probe& probe, double& value)
        {
            uint64_t significand;
            int64_t exponent;
            bool negative;
            std::error_code ec (0, errorCategory ());

            if (atod (probe, value, significand, exponent, negative, ec) == 0)
            {
                return true;
            }

            if (ec == Errc::NotDouble)
            {
                value = static_cast<double> (significand);
                value = negative ? -value : value;
                return true;
            }

            return false;
        }
    }

    /**
     * @brief stream to double conversion that stops on integers.
     * @param probe probe to read from.
     * @param value converted value.
     * @param significand integer value when the error is NotDouble, first significant digits read otherwise.
     * @param exponent decimal exponent read.
     * @param negative sign read.
     * @param ec error code set on failure.
     * @return 0 on success, -1 on failure.
     */
    inline int atod (StreamProbe& probe, double& value, uint64_t& significand, int64_t& exponent, bool& negative,
                     std::error_code& ec)
    {
        return details::atod (probe, value, significand, exponent, negative, ec);
    }

    /**
     * @brief stream to double conversion.
     * @param probe probe to read from.
     * @param value converted value.
     * @return true on success, false otherwise.
     */
    inline bool atod (StreamProbe& probe, double& value)
    {
        return details::atod (probe, value);
    }

    /**
     * @brief stream to double conversion.
     * @param in stream to read from.
     * @param value converted value.
     * @return true on success, false otherwise.
     */
    inline bool atod (std::istream& in, double& value)
    {
        StreamProbe probe (in);
        return details::atod (probe, value);
    }

    /**
     * @brief string to double conversion that stops on integers.
     * @param probe probe to read from.
     * @param value converted value.
     * @param significand integer value when the error is NotDouble, first significant digits read otherwise.
     * @param exponent decimal exponent read.
     * @param negative sign read.
     * @param ec error code set on failure.
     * @return 0 on success, -1 on failure.
     */
    inline int atod (StringProbe& probe, double& value, uint64_t& significand, int64_t& exponent, bool& negative,
                     std::error_code& ec)
    {
        return details::atod (probe, value, significand, exponent, negative, ec);
    }

    /**
     * @brief string to double conversion.
     * @param probe probe to read from.
     * @param value converted value.
     * @return true on success, false otherwise.
     */
    inline bool atod (StringProbe& probe, double& value)
    {
        return details::atod (probe, value);
    }

    /**
     * @brief string to double conversion.
     * @param str string to parse.
     * @param value converted value.
     * @return end position on success, nullptr on failure.
     */
    inline const char* atod (const char* str, double& value)
    {
        StringProbe probe (str);
        return details::atod (probe, value) ? probe.tell () : nullptr;
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
        StringProbe probe (str, length);
        return details::atod (probe, value) ? probe.tell () : nullptr;
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
        StringProbe probe (first, last);
        return details::atod (probe, value) ? probe.tell () : nullptr;
    }

    /**
     * @brief string to double conversion.
     * @param str string to parse.
     * @param value converted value.
     * @return end position on success, nullptr on failure.
     */
    inline const char* atod (std::string_view str, double& value)
    {
        StringProbe probe (str.data (), str.size ());
        return details::atod (probe, value) ? probe.tell () : nullptr;
    }

    /**
     * @brief string to double conversion from a temporary string.
     * @param str temporary string.
     * @param value converted value.
     * @return end position on success, nullptr on failure.
     */
    const char* atod (std::string&& str, double& value) = delete;
}

#endif
