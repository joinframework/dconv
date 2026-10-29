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
#include <dconv/grisu2.hpp>

// libraries.
#include <gtest/gtest.h>

// C++.
#include <initializer_list>
#include <string>

/**
 * @brief scaledBoundaries test.
 */
TEST (grisu2, scaledBoundaries)
{
    for (double value : {5e-324, 2.2250738585072014e-308, 0.1, 1.0, 1e23, 1.7976931348623157e308})
    {
        dconv::DiyFp w, minus, plus;
        int k = 0;
        dconv::grisu2::scaledBoundaries (value, w, minus, plus, k);
        EXPECT_EQ (w._exponent, plus._exponent);
        EXPECT_EQ (minus._exponent, plus._exponent);
        EXPECT_LT (minus._mantissa, w._mantissa);
        EXPECT_LT (w._mantissa, plus._mantissa);
    }
}

/**
 * @brief round test.
 */
TEST (grisu2, round)
{
    char buffer[] = "13";
    dconv::grisu2::round (buffer, 2, 100, 10, 10, 35);
    EXPECT_STREQ (buffer, "11");

    char unchanged[] = "13";
    dconv::grisu2::round (unchanged, 2, 100, 40, 10, 35);
    EXPECT_STREQ (unchanged, "13");
}

/**
 * @brief digitsGen test.
 */
TEST (grisu2, digitsGen)
{
    dconv::DiyFp w, minus, plus;
    int k = 0, length = 0;
    char buffer[32] = {};

    dconv::grisu2::scaledBoundaries (0.3, w, minus, plus, k);
    ++minus._mantissa;
    --plus._mantissa;
    dconv::grisu2::digitsGen (w, plus, plus._mantissa - minus._mantissa, buffer, length, k);
    EXPECT_EQ (std::string (buffer, length), "3");
    EXPECT_EQ (k, -1);

    dconv::grisu2::scaledBoundaries (1234567890123456.0, w, minus, plus, k);
    ++minus._mantissa;
    --plus._mantissa;
    dconv::grisu2::digitsGen (w, plus, plus._mantissa - minus._mantissa, buffer, length, k);
    EXPECT_EQ (std::string (buffer, length), "1234567890123456");
    EXPECT_EQ (k, 0);
}

/**
 * @brief generate test.
 */
TEST (grisu2, generate)
{
    char buffer[32] = {};
    int length = 0, k = 0;

    dconv::grisu2::generate (buffer, length, k, 1e23);
    EXPECT_EQ (std::string (buffer, length), "9999999999999999");
    EXPECT_EQ (k, 7);
}

/**
 * @brief dtoa test.
 */
TEST (grisu2, dtoa)
{
    char beg[32] = {};
    char* end = nullptr;

    end = dconv::grisu2::dtoa (beg, 0.0);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "0.0");

    end = dconv::grisu2::dtoa (beg, -0.0);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "-0.0");

    end = dconv::grisu2::dtoa (beg, 0.1);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "0.1");

    end = dconv::grisu2::dtoa (beg, -1.5);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "-1.5");

    end = dconv::grisu2::dtoa (beg, 123456789.125);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "123456789.125");

    end = dconv::grisu2::dtoa (beg, 1e21);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "1e21");

    end = dconv::grisu2::dtoa (beg, 1e23);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "9.999999999999999e22");

    end = dconv::grisu2::dtoa (beg, 0.000123);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "0.000123");

    end = dconv::grisu2::dtoa (beg, 1.2345e-7);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "1.2345e-7");

    end = dconv::grisu2::dtoa (beg, 5e-324);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "5e-324");

    end = dconv::grisu2::dtoa (beg, 2.2250738585072014e-308);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "2.2250738585072014e-308");

    end = dconv::grisu2::dtoa (beg, 1.7976931348623157e308);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "1.7976931348623157e308");
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
