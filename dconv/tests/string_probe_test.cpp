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

// dconv.
#include <dconv/probe.hpp>

// libraries.
#include <gtest/gtest.h>

// C++.
#include <string>

using dconv::StringProbe;

/**
 * @brief create test.
 */
TEST (StringProbe, create)
{
    char msg[] = "hello world";

    StringProbe probe1 (msg);
    ASSERT_EQ (probe1.tell (), msg);
    ASSERT_EQ (probe1.peek (), 'h');

    StringProbe probe2 (msg, strlen (msg));
    ASSERT_EQ (probe2.tell (), msg);
    ASSERT_EQ (probe2.peek (), 'h');

    StringProbe probe3 (msg, msg + strlen (msg));
    ASSERT_EQ (probe3.tell (), msg);
    ASSERT_EQ (probe3.peek (), 'h');
}

/**
 * @brief assign test.
 */
TEST (StringProbe, assign)
{
    const char* other = "other";
    StringProbe probe = StringProbe (other);
    ASSERT_EQ (probe.tell (), other);
    ASSERT_EQ (probe.peek (), 'o');

    const char* msg = "hello world";
    probe = StringProbe (msg);
    ASSERT_EQ (probe.tell (), msg);
    ASSERT_EQ (probe.peek (), 'h');
}

/**
 * @brief peek test.
 */
TEST (StringProbe, peek)
{
    const char* msg = "hello world";
    StringProbe probe (msg);
    ASSERT_EQ (probe.peek (), 'h');
    ASSERT_EQ (probe.tell (), msg);
    ASSERT_EQ (probe.peek (), 'h');

    probe = StringProbe ("");
    ASSERT_EQ (probe.peek (), std::char_traits<char>::eof ());
}

/**
 * @brief get test.
 */
TEST (StringProbe, get)
{
    const char* msg = "hello world";
    StringProbe probe (msg);
    for (size_t i = 0; i < strlen (msg); ++i)
    {
        ASSERT_EQ (probe.tell (), msg + i);
        ASSERT_EQ (probe.get (), msg[i]);
    }
    ASSERT_EQ (probe.tell (), msg + strlen (msg));
    ASSERT_EQ (probe.get (), std::char_traits<char>::eof ());
}

/**
 * @brief getIf test.
 */
TEST (StringProbe, getIf)
{
    const char* msg = "hello world";
    StringProbe probe (msg);
    ASSERT_FALSE (probe.getIf ('x'));
    ASSERT_EQ (probe.tell (), msg);
    ASSERT_TRUE (probe.getIf ('h'));
    ASSERT_EQ (probe.tell (), msg + 1);
    ASSERT_EQ (probe.peek (), 'e');

    probe = StringProbe ("");
    ASSERT_FALSE (probe.getIf ('h'));
}

/**
 * @brief getIfNoCase test.
 */
TEST (StringProbe, getIfNoCase)
{
    const char* msg = "Hello";
    StringProbe probe (msg);
    ASSERT_FALSE (probe.getIfNoCase ('x'));
    ASSERT_EQ (probe.tell (), msg);
    ASSERT_TRUE (probe.getIfNoCase ('h'));
    ASSERT_TRUE (probe.getIfNoCase ('E'));
    ASSERT_EQ (probe.tell (), msg + 2);

    probe = StringProbe ("");
    ASSERT_FALSE (probe.getIfNoCase ('h'));
}

/**
 * @brief readDigits test.
 */
TEST (StringProbe, readDigits)
{
    uint64_t value = 0;
    StringProbe probe ("1234567890123456789x");
    ASSERT_EQ (probe.readDigits (value), 19);
    ASSERT_EQ (value, 1234567890123456789ULL);
    ASSERT_EQ (probe.peek (), 'x');

    value = 0;
    probe = StringProbe ("12345678");
    ASSERT_EQ (probe.readDigits (value), 8);
    ASSERT_EQ (value, 12345678);
    ASSERT_EQ (probe.peek (), std::char_traits<char>::eof ());

    value = 0;
    probe = StringProbe ("123.5");
    ASSERT_EQ (probe.readDigits (value), 3);
    ASSERT_EQ (value, 123);
    ASSERT_EQ (probe.peek (), '.');

    value = 7;
    probe = StringProbe ("123456789012");
    ASSERT_EQ (probe.readDigits (value, 10), 10);
    ASSERT_EQ (value, 71234567890ULL);
    ASSERT_EQ (probe.peek (), '1');

    value = 0;
    probe = StringProbe ("1234567a90");
    ASSERT_EQ (probe.readDigits (value), 7);
    ASSERT_EQ (value, 1234567);
    ASSERT_EQ (probe.peek (), 'a');

    value = 0;
    probe = StringProbe ("");
    ASSERT_EQ (probe.readDigits (value), 0);
    ASSERT_EQ (value, 0);
}

