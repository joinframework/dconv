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
#include <dconv/digits.hpp>

// libraries.
#include <gtest/gtest.h>

// C++.
#include <string>

/**
 * @brief countDigits test.
 */
TEST (digits, countDigits)
{
    uint64_t power = 1;
    for (int length = 1; length <= 17; ++length)
    {
        EXPECT_EQ (dconv::details::countDigits (power), length);
        EXPECT_EQ (dconv::details::countDigits (power * 10 - 1), length);
        power *= 10;
    }
    EXPECT_EQ (dconv::details::countDigits (0), 1);
    EXPECT_EQ (dconv::details::countDigits (UINT32_MAX), 10);
}

/**
 * @brief countTrailingZeros test.
 */
TEST (digits, countTrailingZeros)
{
    EXPECT_EQ (dconv::details::countTrailingZeros (0), 4);
    EXPECT_EQ (dconv::details::countTrailingZeros (1), 0);
    EXPECT_EQ (dconv::details::countTrailingZeros (10), 1);
    EXPECT_EQ (dconv::details::countTrailingZeros (100), 2);
    EXPECT_EQ (dconv::details::countTrailingZeros (1000), 3);
    EXPECT_EQ (dconv::details::countTrailingZeros (1200), 2);
    EXPECT_EQ (dconv::details::countTrailingZeros (1234), 0);
    EXPECT_EQ (dconv::details::countTrailingZeros (9990), 1);
}

/**
 * @brief writeDigits test.
 */
TEST (digits, writeDigits)
{
    char buffer[32] = {};

    EXPECT_EQ (dconv::details::writeDigits (buffer, 1u, 1), 0);
    EXPECT_EQ (std::string (buffer, 1), "1");

    EXPECT_EQ (dconv::details::writeDigits (buffer, 10u, 2), 1);
    EXPECT_EQ (std::string (buffer, 2), "10");

    EXPECT_EQ (dconv::details::writeDigits (buffer, 123u, 3), 0);
    EXPECT_EQ (std::string (buffer, 3), "123");

    EXPECT_EQ (dconv::details::writeDigits (buffer, 1000u, 4), 3);
    EXPECT_EQ (std::string (buffer, 4), "1000");

    EXPECT_EQ (dconv::details::writeDigits (buffer, 12340000u, 8), 4);
    EXPECT_EQ (std::string (buffer, 8), "12340000");

    EXPECT_EQ (dconv::details::writeDigits (buffer, 12345678u, 8), 0);
    EXPECT_EQ (std::string (buffer, 8), "12345678");

    EXPECT_EQ (dconv::details::writeDigits (buffer, 100000000u, 9), 8);
    EXPECT_EQ (std::string (buffer, 9), "100000000");

    EXPECT_EQ (dconv::details::writeDigits (buffer, 123456789u, 9), 0);
    EXPECT_EQ (std::string (buffer, 9), "123456789");

    EXPECT_EQ (dconv::details::writeDigits (buffer, 120000000000u, 12), 10);
    EXPECT_EQ (std::string (buffer, 12), "120000000000");

    EXPECT_EQ (dconv::details::writeDigits (buffer, 1234567800000000u, 16), 8);
    EXPECT_EQ (std::string (buffer, 16), "1234567800000000");

    EXPECT_EQ (dconv::details::writeDigits (buffer, 1234567890123400u, 16), 2);
    EXPECT_EQ (std::string (buffer, 16), "1234567890123400");

    EXPECT_EQ (dconv::details::writeDigits (buffer, 12345678901234567u, 17), 0);
    EXPECT_EQ (std::string (buffer, 17), "12345678901234567");

    EXPECT_EQ (dconv::details::writeDigits (buffer, 10000000000000000u, 17), 16);
    EXPECT_EQ (std::string (buffer, 17), "10000000000000000");
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
