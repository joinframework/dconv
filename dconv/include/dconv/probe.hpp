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

#ifndef __DCONV_PROBE_HPP__
#define __DCONV_PROBE_HPP__

// C++.
#include <string>
#include <limits>

// C.
#include <cstring>
#include <cstdint>
#include <cstddef>

#define DCONV_LIKELY(x) __builtin_expect ((x), 1)
#define DCONV_UNLIKELY(x) __builtin_expect ((x), 0)

namespace dconv
{
    /**
     * @brief string probe.
     */
    class StringProbe
    {
    public:
        using ProbePos = const char*;

        /**
         * @brief default constructor.
         * @param s pointer to a character array.
         * @param count number of characters in the sequence.
         */
        explicit constexpr StringProbe (const char* s, size_t count)
        : _cur (s)
        , _beg (s)
        , _end (s ? s + count : s)
        {
        }

        /**
         * @brief default constructor.
         * @param first pointer to the first character of the sequence.
         * @param last pointer to the last character of the sequence.
         */
        constexpr StringProbe (const char* first, const char* last)
        : _cur (first)
        , _beg (first)
        , _end (last)
        {
        }

        /**
         * @brief default constructor.
         * @param s pointer to a character array.
         */
        explicit StringProbe (const char* s)
        : _cur (s)
        , _beg (s)
        , _end (s ? s + std::char_traits<char>::length (s) : s)
        {
        }

        /**
         * @brief copy constructor.
         * @param other object to copy.
         */
        StringProbe (const StringProbe& other) noexcept = default;

        /**
         * @brief copy assignment.
         * @param other object to copy.
         * @return a reference of the current object.
         */
        StringProbe& operator= (const StringProbe& other) noexcept = default;

        /**
         * @brief move constructor.
         * @param other object to move.
         */
        StringProbe (StringProbe&& other) noexcept = default;

        /**
         * @brief move assignment.
         * @param other object to move.
         * @return a reference of the current object.
         */
        StringProbe& operator= (StringProbe&& other) noexcept = default;

        /**
         * @brief destroy instance.
         */
        ~StringProbe () = default;

        /**
         * @brief get character without extracting it.
         * @return extracted character.
         */
        inline int peek () const noexcept
        {
            if (DCONV_LIKELY (_cur < _end))
            {
                return static_cast<unsigned char> (*_cur);
            }
            return std::char_traits<char>::eof ();
        }

        /**
         * @brief extracts character.
         * @return extracted character.
         */
        inline int get () noexcept
        {
            if (DCONV_LIKELY (_cur < _end))
            {
                return static_cast<unsigned char> (*_cur++);
            }
            return std::char_traits<char>::eof ();
        }

        /**
         * @brief extracts expected character (case sensitive).
         * @param expected expected character.
         * @return true if extracted, false otherwise.
         */
        inline bool getIf (char expected) noexcept
        {
            if (DCONV_LIKELY (_cur < _end) && (*_cur == expected))
            {
                ++_cur;
                return true;
            }
            return false;
        }

        /**
         * @brief extracts expected character (case insensitive, ASCII-only).
         * @param expected expected character.
         * @return true if extracted, false otherwise.
         */
        inline bool getIfNoCase (char expected) noexcept
        {
            if (DCONV_LIKELY (_cur < _end))
            {
                const char c = *_cur;
                if ((c | 32) == (expected | 32))
                {
                    ++_cur;
                    return true;
                }
            }
            return false;
        }

        /**
         * @brief read consecutive digits.
         * @param value value the digits are appended to.
         * @param max maximum number of digits to read.
         * @return number of digits read.
         */
        inline size_t readDigits (uint64_t& value, size_t max = std::numeric_limits<size_t>::max ()) noexcept
        {
            const char* cur = _cur;
            const char* end = (static_cast<size_t> (_end - _cur) > max) ? _cur + max : _end;
            uint64_t word;

            while ((end - cur >= 8) &&
                   (memcpy (&word, cur, 8),
                    ((word & 0xF0F0F0F0F0F0F0F0ULL) |
                     (((word + 0x0606060606060606ULL) & 0xF0F0F0F0F0F0F0F0ULL) >> 4)) == 0x3333333333333333ULL))
            {
                word = ((word & 0x0F0F0F0F0F0F0F0FULL) * 2561) >> 8;
                word = ((word & 0x00FF00FF00FF00FFULL) * 6553601) >> 16;
                word = ((word & 0x0000FFFF0000FFFFULL) * 42949672960001ULL) >> 32;
                value = (value * 100000000) + word;
                cur += 8;
            }

            while ((cur < end) && (static_cast<unsigned char> (*cur - '0') < 10))
            {
                value = (value * 10) + (*cur++ - '0');
            }

            size_t count = cur - _cur;
            _cur = cur;
            return count;
        }

        /**
         * @brief get input position indicator.
         * @return current position.
         */
        inline ProbePos tell () const noexcept
        {
            return _cur;
        }

        /**
         * @brief seek to the specified position.
         * @param pos position to seek to.
         */
        inline void seek (ProbePos pos) noexcept
        {
            if (DCONV_LIKELY (pos >= _beg && pos <= _end))
            {
                _cur = pos;
            }
            else if (pos < _beg)
            {
                _cur = _beg;
            }
            else
            {
                _cur = _end;
            }
        }

    private:
        /// current position.
        const char* _cur = nullptr;

        /// beginning position.
        const char* _beg = nullptr;

        /// end position.
        const char* _end = nullptr;
    };
}

#endif
