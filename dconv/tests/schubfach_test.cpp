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
#include <dconv/schubfach.hpp>

// libraries.
#include <gtest/gtest.h>

// C++.
#include <string>

/**
 * @brief floorLog10Pow2 test.
 */
TEST (schubfach, floorLog10Pow2)
{
    EXPECT_EQ (dconv::schubfach::floorLog10Pow2 (0, false), 0);
    EXPECT_EQ (dconv::schubfach::floorLog10Pow2 (3, false), 0);
    EXPECT_EQ (dconv::schubfach::floorLog10Pow2 (4, false), 1);
    EXPECT_EQ (dconv::schubfach::floorLog10Pow2 (10, false), 3);
    EXPECT_EQ (dconv::schubfach::floorLog10Pow2 (-1, false), -1);
    EXPECT_EQ (dconv::schubfach::floorLog10Pow2 (-1074, false), -324);
    EXPECT_EQ (dconv::schubfach::floorLog10Pow2 (971, false), 292);
    EXPECT_EQ (dconv::schubfach::floorLog10Pow2 (0, true), -1);
    EXPECT_EQ (dconv::schubfach::floorLog10Pow2 (4, true), 1);
    EXPECT_EQ (dconv::schubfach::floorLog10Pow2 (-1074, true), -324);
    EXPECT_EQ (dconv::schubfach::floorLog10Pow2 (971, true), 292);
}

/**
 * @brief floorLog2Pow10 test.
 */
TEST (schubfach, floorLog2Pow10)
{
    EXPECT_EQ (dconv::schubfach::floorLog2Pow10 (0), 0);
    EXPECT_EQ (dconv::schubfach::floorLog2Pow10 (1), 3);
    EXPECT_EQ (dconv::schubfach::floorLog2Pow10 (2), 6);
    EXPECT_EQ (dconv::schubfach::floorLog2Pow10 (-1), -4);
    EXPECT_EQ (dconv::schubfach::floorLog2Pow10 (308), 1023);
    EXPECT_EQ (dconv::schubfach::floorLog2Pow10 (-292), -971);
    EXPECT_EQ (dconv::schubfach::floorLog2Pow10 (324), 1076);
}

/**
 * @brief roundToOdd test.
 */
TEST (schubfach, roundToOdd)
{
    const dconv::details::Power quarter = {static_cast<uint64_t> (1) << 62, 0};
    EXPECT_EQ (dconv::schubfach::roundToOdd (quarter, 8), 2u);
    EXPECT_EQ (dconv::schubfach::roundToOdd (quarter, 9), 3u);
    EXPECT_EQ (dconv::schubfach::roundToOdd (quarter, 12), 3u);

    const dconv::details::Power carry = {static_cast<uint64_t> (1) << 62, UINT64_MAX};
    EXPECT_EQ (dconv::schubfach::roundToOdd (carry, 4), 1u);
}

/**
 * @brief shortest test.
 */
TEST (schubfach, shortest)
{
    int k = 0;
    EXPECT_EQ (dconv::schubfach::shortest (1234.0, k), 1234u);
    EXPECT_EQ (k, 0);

    EXPECT_EQ (dconv::schubfach::shortest (0.3, k), 3000000000000000u);
    EXPECT_EQ (k, -16);

    EXPECT_EQ (dconv::schubfach::shortest (0.30000000000000004, k), 30000000000000004u);
    EXPECT_EQ (k, -17);

    EXPECT_EQ (dconv::schubfach::shortest (5e-324, k), 5u);
    EXPECT_EQ (k, -324);

    EXPECT_EQ (dconv::schubfach::shortest (2.2250738585072014e-308, k), 22250738585072014u);
    EXPECT_EQ (k, -324);

    EXPECT_EQ (dconv::schubfach::shortest (1.7976931348623157e308, k), 17976931348623157u);
    EXPECT_EQ (k, 292);
}

/**
 * @brief generate test.
 */
TEST (schubfach, generate)
{
    char buffer[32] = {};
    int length = 0, k = 0;

    dconv::schubfach::generate (buffer, length, k, 0.3);
    EXPECT_EQ (std::string (buffer, length), "3");
    EXPECT_EQ (k, -1);

    dconv::schubfach::generate (buffer, length, k, 1000.0);
    EXPECT_EQ (std::string (buffer, length), "1");
    EXPECT_EQ (k, 3);

    dconv::schubfach::generate (buffer, length, k, 1e23);
    EXPECT_EQ (std::string (buffer, length), "1");
    EXPECT_EQ (k, 23);
}

/**
 * @brief dtoa test.
 */
TEST (schubfach, dtoa)
{
    char beg[32] = {};
    char* end = nullptr;

    end = dconv::schubfach::dtoa (beg, 0.0);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "0.0");

    end = dconv::schubfach::dtoa (beg, -0.0);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "-0.0");

    end = dconv::schubfach::dtoa (beg, 0.1);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "0.1");

    end = dconv::schubfach::dtoa (beg, -1.5);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "-1.5");

    end = dconv::schubfach::dtoa (beg, 100.0);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "100.0");

    end = dconv::schubfach::dtoa (beg, 1e21);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "1e21");

    end = dconv::schubfach::dtoa (beg, 1e22);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "1e22");

    end = dconv::schubfach::dtoa (beg, 1e23);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "1e23");

    end = dconv::schubfach::dtoa (beg, 123e18);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "123000000000000000000.0");

    end = dconv::schubfach::dtoa (beg, 0.000123);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "0.000123");

    end = dconv::schubfach::dtoa (beg, 0.00001);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "0.00001");

    end = dconv::schubfach::dtoa (beg, 1e-7);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "1e-7");

    end = dconv::schubfach::dtoa (beg, 7e-10);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "7e-10");

    end = dconv::schubfach::dtoa (beg, 5e-324);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "5e-324");

    end = dconv::schubfach::dtoa (beg, 4.9406564584124654e-320);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "4.9407e-320");

    end = dconv::schubfach::dtoa (beg, 2.225073858507201e-308);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "2.225073858507201e-308");

    end = dconv::schubfach::dtoa (beg, 1.7976931348623157e308);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "1.7976931348623157e308");

    end = dconv::schubfach::dtoa (beg, 9007199254740992.0);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "9007199254740992.0");
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
