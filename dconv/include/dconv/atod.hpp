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

#ifndef __DCONV_ATOD_HPP__
#define __DCONV_ATOD_HPP__

// dconv.
#include <dconv/view.hpp>

// C++.
#include <limits>
#include <memory>

// C.
#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <cmath>

namespace dconv
{
    namespace details
    {
        struct LocaleDelete
        {
            constexpr LocaleDelete () noexcept = default;

            void operator () (locale_t loc)
            {
                freelocale (loc);
            }
        };

        using LocalePtr = std::unique_ptr <std::remove_pointer_t <locale_t>, LocaleDelete>;

        inline const char * strtodSlow (const char * beg, double& value)
        {
            static LocalePtr locale (newlocale (LC_ALL_MASK, "C", nullptr));
            char* end = nullptr;
            value = strtod_l (beg, &end, locale.get ());
            return end;
        }

        struct CachedPower
        {
            uint64_t mul;
            int16_t pow2;
        };

        constexpr CachedPower pow10[80] = {
            {0xff77b1fcbebcdc4f, -1196}, {0x8dd01fad907ffc3b, -1131},
            {0xd3515c2831559a83, -1067}, {0x9d71ac8fada6c9b5, -1004},
            {0xea9c227723ee8bcb,  -940}, {0xaecc49914078536d,  -877},
            {0x823c12795db6ce57,  -814}, {0xc21094364dfb5636,  -751},
            {0x9096ea6f3848984f,  -688}, {0xe096f02c5c4b3e04,  -625},
            {0xab70fe17c79ac6ca,  -562}, {0xfee50b7025c36a08,  -499},
            {0xbd8430bd08277231,  -436}, {0x8bab8eefb6409c1a,  -373},
            {0xd89d64d57a607744,  -310}, {0x9f4f2726179a2245,  -248},
            {0xed63a231d4c4fb27,  -185}, {0xb0de65388cc8ada8,  -122},
            {0x83c7088e1aab65db,   -60}, {0xc45d1df942711d9a,     3},
            {0x924d692ca61be758,    66}, {0xe12e13424bb40e13,   128},
            {0xac5d37d5b79b6239,   191}, {0xfd87b5f28300ca0d,   253},
            {0xbce5086492111aea,   316}, {0x8cbccc096f5088cb,   379},
            {0xd1b71758e219652b,   441}, {0x9c40000000000000,   504},
            {0xe8d4a51000000000,   566}, {0xad78ebc5ac620000,   629},
            {0x813f3978f8940984,   692}, {0xc097ce7bc90715b3,   754},
            {0x8f7e32ce7bea5c6f,   817}, {0xd5d238a4abe98068,   879},
            {0x9becce62836ac577,   942}, {0xe858ad248f5c22c9,  1004},
            {0xaf298d050e4395d6,  1067}, {0x82818f1281ed449f,  1130},
            {0xc2781f49ffcfa6d5,  1192}, {0x8e679c2f5e44ff8f,  1255},
            {0xd433179d9c8cb841,  1317}, {0x9abe14cd44753b52,  1380},
            {0xe596b7b0c643c719,  1442}, {0xaf87023b9bf0ee6a,  1505},
            {0x8000000000000000,  1568}, {0xc000000000000000,  1630},
            {0x8e1bc9bf04000000,  1693}, {0xd3c21bcecceda100,  1755},
            {0x9a130b963a6c115c,  1818}, {0xe592d025b79fc475,  1880},
            {0xae75f2a2b9cd4f7a,  1943}, {0x823c12795db6ce57,  2006},
            {0xbff8f10e7a8921a4,  2068}, {0x8dd01fad907ffc3b,  2131},
            {0xd5605fcdcf32e1d6,  2193}, {0x9b10a4e5e9913128,  2256},
            {0xe7109bfba19c0c9d,  2318}, {0xac2820d9623bf429,  2381},
            {0xfea126b7d78186bc,  2443}, {0xba756174393d88df,  2506},
            {0x8a08f0f8bf0f156b,  2569}, {0xd226fc195c6a2f8c,  2631},
            {0x97c560ba6b0919a5,  2694}, {0xe2250f2b6b2c1e8e,  2756},
            {0xa8d9d1535ce3b396,  2819}, {0xfb9b7cd9a4a7443c,  2881},
            {0xb8157268fdae9e4c,  2944}, {0x8858ce5d8a3a7cf0,  3007},
            {0xcdb02555653131b6,  3069}, {0x952e5d8e1e4e8a81,  3132},
            {0xdd5e8b9afacbe17d,  3194}, {0xa1258379a94d028d,  3257},
            {0xf08045f99aded67b,  3319}, {0xb3f4e093db73a093,  3382},
            {0x87625f056c7c4a8b,  3445}, {0xc90715b34b9f1000,  3507},
            {0x92efd1b8d0cf37be,  3570}, {0xd77485cb25823ac7,  3632},
            {0x9eb9faad07929684,  3695}, {0xe9e7cea3f88b0c64,  3757},
        };

