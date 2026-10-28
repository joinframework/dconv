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
#include <string_view>
#include <istream>
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
    namespace details
    {
        struct alignas (64) EscapedTable
        {
            uint8_t data[256];

            constexpr EscapedTable ()
            : data{}
            {
                for (unsigned i = 0; i < 0x20; ++i)
                {
                    data[i] = 1;
                }
                data[static_cast<unsigned char> ('"')] = 1;
                data[static_cast<unsigned char> ('\\')] = 1;
            }
        };

        constexpr EscapedTable escapedLookup{};

        struct alignas (64) WhitespaceTable
        {
            uint8_t data[256];

            constexpr WhitespaceTable ()
            : data{}
            {
                data[static_cast<unsigned char> ('\t')] = 1;
                data[static_cast<unsigned char> ('\n')] = 1;
                data[static_cast<unsigned char> ('\r')] = 1;
                data[static_cast<unsigned char> (' ')] = 1;
            }
        };

        constexpr WhitespaceTable whitespaceLookup{};
    }

    /**
     * @brief string probe.
     */
    class StringProbe
    {
    public:
        using ProbePos = const char*;

        /**
         * @brief default constructor.
         * @param in input string.
         * @param count number of characters.
         */
        constexpr StringProbe (const char* in, size_t count)
        : _cur (in)
        , _beg (in)
        , _end (in ? in + count : in)
        {
        }

        /**
         * @brief default constructor.
         * @param first pointer to the first character of the string.
         * @param last pointer to the last character of the string.
         */
        constexpr StringProbe (const char* first, const char* last)
        : _cur (first)
        , _beg (first)
        , _end (last)
        {
        }

        /**
         * @brief default constructor.
         * @param in input string.
         */
        explicit StringProbe (const char* in)
        : _cur (in)
        , _beg (in)
        , _end (in ? in + std::char_traits<char>::length (in) : in)
        {
        }

        /**
         * @brief copy constructor.
         * @param other object to copy.
         */
        StringProbe (const StringProbe& other) = default;

        /**
         * @brief copy assignment.
         * @param other object to copy.
         * @return a reference of the current object.
         */
        StringProbe& operator= (const StringProbe& other) = default;

        /**
         * @brief move constructor.
         * @param other object to move.
         */
        StringProbe (StringProbe&& other) = default;

        /**
         * @brief move assignment.
         * @param other object to move.
         * @return a reference of the current object.
         */
        StringProbe& operator= (StringProbe&& other) = default;

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
         * @brief extracts expected character.
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
                   (std::memcpy (&word, cur, 8),
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
         * @brief read characters.
         * @param buf output buffer.
         * @param count number of characters to read.
         * @return number of characters read.
         */
        inline size_t read (char* buf, size_t count) noexcept
        {
            const size_t available = _end - _cur;
            const size_t nread = (count < available) ? count : available;
            std::memcpy (buf, _cur, nread);
            _cur += nread;
            return nread;
        }

        /**
         * @brief read characters until escaped, without copying them.
         * @return characters read.
         */
        inline std::string_view readUntilEscaped () noexcept
        {
            const char* beg = _cur;
            const char* cur = _cur;
            const char* end = _end;

            while (end - cur >= 8)
            {
                uint64_t word, quote, backslash;
                std::memcpy (&word, cur, 8);
                quote = word ^ 0x2222222222222222ULL;
                backslash = word ^ 0x5C5C5C5C5C5C5C5CULL;
                uint64_t mask =
                    (((quote - 0x0101010101010101ULL) & ~quote) | ((backslash - 0x0101010101010101ULL) & ~backslash) |
                     ((word - 0x2020202020202020ULL) & ~word)) &
                    0x8080808080808080ULL;
                if (mask)
                {
                    cur += __builtin_ctzll (mask) >> 3;
                    break;
                }
                cur += 8;
            }

            while (cur < end && !details::escapedLookup.data[static_cast<unsigned char> (*cur)])
            {
                ++cur;
            }

            _cur = cur;
            return std::string_view (beg, static_cast<size_t> (cur - beg));
        }

        /**
         * @brief read characters until escaped.
         * @param out output buffer.
         */
        inline void readUntilEscaped (std::string& out) noexcept
        {
            out.append (readUntilEscaped ());
        }

        /**
         * @brief skip whitespaces.
         * @return 0 on success, -1 otherwise.
         */
        inline int skipWhitespaces () noexcept
        {
            const char* cur = _cur;
            const char* end = _end;

            while (cur < end && details::whitespaceLookup.data[static_cast<unsigned char> (*cur)])
            {
                ++cur;

                uint64_t word;
                while ((end - cur >= 8) && (std::memcpy (&word, cur, 8), word == 0x2020202020202020ULL))
                {
                    cur += 8;
                }
            }

            _cur = cur;

            return 0;
        }

        /**
         * @brief skip whitespaces and comments.
         * @return 0 on success, -1 otherwise.
         */
        inline int skipWhitespacesAndComments () noexcept
        {
            const char* cur = _cur;
            const char* end = _end;

            while (cur < end)
            {
                while (cur < end && details::whitespaceLookup.data[static_cast<unsigned char> (*cur)])
                {
                    ++cur;
                }

                if (cur >= end || *cur != '/')
                {
                    break;
                }

                if (++cur >= end)
                {
                    return -1;
                }

                if (*cur == '*')
                {
                    ++cur;
                    bool closed = false;
                    while (cur < end)
                    {
                        if (*cur == '*' && (cur + 1 < end) && *(cur + 1) == '/')
                        {
                            cur += 2;
                            closed = true;
                            break;
                        }
                        ++cur;
                    }
                    if (!closed)
                    {
                        return -1;
                    }
                }
                else if (*cur == '/')
                {
                    ++cur;
                    const char* p = static_cast<const char*> (memchr (cur, '\n', end - cur));
                    cur = p ? p : end;
                }
                else
                {
                    return -1;
                }
            }

            _cur = cur;

            return 0;
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

        /// ending position.
        const char* _end = nullptr;
    };

    /**
     * @brief stream probe.
     */
    class StreamProbe
    {
    public:
        /**
         * @brief default constructor.
         * @param in input stream.
         */
        explicit StreamProbe (std::istream& in)
        : _in (in.rdbuf ())
        {
        }

        /**
         * @brief copy constructor.
         * @param other object to copy.
         */
        StreamProbe (const StreamProbe& other) = delete;

        /**
         * @brief copy assignment.
         * @param other object to copy.
         * @return a reference of the current object.
         */
        StreamProbe& operator= (const StreamProbe& other) = delete;

        /**
         * @brief move constructor.
         * @param other object to move.
         */
        StreamProbe (StreamProbe&& other) = delete;

        /**
         * @brief move assignment.
         * @param other object to move.
         * @return a reference of the current object.
         */
        StreamProbe& operator= (StreamProbe&& other) = delete;

        /**
         * @brief destroy instance.
         */
        ~StreamProbe () = default;

        /**
         * @brief get character without extracting it.
         * @return extracted character.
         */
        inline int peek () const noexcept
        {
            return _in->sgetc ();
        }

        /**
         * @brief extracts character.
         * @return extracted character.
         */
        inline int get () noexcept
        {
            return _in->sbumpc ();
        }

        /**
         * @brief extracts expected character.
         * @param expected expected character.
         * @return true if extracted, false otherwise.
         */
        inline bool getIf (char expected) noexcept
        {
            if (_in->sgetc () == static_cast<int> (static_cast<unsigned char> (expected)))
            {
                _in->sbumpc ();
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
            const int c = _in->sgetc ();
            if (DCONV_LIKELY (c != std::char_traits<char>::eof ()))
            {
                if ((static_cast<char> (c) | 32) == (expected | 32))
                {
                    _in->sbumpc ();
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
            size_t count = 0;
            while ((count < max) && (static_cast<unsigned> (peek () - '0') < 10))
            {
                value = (value * 10) + (get () - '0');
                ++count;
            }
            return count;
        }

        /**
         * @brief read characters.
         * @param buf output buffer.
         * @param count number of characters to read.
         * @return number of characters read.
         */
        inline size_t read (char* buf, size_t count) noexcept
        {
            return _in->sgetn (buf, count);
        }

        /**
         * @brief read characters until escaped.
         * @param out output buffer.
         */
        inline void readUntilEscaped (std::string& out) noexcept
        {
            int c;
            while ((c = _in->sgetc ()) != std::char_traits<char>::eof () &&
                   !details::escapedLookup.data[static_cast<unsigned char> (c)])
            {
                out.push_back (static_cast<char> (c));
                _in->sbumpc ();
            }
        }

        /**
         * @brief skip whitespaces.
         * @return 0 on success, -1 otherwise.
         */
        inline int skipWhitespaces () noexcept
        {
            int c;
            while ((c = _in->sgetc ()) != std::char_traits<char>::eof () &&
                   details::whitespaceLookup.data[static_cast<unsigned char> (c)])
            {
                _in->sbumpc ();
            }

            return 0;
        }

        /**
         * @brief skip whitespaces and comments.
         * @return 0 on success, -1 otherwise.
         */
        inline int skipWhitespacesAndComments () noexcept
        {
            int c;

            while ((c = _in->sgetc ()) != std::char_traits<char>::eof ())
            {
                while ((c = _in->sgetc ()) != std::char_traits<char>::eof () &&
                       details::whitespaceLookup.data[static_cast<unsigned char> (c)])
                {
                    _in->sbumpc ();
                }

                if (c != '/')
                {
                    break;
                }

                _in->sbumpc ();
                c = _in->sgetc ();

                if (c == std::char_traits<char>::eof ())
                {
                    return -1;
                }

                if (c == '*')
                {
                    _in->sbumpc ();
                    bool closed = false;

                    while ((c = _in->sbumpc ()) != std::char_traits<char>::eof ())
                    {
                        if (c == '*' && _in->sgetc () == '/')
                        {
                            _in->sbumpc ();
                            closed = true;
                            break;
                        }
                    }

                    if (!closed)
                    {
                        return -1;
                    }
                }
                else if (c == '/')
                {
                    _in->sbumpc ();

                    while ((c = _in->sbumpc ()) != std::char_traits<char>::eof () && c != '\n')
                    {
                    }
                }
                else
                {
                    return -1;
                }
            }

            return 0;
        }

    private:
        /// input stream buffer.
        std::streambuf* _in;
    };
}

#endif
