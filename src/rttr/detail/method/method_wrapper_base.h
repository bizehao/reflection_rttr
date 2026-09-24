/************************************************************************************
*                                                                                   *
*   Copyright (c) 2014 - 2018 Axel Menzel <info@rttr.org>                           *
*                                                                                   *
*   This file is part of RTTR (Run Time Type Reflection)                            *
*   License: MIT License                                                            *
*                                                                                   *
*   Permission is hereby granted, free of charge, to any person obtaining           *
*   a copy of this software and associated documentation files (the "Software"),    *
*   to deal in the Software without restriction, including without limitation       *
*   the rights to use, copy, modify, merge, publish, distribute, sublicense,        *
*   and/or sell copies of the Software, and to permit persons to whom the           *
*   Software is furnished to do so, subject to the following conditions:            *
*                                                                                   *
*   The above copyright notice and this permission notice shall be included in      *
*   all copies or substantial portions of the Software.                             *
*                                                                                   *
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR      *
*   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,        *
*   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE     *
*   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER          *
*   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,   *
*   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE   *
*   SOFTWARE.                                                                       *
*                                                                                   *
*************************************************************************************/

#ifndef RTTR_METHOD_WRAPPER_BASE_H_
#define RTTR_METHOD_WRAPPER_BASE_H_

#include "rttr/detail/base/core_prerequisites.h"
#include "rttr/type.h"
#include "rttr/variant.h"
#include "rttr/parameter_info.h"

#include <string>
#include <vector>
#include <string_view>
#include <span>

namespace rttr {
    class type;
    class argument;
    class instance;
    class method;

    namespace detail {
        /*!
 * Abstract class for a method.
 *
 * This is the base class for all methods.
 * You can invoke the method via method_wrapper_base::invoke.
 */
        class RTTR_API method_wrapper_base {
        public:
            method_wrapper_base(std::string_view name, rttr::type declaring_type) RTTR_NOEXCEPT;

            virtual ~method_wrapper_base();

            std::string_view get_name() const RTTR_NOEXCEPT;

            rttr::type get_declaring_type() const RTTR_NOEXCEPT;

            virtual variant get_metadata(const std::string_view &key) const;

            virtual bool is_valid() const RTTR_NOEXCEPT;

            virtual std::string_view get_signature() const RTTR_NOEXCEPT;

            virtual rttr::type get_return_type() const RTTR_NOEXCEPT;

            virtual bool is_static() const RTTR_NOEXCEPT;

            virtual std::vector<bool> get_is_reference() const RTTR_NOEXCEPT;

            virtual std::vector<bool> get_is_const() const RTTR_NOEXCEPT;

            virtual std::span<const parameter_info> get_parameter_infos() const RTTR_NOEXCEPT;

            virtual variant invoke(instance &object) const;

            virtual variant invoke(instance &object, argument &arg1) const;

            virtual variant invoke(instance &object, argument &arg1, argument &arg2) const;

            virtual variant invoke(instance &object, argument &arg1, argument &arg2, argument &arg3) const;

            virtual variant invoke(instance &object, argument &arg1, argument &arg2, argument &arg3,
                                   argument &arg4) const;

            virtual variant invoke(instance &object, argument &arg1, argument &arg2, argument &arg3,
                                   argument &arg4, argument &arg5) const;

            virtual variant invoke(instance &object, argument &arg1, argument &arg2, argument &arg3,
                                   argument &arg4, argument &arg5, argument &arg6) const;

            virtual variant invoke_variadic(const instance &object, std::vector<argument> &args) const;

        protected:
            void init() RTTR_NOEXCEPT;

        private:
            void create_signature_string() RTTR_NOEXCEPT;

        private:
            std::string m_name;
            std::string m_signature_view;
            rttr::type m_declaring_type;
            std::string m_signature;
        };
    } // end namespace detail
} // end namespace rttr

#endif // RTTR_METHOD_WRAPPER_BASE_H_