        static constexpr uint64_t pow10Small[8] = {
            1ULL, 10ULL, 100ULL, 1000ULL, 10000ULL, 100000ULL, 1000000ULL, 10000000ULL
        };

        inline void umul128 (uint64_t a, uint64_t b, uint64_t& hi, uint64_t& lo) noexcept
        {
        #if defined(__SIZEOF_INT128__)
            __uint128_t product = static_cast <__uint128_t> (a) * b;
            hi = static_cast <uint64_t> (product >> 64);
            lo = static_cast <uint64_t> (product);
        #else
            uint64_t a_lo = static_cast <uint32_t> (a);
            uint64_t a_hi = a >> 32;
            uint64_t b_lo = static_cast <uint32_t> (b);
            uint64_t b_hi = b >> 32;
            
            uint64_t p0 = a_lo * b_lo;
            uint64_t p1 = a_lo * b_hi;
            uint64_t p2 = a_hi * b_lo;
            uint64_t p3 = a_hi * b_hi;
            
            uint64_t middle = p1 + (p0 >> 32) + static_cast <uint32_t> (p2);
            
            lo = (middle << 32) | static_cast <uint32_t> (p0);
            hi = p3 + (middle >> 32) + (p2 >> 32);
        #endif
        }

        inline bool strtodFast (bool negative, uint64_t mantissa, int64_t exponent, double& value)
        {
            if (unlikely (mantissa == 0))
            {
                value = negative ? -0.0 : 0.0;
                return true;
            }

            if (unlikely (exponent < -325 || exponent > 308))
            {
                return false;
            }

            int32_t q = static_cast <int32_t> (exponent) + 325;
            int32_t k = q / 8;
            int32_t r = q & 7;

            if (unlikely (k >= 80))
            {
                return false;
            }

            if (r > 0)
            {
                mantissa *= pow10Small[r];
            }

            int clz = __builtin_clzll (mantissa);
            if (unlikely (clz < 1))
            {
                return false;
            }

            mantissa <<= clz;

            const CachedPower& cp = pow10[k];
            uint64_t hi, lo;
            umul128 (mantissa, cp.mul, hi, lo);

            int32_t binExp = cp.pow2 + 64 - clz - 1;

            if ((hi & 0x8000000000000000ULL) == 0)
            {
                hi = (hi << 1) | (lo >> 63);
                lo = lo << 1;
                binExp--;
            }

            int32_t ieeeExp = binExp + 1023;

            if (ieeeExp < 1 || ieeeExp > 2046)
            {
                return false;
            }

            uint64_t mantissaBits = hi >> 11;

            // uint64_t lowerBits = (hi & 0x7FFULL) | (lo != 0 ? 1ULL : 0ULL);
            // if (lowerBits == 0x400ULL || lowerBits == 0x3FFULL)
            // {
            //     return false;
            // }

            uint64_t roundBit = (hi >> 10) & 1;
            uint64_t stickyBits = ((hi & 0x3FFULL) | lo) != 0 ? 1ULL : 0ULL;

            if (roundBit && (stickyBits || (mantissaBits & 1)))
            {
                mantissaBits++;

                if (mantissaBits > 0x1FFFFFFFFFFFFFULL)
                {
                    mantissaBits >>= 1;
                    ieeeExp++;

                    if (ieeeExp > 2046)
                    {
                        return false;
                    }
                }
            }

            mantissaBits &= 0xFFFFFFFFFFFFFULL;
            uint64_t bits = (static_cast <uint64_t> (ieeeExp) << 52) | mantissaBits;

            if (negative)
            {
                bits |= 0x8000000000000000ULL;
            }

            std::memcpy (&value, &bits, sizeof(double));
            return true;
        }

