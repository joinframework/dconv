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

#ifndef __DCONV_ERROR_HPP__
#define __DCONV_ERROR_HPP__

// C++.
#include <system_error>
#include <string>

namespace dconv
{
    /**
     * @brief conversion error codes.
     */
    enum class Errc
    {
        InvalidNumber = 1, /**< Number is invalid. */
        NotDouble,         /**< Number is an integer. */
    };

    /**
     * @brief conversion error category.
     */
    class ErrorCategory : public std::error_category
    {
    public:
        /**
         * @brief get conversion error category name.
         * @return conversion error category name.
         */
        const char* name () const noexcept override
        {
            return "dconv";
        }

        /**
         * @brief translate conversion error code to human readable error string.
         * @param code error code.
         * @return human readable error string.
         */
        std::string message (int code) const override
        {
            switch (static_cast<Errc> (code))
            {
                case Errc::InvalidNumber:
                    return "number is invalid";
                case Errc::NotDouble:
                    return "number is an integer";
                default:
                    return "success";
            }
        }
    };

    /// conversion error category instance.
    inline const ErrorCategory errorCategoryInstance;

    /**
     * @brief get error category.
     * @return the created std::error_category object.
     */
    inline const std::error_category& errorCategory () noexcept
    {
        return errorCategoryInstance;
    }

    /**
     * @brief create an std::error_code object.
     * @param code error code number.
     * @return the created std::error_code object.
     */
    inline std::error_code make_error_code (Errc code) noexcept
    {
        return std::error_code (static_cast<int> (code), errorCategory ());
    }

    /**
     * @brief create an std::error_condition object.
     * @param code error code number.
     * @return the created std::error_condition object.
     */
    inline std::error_condition make_error_condition (Errc code) noexcept
    {
        return std::error_condition (static_cast<int> (code), errorCategory ());
    }
}

namespace std
{
    /// conversion error code specialization.
    template <>
    struct is_error_code_enum<dconv::Errc> : public true_type
    {
    };
}

#endif