/**
 * @brief read test.
 */
TEST (StringProbe, read)
{
    StringProbe probe ("hello world");
    char buf[8] = {};

    ASSERT_EQ (probe.read (buf, 5), 5);
    ASSERT_EQ (probe.read (buf, 8), 6);
    ASSERT_EQ (probe.read (buf, 8), 0);
}

/**
 * @brief readUntilEscaped test.
 */
TEST (StringProbe, readUntilEscaped)
{
    StringProbe probe ("hello\"world");
    std::string out;

    probe.readUntilEscaped (out);
    ASSERT_EQ (out, "hello");
    ASSERT_EQ (probe.peek (), '"');

    probe = "hello wonderful world\\";
    out.clear ();
    probe.readUntilEscaped (out);
    ASSERT_EQ (out, "hello wonderful world");
    ASSERT_EQ (probe.peek (), '\\');

    probe = "abc";
    out.clear ();
    probe.readUntilEscaped (out);
    ASSERT_EQ (out, "abc");
    ASSERT_EQ (probe.peek (), std::char_traits<char>::eof ());
}

/**
 * @brief skipWhitespaces test.
 */
TEST (StringProbe, skipWhitespaces)
{
    StringProbe probe ("   hello");

    probe.skipWhitespaces ();
    ASSERT_EQ (probe.peek (), 'h');

    probe = "\t                    hello";
    probe.skipWhitespaces ();
    ASSERT_EQ (probe.peek (), 'h');
}

/**
 * @brief skipWhitespacesAndComments test.
 */
TEST (StringProbe, skipWhitespacesAndComments)
{
    StringProbe probe ("   // comment\nhello");
    ASSERT_EQ (probe.skipWhitespacesAndComments (), 0);
    ASSERT_EQ (probe.peek (), 'h');

    probe = "   /* comment */hello";
    ASSERT_EQ (probe.skipWhitespacesAndComments (), 0);
    ASSERT_EQ (probe.peek (), 'h');

    probe = "// comment";
    ASSERT_EQ (probe.skipWhitespacesAndComments (), 0);
    ASSERT_EQ (probe.peek (), std::char_traits<char>::eof ());

    probe = "/* comment */";
    ASSERT_EQ (probe.skipWhitespacesAndComments (), 0);
    ASSERT_EQ (probe.peek (), std::char_traits<char>::eof ());

    probe = "/";
    ASSERT_EQ (probe.skipWhitespacesAndComments (), -1);

    probe = "/*";
    ASSERT_EQ (probe.skipWhitespacesAndComments (), -1);

    probe = "/!";
    ASSERT_EQ (probe.skipWhitespacesAndComments (), -1);
}

/**
 * @brief tell test.
 */
TEST (StringProbe, tell)
{
    const char* msg = "hello world";
    StringProbe probe (msg);
    ASSERT_EQ (probe.tell (), msg);
    probe.get ();
    ASSERT_EQ (probe.tell (), msg + 1);
}

/**
 * @brief seek test.
 */
TEST (StringProbe, seek)
{
    const char* msg = "hello world";
    StringProbe probe (msg + 2, msg + 6);
    probe.seek (msg + 4);
    ASSERT_EQ (probe.tell (), msg + 4);
    ASSERT_EQ (probe.peek (), 'o');

    probe.seek (msg);
    ASSERT_EQ (probe.tell (), msg + 2);

    probe.seek (msg + 10);
    ASSERT_EQ (probe.tell (), msg + 6);
    ASSERT_EQ (probe.peek (), std::char_traits<char>::eof ());
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
