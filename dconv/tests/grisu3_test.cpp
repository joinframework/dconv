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
#include <dconv/grisu3.hpp>

// libraries.
#include <gtest/gtest.h>

// C++.
#include <string>

/**
 * @brief roundWeed test.
 */
TEST (grisu3, roundWeed)
{
    char buffer[] = "13";
    EXPECT_TRUE (dconv::grisu3::roundWeed (buffer, 2, 30, 100, 10, 10, 1));
    EXPECT_STREQ (buffer, "11");

    char ambiguous[] = "13";
    EXPECT_FALSE (dconv::grisu3::roundWeed (ambiguous, 2, 35, 100, 10, 10, 1));

    char unsafe[] = "13";
    EXPECT_FALSE (dconv::grisu3::roundWeed (unsafe, 2, 10, 15, 9, 10, 5));
    EXPECT_STREQ (unsafe, "13");
}

/**
 * @brief digitsGen test.
 */
TEST (grisu3, digitsGen)
{
    dconv::DiyFp w, minus, plus;
    int k = 0, length = 0;
    char buffer[32] = {};

    dconv::grisu2::scaledBoundaries (0.3, w, minus, plus, k);
    EXPECT_TRUE (dconv::grisu3::digitsGen (minus, w, plus, buffer, length, k));
    EXPECT_EQ (std::string (buffer, length), "3");
    EXPECT_EQ (k, -1);

    dconv::grisu2::scaledBoundaries (9007199254740992.0, w, minus, plus, k);
    EXPECT_TRUE (dconv::grisu3::digitsGen (minus, w, plus, buffer, length, k));
    EXPECT_EQ (std::string (buffer, length), "9007199254740992");
    EXPECT_EQ (k, 0);

    dconv::grisu2::scaledBoundaries (1e23, w, minus, plus, k);
    EXPECT_FALSE (dconv::grisu3::digitsGen (minus, w, plus, buffer, length, k));
}

/**
 * @brief fallback test.
 */
TEST (grisu3, fallback)
{
    char buffer[32] = {};
    int length = 0, k = 0;

    dconv::grisu3::fallback (buffer, length, k, 1e23);
    EXPECT_EQ (std::string (buffer, length), "1");
    EXPECT_EQ (k, 23);

    dconv::grisu3::fallback (buffer, length, k, 0.30000000000000004);
    EXPECT_EQ (std::string (buffer, length), "30000000000000004");
    EXPECT_EQ (k, -17);

    dconv::grisu3::fallback (buffer, length, k, 5e-324);
    EXPECT_EQ (std::string (buffer, length), "5");
    EXPECT_EQ (k, -324);

    dconv::grisu3::fallback (buffer, length, k, 1.7976931348623157e308);
    EXPECT_EQ (std::string (buffer, length), "17976931348623157");
    EXPECT_EQ (k, 292);
}

/**
 * @brief generate test.
 */
TEST (grisu3, generate)
{
    char buffer[32] = {};
    int length = 0, k = 0;

    dconv::grisu3::generate (buffer, length, k, 0.3);
    EXPECT_EQ (std::string (buffer, length), "3");
    EXPECT_EQ (k, -1);

    dconv::grisu3::generate (buffer, length, k, 1e23);
    EXPECT_EQ (std::string (buffer, length), "1");
    EXPECT_EQ (k, 23);
}

/**
 * @brief dtoa test.
 */
TEST (grisu3, dtoa)
{
    char beg[32] = {};
    char* end = nullptr;

    end = dconv::grisu3::dtoa (beg, 0.0);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "0.0");

    end = dconv::grisu3::dtoa (beg, -0.0);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "-0.0");

    end = dconv::grisu3::dtoa (beg, 0.1);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "0.1");

    end = dconv::grisu3::dtoa (beg, -1.5);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "-1.5");

    end = dconv::grisu3::dtoa (beg, 1e21);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "1e21");

    end = dconv::grisu3::dtoa (beg, 1e23);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "1e23");

    end = dconv::grisu3::dtoa (beg, 0.000123);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "0.000123");

    end = dconv::grisu3::dtoa (beg, 5e-324);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "5e-324");

    end = dconv::grisu3::dtoa (beg, 4.9406564584124654e-320);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "4.9407e-320");

    end = dconv::grisu3::dtoa (beg, 1.7976931348623157e308);
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
