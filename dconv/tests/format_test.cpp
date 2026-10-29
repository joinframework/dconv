/**
 * MIT License
 *
 * Copyright (c) 2026 Mathieu Rabine
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
#include <dconv/format.hpp>

// libraries.
#include <gtest/gtest.h>

// C++.
#include <cstring>
#include <string>

/**
 * @brief writeExponent test.
 */
TEST (format, writeExponent)
{
    char buffer[8] = {};
    char* end = nullptr;

    end = dconv::details::writeExponent (buffer, 0);
    EXPECT_EQ (std::string (buffer, end), "0");

    end = dconv::details::writeExponent (buffer, 7);
    EXPECT_EQ (std::string (buffer, end), "7");

    end = dconv::details::writeExponent (buffer, -7);
    EXPECT_EQ (std::string (buffer, end), "-7");

    end = dconv::details::writeExponent (buffer, 42);
    EXPECT_EQ (std::string (buffer, end), "42");

    end = dconv::details::writeExponent (buffer, -42);
    EXPECT_EQ (std::string (buffer, end), "-42");

    end = dconv::details::writeExponent (buffer, 308);
    EXPECT_EQ (std::string (buffer, end), "308");

    end = dconv::details::writeExponent (buffer, -324);
    EXPECT_EQ (std::string (buffer, end), "-324");
}

/**
 * @brief prettify test.
 */
TEST (format, prettify)
{
    char buffer[40] = {};
    char* end = nullptr;

    std::memcpy (buffer, "12345", 5);
    end = dconv::details::prettify (buffer, 5, 0);
    EXPECT_EQ (std::string (buffer, end), "12345.0");

    std::memcpy (buffer, "12345", 5);
    end = dconv::details::prettify (buffer, 5, 2);
    EXPECT_EQ (std::string (buffer, end), "1234500.0");

    std::memcpy (buffer, "12345", 5);
    end = dconv::details::prettify (buffer, 5, -2);
    EXPECT_EQ (std::string (buffer, end), "123.45");

    std::memcpy (buffer, "12345", 5);
    end = dconv::details::prettify (buffer, 5, -5);
    EXPECT_EQ (std::string (buffer, end), "0.12345");

    std::memcpy (buffer, "12345", 5);
    end = dconv::details::prettify (buffer, 5, -7);
    EXPECT_EQ (std::string (buffer, end), "0.0012345");

    std::memcpy (buffer, "12345", 5);
    end = dconv::details::prettify (buffer, 5, -11);
    EXPECT_EQ (std::string (buffer, end), "1.2345e-7");

    std::memcpy (buffer, "12345", 5);
    end = dconv::details::prettify (buffer, 5, 17);
    EXPECT_EQ (std::string (buffer, end), "1.2345e21");

    std::memcpy (buffer, "1", 1);
    end = dconv::details::prettify (buffer, 1, -7);
    EXPECT_EQ (std::string (buffer, end), "1e-7");

    std::memcpy (buffer, "1", 1);
    end = dconv::details::prettify (buffer, 1, 30);
    EXPECT_EQ (std::string (buffer, end), "1e30");

    std::memcpy (buffer, "123456789012345678901", 21);
    end = dconv::details::prettify (buffer, 21, -1);
    EXPECT_EQ (std::string (buffer, end), "12345678901234567890.1");

    std::memcpy (buffer, "123456789012345678", 18);
    end = dconv::details::prettify (buffer, 18, -1);
    EXPECT_EQ (std::string (buffer, end), "12345678901234567.8");
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
