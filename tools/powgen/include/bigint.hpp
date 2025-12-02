/**
 * MIT License
 *
 * Copyright (c) 2025 Mathieu Rabine
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

#ifndef __POWGEN_BIGINT_HPP__
#define __POWGEN_BIGINT_HPP__

#include <vector>

#include <cstdint>
#include <cstddef>

struct Power
{
    uint64_t hi;
    uint64_t lo;
};

class BigInt
{
public:
    explicit BigInt (uint64_t value = 0)
    {
        if (value == 0)
        {
            data.push_back (0);
        }
        else
        {
            data.push_back (static_cast <uint32_t> (value));
            uint32_t high = static_cast <uint32_t> (value >> 32);
            if (high != 0)
            {
                data.push_back (high);
            }
        }
    }

    void multiply (uint32_t factor)
    {
        uint64_t carry = 0;
        for (size_t i = 0; i < data.size (); ++i)
        {
            uint64_t product = static_cast <uint64_t> (data[i]) * factor + carry;
            data[i] = static_cast <uint32_t> (product);
            carry = product >> 32;
        }
        if (carry != 0)
        {
            data.push_back (static_cast <uint32_t> (carry));
        }
    }

    void divide (uint32_t divisor)
    {
        uint64_t remainder = 0;
        for (int i = static_cast <int> (data.size ()) - 1; i >= 0; --i)
        {
            uint64_t current = (remainder << 32) | data[i];
            data[i] = static_cast <uint32_t> (current / divisor);
            remainder = current % divisor;
        }
        removeLeadingZeros ();
    }

    void shiftLeft (int shift)
    {
        if (shift == 0) return;

        int blockShift = shift / 32;
        int bitShift = shift % 32;

        if (blockShift > 0)
        {
            data.insert (data.begin (), blockShift, 0);
        }

        if (bitShift > 0)
        {
            uint32_t carry = 0;
            for (size_t i = blockShift; i < data.size (); ++i)
            {
                uint64_t temp = (static_cast <uint64_t> (data[i]) << bitShift) | carry;
                data[i] = static_cast <uint32_t> (temp);
                carry = static_cast <uint32_t> (temp >> 32);
            }
            if (carry != 0)
            {
                data.push_back (carry);
            }
        }
    }

    static BigInt powerOfTwo (int shift)
    {
        BigInt result;
        result.data.clear ();
        
        int chunks = shift / 32;
        int bit = shift % 32;
        
        result.data.resize (chunks, 0);
        result.data.push_back (1U << bit);
        
        return result;
    }

    Power getTop128 () const
    {
        if (data.empty () || (data.size () == 1 && data[0] == 0))
        {
            return {0, 0};
        }

        int msbBlock = static_cast <int> (data.size ()) - 1;
        uint32_t msbValue = data[msbBlock];

        int msbBit = 31;
        while (msbBit > 0 && ((msbValue >> msbBit) & 1) == 0)
        {
            --msbBit;
        }

        int startBit = msbBlock * 32 + msbBit;

        uint64_t hi = 0;
        uint64_t lo = 0;

        for (int i = 0; i < 64; ++i)
        {
            int bitPos = startBit - i;
            if (bitPos >= 0)
            {
                int block = bitPos / 32;
                int bit = bitPos % 32;
                if (block < static_cast <int> (data.size ()))
                {
                    uint64_t bitValue = (data[block] >> bit) & 1;
                    hi |= (bitValue << (63 - i));
                }
            }
        }

        for (int i = 0; i < 64; ++i)
        {
            int bitPos = startBit - 64 - i;
            if (bitPos >= 0)
            {
                int block = bitPos / 32;
                int bit = bitPos % 32;
                if (block < static_cast <int> (data.size ()))
                {
                    uint64_t bitValue = (data[block] >> bit) & 1;
                    lo |= (bitValue << (63 - i));
                }
            }
        }

        return {hi, lo};
    }

private:
    void removeLeadingZeros ()
    {
        while (data.size () > 1 && data.back () == 0)
        {
            data.pop_back ();
        }
    }

    std::vector <uint32_t> data;
};

#endif
