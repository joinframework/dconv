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
#include <dconv/atod.hpp>
#include <dconv/dtoa.hpp>

// libraries.
#include <gtest/gtest.h>

/**
 * @brief round trip test.
 */
TEST (dconv, roundtrip)
{
    char beg[25] = {};
    double value = 0.0;

    ASSERT_NE (dconv::atod ("0.0", value), nullptr);
    char *end = dconv::dtoa (beg, value);
    ASSERT_NE (end, nullptr);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "0.0");

    ASSERT_NE (dconv::atod ("-0.0", value), nullptr);
    end = dconv::dtoa (beg, value);
    ASSERT_NE (end, nullptr);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "-0.0");

    ASSERT_NE (dconv::atod ("1.2345", value), nullptr);
    end = dconv::dtoa (beg, value);
    ASSERT_NE (end, nullptr);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "1.2345");

    ASSERT_NE (dconv::atod ("-1.2345", value), nullptr);
    end = dconv::dtoa (beg, value);
    ASSERT_NE (end, nullptr);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "-1.2345");

    ASSERT_NE (dconv::atod ("5e-324", value), nullptr);
    end = dconv::dtoa (beg, value);
    ASSERT_NE (end, nullptr);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "5e-324");

    ASSERT_NE (dconv::atod ("2.225073858507201e-308", value), nullptr);
    end = dconv::dtoa (beg, value);
    ASSERT_NE (end, nullptr);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "2.225073858507201e-308");

    ASSERT_NE (dconv::atod ("2.2250738585072014e-308", value), nullptr);
    end = dconv::dtoa (beg, value);
    ASSERT_NE (end, nullptr);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "2.2250738585072014e-308");

    ASSERT_NE (dconv::atod ("1.7976931348623157e308", value), nullptr);
    end = dconv::dtoa (beg, value);
    ASSERT_NE (end, nullptr);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "1.7976931348623157e308");
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
