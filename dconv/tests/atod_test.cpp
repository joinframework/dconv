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

// libraries.
#include <gtest/gtest.h>

// C.
#include <cmath>

std::string asciiHextoBinary (const std::string & input)
{
    std::stringstream stream(input);
    std::string output, a;
    while (stream >> a[0] >> a[1]) {
        output += (char)std::stol(a, nullptr, 16);
    }
    return output;
}

/**
 * @brief pow10 test.
 */
TEST (atod, pow10)
{
    std::string foo = asciiHextoBinary("FFFFFFFFF");
    std::cout << foo << std::endl;

    EXPECT_EQ (dconv::details::pow10 (0), 1e0);
    EXPECT_EQ (dconv::details::pow10 (22), 1e22);
    EXPECT_EQ (dconv::details::pow10 (124), 1e124);
    EXPECT_EQ (dconv::details::pow10 (218), 1e218);
    EXPECT_EQ (dconv::details::pow10 (308), 1e308);
    EXPECT_THROW (dconv::details::pow10 (309), std::invalid_argument);
    EXPECT_THROW (dconv::details::pow10 (412), std::invalid_argument);
}

/**
 * @brief isSign test.
 */
TEST (atod, isSign)
{
    EXPECT_TRUE  (dconv::details::isSign ('-'));
    EXPECT_TRUE  (dconv::details::isSign ('+'));
    EXPECT_FALSE (dconv::details::isSign ('0'));
    EXPECT_FALSE (dconv::details::isSign ('9'));
    EXPECT_FALSE (dconv::details::isSign ('a'));
    EXPECT_FALSE (dconv::details::isSign ('z'));
}

/**
 * @brief isDigit test.
 */
TEST (atod, isDigit)
{
    EXPECT_FALSE (dconv::details::isDigit ('-'));
    EXPECT_FALSE (dconv::details::isDigit ('+'));
    EXPECT_TRUE  (dconv::details::isDigit ('0'));
    EXPECT_TRUE  (dconv::details::isDigit ('9'));
    EXPECT_FALSE (dconv::details::isDigit ('a'));
    EXPECT_FALSE (dconv::details::isDigit ('z'));
}

/**
 * @brief atod test.
 */
