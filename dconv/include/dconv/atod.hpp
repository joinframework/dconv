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
    /**
     * @brief string to double conversion mode.
     */
    enum class AtodMode : uint32_t
    {
        Json = 0,                                      /**< JSON number grammar. */
        ParseNanAndInf = 1L << 0,                      /**< parse nan, inf and infinity. */
        ParseRelaxedNumber = 1L << 1,                  /**< parse leading zeros and missing digits around the point. */
        General = ParseNanAndInf | ParseRelaxedNumber, /**< general format. */
    };

    /**
     * @brief perform binary AND on AtodMode.
     * @param a bitset.
     * @param b other bitset.
     * @return bitset result of binary AND on AtodMode.
     */
    constexpr AtodMode operator& (AtodMode a, AtodMode b) noexcept
    {
        return AtodMode (static_cast<uint32_t> (a) & static_cast<uint32_t> (b));
    }

    /**
     * @brief perform binary OR on AtodMode.
     * @param a bitset.
     * @param b other bitset.
     * @return bitset result of binary OR on AtodMode.
     */
    constexpr AtodMode operator| (AtodMode a, AtodMode b) noexcept
    {
        return AtodMode (static_cast<uint32_t> (a) | static_cast<uint32_t> (b));
    }

    /**
     * @brief perform binary XOR on AtodMode.
     * @param a bitset.
     * @param b other bitset.
     * @return bitset result of binary XOR on AtodMode.
     */
    constexpr AtodMode operator^ (AtodMode a, AtodMode b) noexcept
    {
        return AtodMode (static_cast<uint32_t> (a) ^ static_cast<uint32_t> (b));
    }

    /**
     * @brief perform binary NOT on AtodMode.
     * @param a bitset.
     * @return bitset result of binary NOT on AtodMode.
     */
    constexpr AtodMode operator~(AtodMode a) noexcept
    {
        return AtodMode (~static_cast<uint32_t> (a));
    }

    /**
     * @brief perform binary AND assignment on AtodMode.
     * @param a bitset.
     * @param b other bitset.
     * @return reference to updated bitset.
     */
    constexpr const AtodMode& operator&= (AtodMode& a, AtodMode b) noexcept
    {
        return a = a & b;
    }

    /**
     * @brief perform binary OR assignment on AtodMode.
     * @param a bitset.
     * @param b other bitset.
     * @return reference to updated bitset.
     */
    constexpr const AtodMode& operator|= (AtodMode& a, AtodMode b) noexcept
    {
        return a = a | b;
    }

    /**
     * @brief perform binary XOR assignment on AtodMode.
     * @param a bitset.
     * @param b other bitset.
     * @return reference to updated bitset.
     */
    constexpr const AtodMode& operator^= (AtodMode& a, AtodMode b) noexcept
    {
        return a = a ^ b;
    }

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
        template <typename Probe, bool Relaxed>
        DCONV_COLD bool atodLong (Probe& probe, bool negative, uint64_t significand, int64_t& scale, bool fraction,
                                  double& value, int pending = -1)
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
                    if (DCONV_UNLIKELY (!Relaxed && !isDigit (probe.peek ())))
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
            if constexpr (Relaxed && std::is_same_v<Probe, StringProbe>)
            {
                const char* mark = probe.tell ();
                if (!readExponent (probe, exponent))
                {
                    probe.seek (mark);
                    exponent = 0;
                }
            }
            else
            {
                if (!readExponent (probe, exponent))
                {
                    return false;
                }
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
         * @brief string to double conversion in general format that stops on integers.
         * @param probe probe to read from.
         * @param value converted value.
         * @param significand integer value when the error is NotDouble, first significant digits read otherwise.
         * @param exponent decimal exponent read.
         * @param negative sign read.
         * @return Errc::Success on success, the error otherwise.
         */
        template <typename Probe, AtodMode Mode>
        inline Errc atodRelaxed (Probe& probe, double& value, uint64_t& significand, int64_t& exponent, bool& negative)
        {
            constexpr bool special = (Mode & AtodMode::ParseNanAndInf) != AtodMode::Json;

            significand = 0;
            exponent = 0;
            negative = probe.getIf ('-');
            uint64_t u = 0;
            int64_t scale = 0;
            size_t digits = 0;

            const bool zero = probe.getIf ('0');
            if (zero)
            {
                while (probe.getIf ('0'))
                {
                }
            }

            if (isDigit (probe.peek ()))
            {
                digits = probe.readDigits (u, 19);
                if (DCONV_UNLIKELY (isDigit (probe.peek ())))
                {
                    significand = u;
                    const int pending = probe.get () - '0';
                    const int next = probe.peek ();
                    if (!isDigit (next) && (next != '.') && (next != 'e') && (next != 'E') &&
                        (u <= (std::numeric_limits<uint64_t>::max () - pending) / 10))
                    {
                        significand = (u * 10) + pending;
                        return Errc::NotDouble;
                    }
                    if (!atodLong<Probe, true> (probe, negative, u, exponent, false, value, pending))
                    {
                        return Errc::InvalidNumber;
                    }
                    return Errc::Success;
                }
                significand = u;
            }
            else if (!zero && (probe.peek () != '.'))
            {
                if (special && probe.getIfNoCase ('i') && probe.getIfNoCase ('n') && probe.getIfNoCase ('f'))
                {
                    if constexpr (std::is_same_v<Probe, StringProbe>)
                    {
                        const char* mark = probe.tell ();
                        if (!(probe.getIfNoCase ('i') && probe.getIfNoCase ('n') && probe.getIfNoCase ('i') &&
                              probe.getIfNoCase ('t') && probe.getIfNoCase ('y')))
                        {
                            probe.seek (mark);
                        }
                    }
                    else
                    {
                        if (probe.getIfNoCase ('i'))
                        {
                            if (!(probe.getIfNoCase ('n') && probe.getIfNoCase ('i') && probe.getIfNoCase ('t') &&
                                  probe.getIfNoCase ('y')))
                            {
                                return Errc::InvalidNumber;
                            }
                        }
                    }
                    value =
                        negative ? -std::numeric_limits<double>::infinity () : std::numeric_limits<double>::infinity ();
                    return Errc::Success;
                }
                else if (special && probe.getIfNoCase ('n') && probe.getIfNoCase ('a') && probe.getIfNoCase ('n'))
                {
                    value = negative ? -std::numeric_limits<double>::quiet_NaN ()
                                     : std::numeric_limits<double>::quiet_NaN ();
                    return Errc::Success;
                }
                return Errc::InvalidNumber;
            }

            bool real = false;
            if (probe.getIf ('.'))
            {
                real = true;
                if (DCONV_UNLIKELY (!isDigit (probe.peek ())) && !zero && (digits == 0))
                {
                    return Errc::InvalidNumber;
                }

                if (digits == 0)
                {
                    while (probe.getIf ('0'))
                    {
                        --scale;
                    }
                }

                size_t count = probe.readDigits (u, 19 - digits);
                significand = u;
                digits += count;
                scale -= static_cast<int64_t> (count);
                exponent = scale;

                if (DCONV_UNLIKELY (isDigit (probe.peek ())))
                {
                    if (!atodLong<Probe, true> (probe, negative, u, exponent, true, value))
                    {
                        return Errc::InvalidNumber;
                    }
                    return Errc::Success;
                }
            }

            bool scientific = (probe.peek () == 'e') || (probe.peek () == 'E');
            int64_t power = 0;

            if constexpr (std::is_same_v<Probe, StringProbe>)
            {
                const char* mark = probe.tell ();
                if (DCONV_UNLIKELY (!readExponent (probe, power)))
                {
                    probe.seek (mark);
                    power = 0;
                    scientific = false;
                }
            }
            else
            {
                if (!readExponent (probe, power))
                {
                    return Errc::InvalidNumber;
                }
            }

            real |= scientific;
            if (!real)
            {
                return Errc::NotDouble;
            }

            exponent = scale + power;
            value = eisellemire::compute (u, exponent);
            value = negative ? -value : value;
            return Errc::Success;
        }

        /**
         * @brief parse again a number the fast path does not handle.
         * @param probe probe to read from, positioned on the start of the number.
         * @param value converted value.
         * @param significand integer value when the error is NotDouble, first significant digits read otherwise.
         * @param exponent decimal exponent read.
         * @param negative sign read.
         * @return Errc::Success on success, the error otherwise.
         */
        template <typename Probe, AtodMode Mode>
        DCONV_COLD Errc atodRewind (Probe& probe, double& value, uint64_t& significand, int64_t& exponent,
                                    bool& negative);

        /**
         * @brief string to double conversion that stops on integers.
         * @param probe probe to read from.
         * @param value converted value.
         * @param significand integer value when the error is NotDouble, first significant digits read otherwise.
         * @param exponent decimal exponent read.
         * @param negative sign read.
         * @return Errc::Success on success, the error otherwise.
         */
        template <typename Probe, AtodMode Mode = AtodMode::General, bool Rewind = std::is_same_v<Probe, StringProbe>>
        inline Errc atod (Probe& probe, double& value, uint64_t& significand, int64_t& exponent, bool& negative)
        {
            constexpr bool relaxed = (Mode & AtodMode::ParseRelaxedNumber) != AtodMode::Json;

            if constexpr (relaxed && !Rewind)
            {
                return atodRelaxed<Probe, Mode> (probe, value, significand, exponent, negative);
            }

            const char* beg = nullptr;
            if constexpr (Rewind)
            {
                beg = probe.tell ();
            }

            do
            {
                significand = 0;
                exponent = 0;
                negative = probe.getIf ('-');
                uint64_t u = 0;
                int64_t scale = 0;
                size_t digits = 0;

                if (probe.getIf ('0'))
                {
                    if (DCONV_UNLIKELY (isDigit (probe.peek ())))
                    {
                        if constexpr (relaxed && Rewind)
                        {
                            break;
                        }
                        else
                        {
                            return Errc::InvalidNumber;
                        }
                    }
                }
                else if (DCONV_LIKELY (isDigit (probe.peek ())))
                {
                    if constexpr (Rewind)
                    {
                        digits = probe.readDigits (u);
                    }
                    else
                    {
                        digits = probe.readDigits (u, 19);
                        if (DCONV_UNLIKELY (isDigit (probe.peek ())))
                        {
                            significand = u;
                            const int pending = probe.get () - '0';
                            const int next = probe.peek ();
                            if (!isDigit (next) && (next != '.') && (next != 'e') && (next != 'E') &&
                                (u <= (std::numeric_limits<uint64_t>::max () - pending) / 10))
                            {
                                significand = (u * 10) + pending;
                                return Errc::NotDouble;
                            }
                            if (!atodLong<Probe, relaxed> (probe, negative, u, exponent, false, value, pending))
                            {
                                return Errc::InvalidNumber;
                            }
                            return Errc::Success;
                        }
                    }
                    significand = u;
                }
                else if (((Mode & AtodMode::ParseNanAndInf) != AtodMode::Json) && probe.getIfNoCase ('i') &&
                         probe.getIfNoCase ('n') && probe.getIfNoCase ('f'))
                {
                    if (probe.getIfNoCase ('i'))
                    {
                        if (!(probe.getIfNoCase ('n') && probe.getIfNoCase ('i') && probe.getIfNoCase ('t') &&
                              probe.getIfNoCase ('y')))
                        {
                            if constexpr (relaxed && Rewind)
                            {
                                break;
                            }
                            else
                            {
                                return Errc::InvalidNumber;
                            }
                        }
                    }
                    value =
                        negative ? -std::numeric_limits<double>::infinity () : std::numeric_limits<double>::infinity ();
                    return Errc::Success;
                }
                else if (((Mode & AtodMode::ParseNanAndInf) != AtodMode::Json) && probe.getIfNoCase ('n') &&
                         probe.getIfNoCase ('a') && probe.getIfNoCase ('n'))
                {
                    value = negative ? -std::numeric_limits<double>::quiet_NaN ()
                                     : std::numeric_limits<double>::quiet_NaN ();
                    return Errc::Success;
                }
                else
                {
                    if constexpr (relaxed && Rewind)
                    {
                        break;
                    }
                    else
                    {
                        return Errc::InvalidNumber;
                    }
                }

                bool real = false;
                if (probe.getIf ('.'))
                {
                    real = true;
                    if (DCONV_UNLIKELY (!isDigit (probe.peek ())))
                    {
                        if constexpr (relaxed && Rewind)
                        {
                            break;
                        }
                        else
                        {
                            return Errc::InvalidNumber;
                        }
                    }

                    if (digits == 0)
                    {
                        while (probe.getIf ('0'))
                        {
                            --scale;
                        }
                    }

                    size_t count = probe.readDigits (u, Rewind ? std::numeric_limits<size_t>::max () : 19 - digits);
                    significand = u;
                    digits += count;
                    scale -= static_cast<int64_t> (count);
                    exponent = scale;

                    if constexpr (!Rewind)
                    {
                        if (DCONV_UNLIKELY (isDigit (probe.peek ())))
                        {
                            if (!atodLong<Probe, relaxed> (probe, negative, u, exponent, true, value))
                            {
                                return Errc::InvalidNumber;
                            }
                            return Errc::Success;
                        }
                    }
                }

                if constexpr (Rewind)
                {
                    if (DCONV_UNLIKELY (digits > 19))
                    {
                        break;
                    }
                }

                real |= (probe.peek () == 'e') || (probe.peek () == 'E');
                int64_t power = 0;

                if (!readExponent (probe, power))
                {
                    if constexpr (relaxed && Rewind)
                    {
                        break;
                    }
                    else
                    {
                        return Errc::InvalidNumber;
                    }
                }

                if (!real)
                {
                    return Errc::NotDouble;
                }

                exponent = scale + power;
                value = eisellemire::compute (u, exponent);
                value = negative ? -value : value;
                return Errc::Success;
            }
            while (false);

            if constexpr (Rewind)
            {
                probe.seek (beg);
                return atodRewind<Probe, Mode> (probe, value, significand, exponent, negative);
            }
            else
            {
                return Errc::InvalidNumber;  // LCOV_EXCL_LINE
            }
        }

        template <typename Probe, AtodMode Mode>
        DCONV_COLD Errc atodRewind (Probe& probe, double& value, uint64_t& significand, int64_t& exponent,
                                    bool& negative)
        {
            return atod<Probe, Mode, false> (probe, value, significand, exponent, negative);
        }

        /**
         * @brief string to double conversion.
         * @param probe probe to read from.
         * @param value converted value.
         * @return true on success, false otherwise.
         */
        template <typename Probe, AtodMode Mode = AtodMode::General>
        inline bool atod (Probe& probe, double& value)
        {
            uint64_t significand;
            int64_t exponent;
            bool negative;
            const Errc err = atod<Probe, Mode> (probe, value, significand, exponent, negative);
            if (err == Errc::NotDouble)
            {
                value = negative ? -static_cast<double> (significand) : static_cast<double> (significand);
                return true;
            }
            return err == Errc::Success;
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
    template <AtodMode Mode = AtodMode::General>
    inline int atod (StreamProbe& probe, double& value, uint64_t& significand, int64_t& exponent, bool& negative,
                     std::error_code& ec)
    {
        const Errc err = details::atod<StreamProbe, Mode> (probe, value, significand, exponent, negative);
        if (err != Errc::Success)
        {
            ec = make_error_code (err);
            return -1;
        }
        return 0;
    }

    /**
     * @brief stream to double conversion.
     * @param probe probe to read from.
     * @param value converted value.
     * @return true on success, false otherwise.
     */
    template <AtodMode Mode = AtodMode::General>
    inline bool atod (StreamProbe& probe, double& value)
    {
        return details::atod<StreamProbe, Mode> (probe, value);
    }

    /**
     * @brief stream to double conversion.
     * @param in stream to read from.
     * @param value converted value.
     * @return true on success, false otherwise.
     */
    template <AtodMode Mode = AtodMode::General>
    inline bool atod (std::istream& in, double& value)
    {
        StreamProbe probe (in);
        return details::atod<StreamProbe, Mode> (probe, value);
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
    template <AtodMode Mode = AtodMode::General>
    inline int atod (StringProbe& probe, double& value, uint64_t& significand, int64_t& exponent, bool& negative,
                     std::error_code& ec)
    {
        const Errc err = details::atod<StringProbe, Mode> (probe, value, significand, exponent, negative);
        if (err != Errc::Success)
        {
            ec = make_error_code (err);
            return -1;
        }
        return 0;
    }

    /**
     * @brief string to double conversion.
     * @param probe probe to read from.
     * @param value converted value.
     * @return true on success, false otherwise.
     */
    template <AtodMode Mode = AtodMode::General>
    inline bool atod (StringProbe& probe, double& value)
    {
        return details::atod<StringProbe, Mode> (probe, value);
    }

    /**
     * @brief string to double conversion.
     * @param str string to parse.
     * @param value converted value.
     * @return end position on success, nullptr on failure.
     */
    template <AtodMode Mode = AtodMode::General>
    inline const char* atod (const char* str, double& value)
    {
        StringProbe probe (str);
        return details::atod<StringProbe, Mode> (probe, value) ? probe.tell () : nullptr;
    }

    /**
     * @brief string to double conversion.
     * @param str string to parse.
     * @param length string length.
     * @param value converted value.
     * @return end position on success, nullptr on failure.
     */
    template <AtodMode Mode = AtodMode::General>
    inline const char* atod (const char* str, size_t length, double& value)
    {
        StringProbe probe (str, length);
        return details::atod<StringProbe, Mode> (probe, value) ? probe.tell () : nullptr;
    }

    /**
     * @brief string to double conversion.
     * @param first string first position.
     * @param last string last position.
     * @param value converted value.
     * @return end position on success, nullptr on failure.
     */
    template <AtodMode Mode = AtodMode::General>
    inline const char* atod (const char* first, const char* last, double& value)
    {
        StringProbe probe (first, last);
        return details::atod<StringProbe, Mode> (probe, value) ? probe.tell () : nullptr;
    }

    /**
     * @brief string to double conversion.
     * @param str string to parse.
     * @param value converted value.
     * @return end position on success, nullptr on failure.
     */
    template <AtodMode Mode = AtodMode::General>
    inline const char* atod (std::string_view str, double& value)
    {
        StringProbe probe (str.data (), str.size ());
        return details::atod<StringProbe, Mode> (probe, value) ? probe.tell () : nullptr;
    }

    /**
     * @brief string to double conversion from a temporary string.
     * @param str temporary string.
     * @param value converted value.
     * @return end position on success, nullptr on failure.
     */
    template <AtodMode Mode = AtodMode::General>
    const char* atod (std::string&& str, double& value) = delete;
}

#endif
