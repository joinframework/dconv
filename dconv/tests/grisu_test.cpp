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
#include <dconv/grisu.hpp>

// libraries.
#include <gtest/gtest.h>

// C++.
#include <string>

/**
 * @brief cachedPower test.
 */
TEST (grisu, cachedPower)
{
    for (int exponent = -1137; exponent <= 960; ++exponent)
    {
        int k = 0;
        const dconv::DiyFp& power = dconv::grisu::cachedPower (exponent, k);
        const int scaled = exponent + power._exponent + 64;
        EXPECT_GE (scaled, -60) << exponent;
        EXPECT_LE (scaled, -32) << exponent;
    }
}

/**
 * @brief generate test.
 */
TEST (grisu, generate)
{
    char buffer[32] = {};
    int length = 0, k = 0;

    dconv::grisu::generate (buffer, length, k, 0.5);
    EXPECT_EQ (std::string (buffer, length), "50");
    EXPECT_EQ (k, -2);

    dconv::grisu::generate (buffer, length, k, 0.1);
    EXPECT_EQ (std::string (buffer, length), "100000000000000005");
    EXPECT_EQ (k, -18);
}

/**
 * @brief dtoa test.
 */
TEST (grisu, dtoa)
{
    char beg[32] = {};
    char* end = nullptr;

    end = dconv::grisu::dtoa (beg, 0.0);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "0.0");

    end = dconv::grisu::dtoa (beg, -0.0);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "-0.0");

    end = dconv::grisu::dtoa (beg, 1.0);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "1.0");

    end = dconv::grisu::dtoa (beg, -1.0);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "-1.0");

    end = dconv::grisu::dtoa (beg, 0.1);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "0.100000000000000005");

    end = dconv::grisu::dtoa (beg, 1024.0);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "1024.00000000000000");

    end = dconv::grisu::dtoa (beg, 1e21);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "1.000e21");

    end = dconv::grisu::dtoa (beg, 1e23);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "9.99999999999999916e22");

    end = dconv::grisu::dtoa (beg, 5e-324);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "4.94065645841246544e-324");

    end = dconv::grisu::dtoa (beg, 1.7976931348623157e308);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "1.79769313486231570e308");

    end = dconv::grisu::dtoa (beg, 0.00001);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "0.0000100000000000000008");
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