TEST (atod, atod)
{
    double value = 0.0;

    EXPECT_EQ (dconv::atod ("003", value), nullptr);

    EXPECT_EQ (dconv::atod ("Infinaty", value), nullptr);

    EXPECT_EQ (dconv::atod ("-Infinaty", value), nullptr);

    EXPECT_EQ (dconv::atod ("not valid", value), nullptr);

    EXPECT_EQ (dconv::atod ("1ee+10", value), nullptr);

    ASSERT_NE (dconv::atod ("Inf", value), nullptr);
    EXPECT_TRUE (!std::signbit (value) && std::isinf (value));

    ASSERT_NE (dconv::atod ("-Inf", value), nullptr);
    EXPECT_TRUE (std::signbit (value) && std::isinf (value));

    ASSERT_NE (dconv::atod ("NaN", value), nullptr);
    EXPECT_TRUE (!std::signbit (value) && std::isnan (value));

    ASSERT_NE (dconv::atod ("-NaN", value), nullptr);
    EXPECT_TRUE (std::signbit (value) && std::isnan (value));

    ASSERT_NE (dconv::atod ("0.0", value), nullptr);
    EXPECT_EQ (value, 0.0);

    ASSERT_NE (dconv::atod ("-0.0", value), nullptr);
    EXPECT_EQ (value, -0.0);

    ASSERT_NE (dconv::atod ("1.0", value), nullptr);
    EXPECT_EQ (value, 1.0);

    ASSERT_NE (dconv::atod ("-1.0", value), nullptr);
    EXPECT_EQ (value, -1.0);

    ASSERT_NE (dconv::atod ("1.5", value), nullptr);
    EXPECT_EQ (value, 1.5);

    ASSERT_NE (dconv::atod ("-1.5", value), nullptr);
    EXPECT_EQ (value, -1.5);

    ASSERT_NE (dconv::atod ("3.1416", value), nullptr);
    EXPECT_EQ (value, 3.1416);

    ASSERT_NE (dconv::atod ("1E10", value), nullptr);
    EXPECT_EQ (value, 1E10);

    ASSERT_NE (dconv::atod ("1e10", value), nullptr);
    EXPECT_EQ (value, 1e10);

    ASSERT_NE (dconv::atod ("1E+10", value), nullptr);
    EXPECT_EQ (value, 1E+10);

    ASSERT_NE (dconv::atod ("1E-10", value), nullptr);
    EXPECT_EQ (value, 1E-10);

    ASSERT_NE (dconv::atod ("-1E10", value), nullptr);
    EXPECT_EQ (value, -1E10);

    ASSERT_NE (dconv::atod ("-1e10", value), nullptr);
    EXPECT_EQ (value, -1e10);

    ASSERT_NE (dconv::atod ("-1E+10", value), nullptr);
    EXPECT_EQ (value, -1E+10);

    ASSERT_NE (dconv::atod ("-1E-10", value), nullptr);
    EXPECT_EQ (value, -1E-10);

    ASSERT_NE (dconv::atod ("1.234E+10", value), nullptr);
    EXPECT_EQ (value, 1.234E+10);

    ASSERT_NE (dconv::atod ("1.234E-10", value), nullptr);
    EXPECT_EQ (value, 1.234E-10);

    ASSERT_NE (dconv::atod ("1.79769e+308", value), nullptr);
    EXPECT_EQ (value, 1.79769e+308);

    ASSERT_NE (dconv::atod ("2.22507e-308", value), nullptr);
    EXPECT_EQ (value, 2.22507e-308);

    ASSERT_NE (dconv::atod ("-1.79769e+308", value), nullptr);
    EXPECT_EQ (value, -1.79769e+308);

    ASSERT_NE (dconv::atod ("-2.22507e-308", value), nullptr);
    EXPECT_EQ (value, -2.22507e-308);

    ASSERT_NE (dconv::atod ("-4.9406564584124654e-324", value), nullptr);
    EXPECT_EQ (value, -4.9406564584124654e-324);

    ASSERT_NE (dconv::atod ("2.2250738585072009e-308", value), nullptr);
    EXPECT_EQ (value, 2.2250738585072009e-308);

    ASSERT_NE (dconv::atod ("2.2250738585072014e-308", value), nullptr);
    EXPECT_EQ (value, 2.2250738585072014e-308);

    ASSERT_NE (dconv::atod ("1.7976931348623157e+308", value), nullptr);
    EXPECT_EQ (value, 1.7976931348623157e+308);

    ASSERT_NE (dconv::atod ("1e-10000", value), nullptr);
    EXPECT_EQ (value, 0.0);

    ASSERT_NE (dconv::atod ("18446744073709551616", value), nullptr);
    EXPECT_EQ (value, 18446744073709551616.0);

    ASSERT_NE (dconv::atod ("-9223372036854775809", value), nullptr);
    EXPECT_EQ (value, -9223372036854775809.0);

    //ASSERT_NE (dconv::atod ("0.9868011474609375", value), nullptr);
    //EXPECT_EQ (value, 0.9868011474609375);

    ASSERT_NE (dconv::atod ("123e34", value), nullptr);
    EXPECT_EQ (value, 123e34);

    ASSERT_NE (dconv::atod ("45913141877270640000.0", value), nullptr);
    EXPECT_EQ (value, 45913141877270640000.0);

    ASSERT_NE (dconv::atod ("2.2250738585072011e-308", value), nullptr);
    EXPECT_EQ (value, 2.2250738585072011e-308);

    ASSERT_NE (dconv::atod ("1e-214748363", value), nullptr);
    EXPECT_EQ (value, 0.0);

    ASSERT_NE (dconv::atod ("1e-214748364", value), nullptr);
    EXPECT_EQ (value, 0.0);

    ASSERT_NE (dconv::atod ("0.017976931348623157e+310", value), nullptr);
    EXPECT_EQ (value, 1.7976931348623157e+308);

    ASSERT_NE (dconv::atod ("2.2250738585072012e-308", value), nullptr);
    EXPECT_EQ (value, 2.2250738585072014e-308);

    ASSERT_NE (dconv::atod ("0.999999999999999944488848768742172978818416595458984375", value), nullptr);
    EXPECT_EQ (value, 1.0);

    ASSERT_NE (dconv::atod ("0.999999999999999944488848768742172978818416595458984374", value), nullptr);
    EXPECT_EQ (value, 0.99999999999999989);

    ASSERT_NE (dconv::atod ("0.999999999999999944488848768742172978818416595458984376", value), nullptr);
    EXPECT_EQ (value, 1.0);

    ASSERT_NE (dconv::atod ("1.00000000000000011102230246251565404236316680908203125", value), nullptr);
    EXPECT_EQ (value, 1.0);

    ASSERT_NE (dconv::atod ("1.00000000000000011102230246251565404236316680908203124", value), nullptr);
    EXPECT_EQ (value, 1.0);

    ASSERT_NE (dconv::atod ("1.00000000000000011102230246251565404236316680908203126", value), nullptr);
    EXPECT_EQ (value, 1.00000000000000022);

    ASSERT_NE (dconv::atod ("72057594037927928.0", value), nullptr);
    EXPECT_EQ (value, 72057594037927928.0);

    ASSERT_NE (dconv::atod ("72057594037927936.0", value), nullptr);
    EXPECT_EQ (value, 72057594037927936.0);

    ASSERT_NE (dconv::atod ("72057594037927932.0", value), nullptr);
    EXPECT_EQ (value, 72057594037927936.0);

    ASSERT_NE (dconv::atod ("7205759403792793199999e-5", value), nullptr);
    EXPECT_EQ (value, 72057594037927928.0);

    ASSERT_NE (dconv::atod ("7205759403792793200001e-5", value), nullptr);
    EXPECT_EQ (value, 72057594037927936.0);

    ASSERT_NE (dconv::atod ("9223372036854774784.0", value), nullptr);
    EXPECT_EQ (value, 9223372036854774784.0);

    ASSERT_NE (dconv::atod ("9223372036854775808.0", value), nullptr);
    EXPECT_EQ (value, 9223372036854775808.0);

    ASSERT_NE (dconv::atod ("9223372036854775296.0", value), nullptr);
    EXPECT_EQ (value, 9223372036854775808.0);

    ASSERT_NE (dconv::atod ("922337203685477529599999e-5", value), nullptr);
    EXPECT_EQ (value, 9223372036854774784.0);

    ASSERT_NE (dconv::atod ("922337203685477529600001e-5", value), nullptr);
    EXPECT_EQ (value, 9223372036854775808.0);

    ASSERT_NE (dconv::atod ("10141204801825834086073718800384", value), nullptr);
    EXPECT_EQ (value, 10141204801825834086073718800384.0);

    ASSERT_NE (dconv::atod ("10141204801825835211973625643008", value), nullptr);
    EXPECT_EQ (value, 10141204801825835211973625643008.0);

    ASSERT_NE (dconv::atod ("10141204801825834649023672221696", value), nullptr);
    EXPECT_EQ (value, 10141204801825835211973625643008.0);

    ASSERT_NE (dconv::atod ("1014120480182583464902367222169599999e-5", value), nullptr);
    EXPECT_EQ (value, 10141204801825834086073718800384.0);

    ASSERT_NE (dconv::atod ("1014120480182583464902367222169600001e-5", value), nullptr);
    EXPECT_EQ (value, 10141204801825835211973625643008.0);

    ASSERT_NE (dconv::atod ("5708990770823838890407843763683279797179383808", value), nullptr);
    EXPECT_EQ (value, 5708990770823838890407843763683279797179383808.0);

    ASSERT_NE (dconv::atod ("5708990770823839524233143877797980545530986496", value), nullptr);
    EXPECT_EQ (value, 5708990770823839524233143877797980545530986496.0);

    ASSERT_NE (dconv::atod ("5708990770823839207320493820740630171355185152", value), nullptr);
    EXPECT_EQ (value, 5708990770823839524233143877797980545530986496.0);

    ASSERT_NE (dconv::atod ("5708990770823839207320493820740630171355185151999e-3", value), nullptr);
    EXPECT_EQ (value, 5708990770823838890407843763683279797179383808.0);

    ASSERT_NE (dconv::atod ("5708990770823839207320493820740630171355185152001e-3", value), nullptr);
    EXPECT_EQ (value, 5708990770823839524233143877797980545530986496.0);

    ASSERT_NE (dconv::atod ("1000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
                            "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
                            "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
                            "000000000", value), nullptr);
    EXPECT_EQ (value, 1E308);

    ASSERT_NE (dconv::atod ("2.225073858507201136057409796709131975934819546351645648023426109724822222021076945516529523908135087"
                            "91414915891303962110687008643869459464552765720740782062174337998814106326732925355228688137214901298"
                            "11224514518898490572223072852551331557550159143974763979834118019993239625482890171070818506906306666"
                            "55994938275772572015763062690663332647565300009245888316433037779791869612049497390377829704905051080"
                            "60994073026293712895895000358379996720725430436028407889577179615094551674824347103070260914462157228"
                            "98802581825451803257070188608721131280795122334262883686223215037756666225039825343359745688844239002"
                            "65498198385487948292206894721689831099698365846814022854243330660339850886445804001034933970427567186"
                            "443383770486037861622771738545623065874679014086723327636718751234567890123456789012345678901e-308", value), nullptr);
    EXPECT_EQ (value, 2.2250738585072014e-308);
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
