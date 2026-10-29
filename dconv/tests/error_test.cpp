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
#include <dconv/error.hpp>

// libraries.
#include <gtest/gtest.h>

/**
 * @brief name test.
 */
TEST (ErrorCategory, name)
{
    EXPECT_STREQ (dconv::errorCategory ().name (), "dconv");
}

/**
 * @brief message test.
 */
TEST (ErrorCategory, message)
{
    EXPECT_EQ (dconv::errorCategory ().message (0), "success");
    EXPECT_EQ (dconv::errorCategory ().message (static_cast<int> (dconv::Errc::InvalidNumber)), "number is invalid");
    EXPECT_EQ (dconv::errorCategory ().message (static_cast<int> (dconv::Errc::NotDouble)), "number is an integer");
}

/**
 * @brief make_error_code test.
 */
TEST (ErrorCategory, make_error_code)
{
    std::error_code ec = dconv::make_error_code (dconv::Errc::NotDouble);
    EXPECT_EQ (ec.value (), static_cast<int> (dconv::Errc::NotDouble));
    EXPECT_EQ (&ec.category (), &dconv::errorCategory ());
    EXPECT_TRUE (ec == dconv::Errc::NotDouble);
    EXPECT_FALSE (ec == dconv::Errc::InvalidNumber);
}

/**
 * @brief make_error_condition test.
 */
TEST (ErrorCategory, make_error_condition)
{
    std::error_condition condition = dconv::make_error_condition (dconv::Errc::InvalidNumber);
    EXPECT_EQ (condition.value (), static_cast<int> (dconv::Errc::InvalidNumber));
    EXPECT_EQ (&condition.category (), &dconv::errorCategory ());
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
