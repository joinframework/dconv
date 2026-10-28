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
#include <sstream>
#include <string>

using dconv::StreamProbe;

/**
 * @brief create test.
 */
TEST (StreamProbe, create)
{
    std::stringstream msg ("hello world");
    StreamProbe probe (msg);

    ASSERT_NE (probe.peek (), std::char_traits<char>::eof ());
}

/**
 * @brief peek test.
 */
TEST (StreamProbe, peek)
{
    std::stringstream msg ("hello world");
    StreamProbe probe (msg);

    ASSERT_EQ (probe.peek (), 'h');
    ASSERT_EQ (probe.get (), 'h');
    ASSERT_EQ (probe.peek (), 'e');
    ASSERT_EQ (probe.get (), 'e');
    ASSERT_EQ (probe.peek (), 'l');
    ASSERT_EQ (probe.get (), 'l');
    ASSERT_EQ (probe.peek (), 'l');
    ASSERT_EQ (probe.get (), 'l');
    ASSERT_EQ (probe.peek (), 'o');
    ASSERT_EQ (probe.get (), 'o');
    ASSERT_EQ (probe.peek (), ' ');
    ASSERT_EQ (probe.get (), ' ');
    ASSERT_EQ (probe.peek (), 'w');
    ASSERT_EQ (probe.get (), 'w');
    ASSERT_EQ (probe.peek (), 'o');
    ASSERT_EQ (probe.get (), 'o');
    ASSERT_EQ (probe.peek (), 'r');
    ASSERT_EQ (probe.get (), 'r');
    ASSERT_EQ (probe.peek (), 'l');
    ASSERT_EQ (probe.get (), 'l');
    ASSERT_EQ (probe.peek (), 'd');
    ASSERT_EQ (probe.get (), 'd');
    ASSERT_EQ (probe.peek (), std::char_traits<char>::eof ());
}

/**
 * @brief get test.
 */
TEST (StreamProbe, get)
{
    std::stringstream msg ("hello world");
    StreamProbe probe (msg);

    ASSERT_EQ (probe.get (), 'h');
    ASSERT_EQ (probe.get (), 'e');
    ASSERT_EQ (probe.get (), 'l');
    ASSERT_EQ (probe.get (), 'l');
    ASSERT_EQ (probe.get (), 'o');
    ASSERT_EQ (probe.get (), ' ');
    ASSERT_EQ (probe.get (), 'w');
    ASSERT_EQ (probe.get (), 'o');
    ASSERT_EQ (probe.get (), 'r');
    ASSERT_EQ (probe.get (), 'l');
    ASSERT_EQ (probe.get (), 'd');
    ASSERT_EQ (probe.get (), std::char_traits<char>::eof ());
}

/**
 * @brief getIf test.
 */
TEST (StreamProbe, getIf)
{
    std::stringstream msg ("hello world");
    StreamProbe probe (msg);

    ASSERT_FALSE (probe.getIf ('X'));
    ASSERT_FALSE (probe.getIf ('x'));
    ASSERT_FALSE (probe.getIf ('H'));
    ASSERT_TRUE (probe.getIf ('h'));
    ASSERT_FALSE (probe.getIf ('E'));
    ASSERT_TRUE (probe.getIf ('e'));
}

/**
 * @brief getIfNoCase test.
 */
TEST (StreamProbe, getIfNoCase)
{
    std::stringstream msg ("hello world");
    StreamProbe probe (msg);

    ASSERT_FALSE (probe.getIfNoCase ('x'));
    ASSERT_FALSE (probe.getIfNoCase ('X'));
    ASSERT_TRUE (probe.getIfNoCase ('h'));
    ASSERT_TRUE (probe.getIfNoCase ('E'));
    ASSERT_TRUE (probe.getIfNoCase ('l'));
    ASSERT_TRUE (probe.getIfNoCase ('L'));
}

/**
 * @brief readDigits test.
 */
TEST (StreamProbe, readDigits)
{
    std::stringstream msg ("1234567890123456789x");
    StreamProbe probe (msg);
    uint64_t value = 0;

    ASSERT_EQ (probe.readDigits (value), 19);
    ASSERT_EQ (value, 1234567890123456789ULL);
    ASSERT_EQ (probe.peek (), 'x');

    std::stringstream limited ("123456789012");
    StreamProbe probe2 (limited);
    value = 7;
    ASSERT_EQ (probe2.readDigits (value, 10), 10);
    ASSERT_EQ (value, 71234567890ULL);
    ASSERT_EQ (probe2.peek (), '1');
}

/**
 * @brief read test.
 */
TEST (StreamProbe, read)
{
    std::stringstream msg ("hello world");
    StreamProbe probe (msg);
    char buf[8] = {};

    ASSERT_EQ (probe.read (buf, 5), 5);
    ASSERT_EQ (probe.read (buf, 8), 6);
    ASSERT_EQ (probe.read (buf, 8), 0);
}

/**
 * @brief readUntilEscaped test.
 */
TEST (StreamProbe, readUntilEscaped)
{
    std::stringstream msg ("hello\"world");
    StreamProbe probe (msg);
    std::string out;

    probe.readUntilEscaped (out);
    ASSERT_EQ (out, "hello");
    ASSERT_EQ (probe.peek (), '"');
}

/**
 * @brief skipWhitespaces test.
 */
TEST (StreamProbe, skipWhitespaces)
{
    std::stringstream msg ("   hello");
    StreamProbe probe (msg);
    std::string out;

    probe.skipWhitespaces ();
    ASSERT_EQ (probe.peek (), 'h');
}

/**
 * @brief skipWhitespacesAndComments test.
 */
TEST (StreamProbe, skipWhitespacesAndComments)
{
    std::stringstream msg ("   // comment\nhello");
    StreamProbe probe1 (msg);
    ASSERT_EQ (probe1.skipWhitespacesAndComments (), 0);
    ASSERT_EQ (probe1.peek (), 'h');

    msg.str ("   /* comment */hello");
    msg.clear ();
    StreamProbe probe2 (msg);
    ASSERT_EQ (probe2.skipWhitespacesAndComments (), 0);
    ASSERT_EQ (probe2.peek (), 'h');

    msg.str ("// comment");
    msg.clear ();
    StreamProbe probe3 (msg);
    ASSERT_EQ (probe3.skipWhitespacesAndComments (), 0);
    ASSERT_EQ (probe3.peek (), std::char_traits<char>::eof ());

    msg.str ("/* comment */");
    msg.clear ();
    StreamProbe probe4 (msg);
    ASSERT_EQ (probe4.skipWhitespacesAndComments (), 0);
    ASSERT_EQ (probe4.peek (), std::char_traits<char>::eof ());

    msg.str ("/");
    msg.clear ();
    StreamProbe probe5 (msg);
    ASSERT_EQ (probe5.skipWhitespacesAndComments (), -1);

    msg.str ("/*");
    msg.clear ();
    StreamProbe probe6 (msg);
    ASSERT_EQ (probe6.skipWhitespacesAndComments (), -1);

    msg.str ("/!");
    msg.clear ();
    StreamProbe probe7 (msg);
    ASSERT_EQ (probe7.skipWhitespacesAndComments (), -1);
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
