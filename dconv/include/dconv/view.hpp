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

#ifndef __DCONV_VIEW_HPP__
#define __DCONV_VIEW_HPP__

// C++.
#include <string>

// C.
#include <cstring>
#include <cstddef>

#define likely(x)   __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x), 0)

namespace dconv
{
    /**
     * @brief char array view.
     */
    class View
    {
    public:
        /**
         * @brief default constructor.
         * @param s pointer to a character array.
         * @param count number of characters in the sequence.
         */
        constexpr View (const char * s, size_t count)
        : _pos (s)
        , _end (s ? s + count : s)
        {
        }

        /**
         * @brief default constructor.
         * @param first pointer to the first character of the sequence.
         * @param last pointer to the last character of the sequence.
         */
        constexpr View (const char * first, const char * last)
        : _pos (first)
        , _end (last)
        {
        }

        /**
         * @brief default constructor.
         * @param s pointer to a character array.
         */
        View (const char * s)
        : _pos (s)
        , _end (s ? s + std::char_traits <char>::length (s) : s)
        {
        }

        /**
         * @brief copy constructor.
         * @param other object to copy.
         */
        View (const View& other) noexcept = default;

        /**
         * @brief copy assignment.
         * @param other object to copy.
         * @return a reference of the current object.
         */
        View& operator= (const View& other) noexcept = default;

        /**
         * @brief move constructor.
         * @param other object to move.
         */
        View (View&& other) noexcept = default;

        /**
         * @brief move assignment.
         * @param other object to move.
         * @return a reference of the current object.
         */
        View& operator=(View&& other) noexcept = default;

        /**
         * @brief destroy instance.
         */
        ~View () = default;

        /**
         * @brief get character without extracting it.
         * @return extracted character.
         */
        inline int peek () const noexcept
        {
            if (likely (_pos < _end))
            {
                return static_cast <unsigned char> (*_pos);
            }
            return std::char_traits <char>::eof ();
        }

        /**
         * @brief extracts character.
         * @return extracted character.
         */
        inline int get () noexcept
        {
            if (likely (_pos < _end))
            {
                return static_cast <unsigned char> (*_pos++);
            }
            return std::char_traits <char>::eof ();
        }

        /**
         * @brief extracts expected character (case sensitive).
         * @param expected expected character.
         * @return true if extracted, false otherwise.
         */
        inline bool getIf (char expected) noexcept
        {
            if (likely (_pos < _end) && (*_pos == expected))
            {
                ++_pos;
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
            if (likely (_pos < _end))
            {
                const char c = *_pos;
                if ((c | 32) == (expected | 32))
                {
                    ++_pos;
                    return true;
                }
            }
            return false;
        }

        /**
         * @brief read characters.
         * @param buf output buffer.
         * @param count number of characters to read.
         * @return number of characters read.
         */
        inline size_t read (char* buf, size_t count) noexcept
        {
            const size_t available = _end - _pos;
            const size_t nread = (count < available) ? count : available;
            std::memcpy (buf, _pos, nread);
            _pos += nread;
            return nread;
        }

        /**
         * @brief returns a pointer to the first character of a view.
         * @return a pointer to the first character of a view.
         */
        inline const char * data () const noexcept
        {
            return _pos;
        }

        /**
         * @brief returns the number of characters in the view.
         * @return the number of characters in the view.
         */
        inline size_t size () const noexcept
        {
            return _end - _pos;
        }

    private:
        /// current position.
        const char * _pos = nullptr;

        /// end position.
        const char * _end = nullptr;
    };
}

#endif