        inline bool isDigit (char c) noexcept
        {
            return static_cast <unsigned char> (c - '0') <= 9u;
        }

        inline bool isSign (char c) noexcept
        {
            return (c == '+') || (c == '-');
        }

        inline const char * atod (View& view, double& value)
        {
            auto beg = view.data ();

            uint64_t mantissa = 0;
            uint64_t digits = 0;
            bool neg = view.getIf ('-');

            if (view.getIf ('0'))
            {
                if (unlikely (isDigit (view.peek ())))
                {
                    return nullptr;
                }
            }
            else if (likely (isDigit (view.peek ())))
            {
                mantissa = view.get () - '0';
                ++digits;

                while (isDigit (view.peek ()))
                {
                    uint64_t next = (10 * mantissa) + (view.get () - '0');
                    if (unlikely (next < mantissa))
                    {
                        return strtodSlow (beg, value);
                    }
                    mantissa = next;
                    ++digits;
                }
            }
            else if (view.getIfNoCase ('i') && view.getIfNoCase ('n') && view.getIfNoCase ('f'))
            {
                if (view.getIfNoCase ('i'))
                {
                    if (!(view.getIfNoCase ('n') && view.getIfNoCase ('i') && 
                          view.getIfNoCase ('t') && view.getIfNoCase ('y')))
                    {
                        return nullptr;
                    }
                }

                value = neg ? -std::numeric_limits <double>::infinity () : std::numeric_limits <double>::infinity ();
                return view.data ();
            }
            else if (view.getIfNoCase ('n') && view.getIfNoCase ('a') && view.getIfNoCase ('n'))
            {
                value = neg ? -std::numeric_limits <double>::quiet_NaN () : std::numeric_limits <double>::quiet_NaN ();
                return view.data ();
            }
            else
            {
                return nullptr;
            }

            int64_t exponent = 0;

            if (view.getIf ('.'))
            {
                if (unlikely (!isDigit (view.peek ())))
                {
                    return nullptr;
                }

                mantissa = (10 * mantissa) + (view.get () - '0');
                if (mantissa || digits) ++digits;
                --exponent;

                while (isDigit (view.peek ()))
                {
                    uint64_t next = (10 * mantissa) + (view.get () - '0');
                    if (unlikely (next < mantissa))
                    {
                        return strtodSlow (beg, value);
                    }
                    mantissa = next;
                    if (mantissa || digits) ++digits;
                    --exponent;
                }
            }

            if (view.getIf ('e') || view.getIf ('E'))
            {
                bool negExp = false;

                if (isSign (view.peek ()))
                {
                    negExp = (view.get () == '-');
                }

                if (unlikely (!isDigit (view.peek ())))
                {
                    return nullptr;
                }

                int64_t exp = view.get () - '0';

                while (isDigit (view.peek ()))
                {
                    if (likely (exp < 1000))
                    {
                        exp = (10 * exp) + (view.get () - '0');
                    }
                    else
                    {
                        view.get ();
                    }
                }

                exponent += (negExp ? -exp : exp);
            }

            if (likely (digits <= 19))
            {
                if (strtodFast (neg, mantissa, exponent, value))
                {
                    return view.data ();
                }
            }

            return strtodSlow (beg, value);
        }
    }

    /**
     * @brief string to double conversion.
     * @param str string to parse.
     * @param value converted value.
     * @return end position on success, nullptr on failure.
     */
    inline const char* atod (const char* str, double& value)
    {
        View view (str);
        return details::atod (view, value);
    }

    /**
     * @brief string to double conversion.
     * @param str string to parse.
     * @param length string length.
     * @param value converted value.
     * @return end position on success, nullptr on failure.
     */
    inline const char* atod (const char* str, size_t length, double& value)
    {
        View view (str, length);
        return details::atod (view, value);
    }

    /**
     * @brief string to double conversion.
     * @param first string first position.
     * @param last string last position.
     * @param value converted value.
     * @return end position on success, nullptr on failure.
     */
    inline const char* atod (const char* first, const char* last, double& value)
    {
        View view (first, last);
        return details::atod (view, value);
    }
}

#endif
