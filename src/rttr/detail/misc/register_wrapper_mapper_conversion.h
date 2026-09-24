/************************************************************************************
*                                                                                   *
*   Copyright (c) 2014, 2015 - 2017 Axel Menzel <info@rttr.org>                     *
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

#ifndef RTTR_REGISTER_WRAPPER_MAPPER_CONVERSION_H_
#define RTTR_REGISTER_WRAPPER_MAPPER_CONVERSION_H_

#include "rttr/detail/base/core_prerequisites.h"

#include "rttr/detail/misc/misc_type_traits.h"

#include <type_traits>

namespace rttr {
    template<typename... U>
    struct type_list;

    namespace detail {
        /*!
 * Determine if the given type \a T has a wrapper_mapper method called `convert`.
 */
        template<typename T>
        concept has_conversion_function_impl = requires()
        {
            &wrapper_mapper<raw_type_t<T> >::convert;
        };

        /*!
 * If \a T has a wrapper_mapper function `convert` then is the same like `std::true_type`, otherwise inherits from `std::false_type`.
 */
        template<typename T>
        using has_wrapper_conv_func = std::integral_constant<bool, has_conversion_function_impl<T> >;

        /////////////////////////////////////////////////////////////////////////////////////

        /*!
 * Registers for the given type \p DerivedClass a conversion function
 * from \ref wrapper_mapper<T> from and to the types in the list `T...`.
 */
        template<std::meta::info info, std::meta::info baseInfo>
        static RTTR_INLINE void perform() {
            using DerivedClass = [:info:];
            using BaseClass = [:baseInfo:];
            type::register_converter_func(wrapper_mapper<DerivedClass>::template convert<BaseClass>);
            using return_type = typename function_traits<decltype(&wrapper_mapper<DerivedClass>::template convert<
                BaseClass>)>::return_type;
            // TO DO: remove raw_type_t, std::shared_ptr<const T> should also be converted, when necessary
            using wrapped_derived_t = raw_type_t<wrapper_mapper_t<DerivedClass> >;
            type::register_converter_func(wrapper_mapper<return_type>::template convert<wrapped_derived_t>);

            template for (constexpr auto b: std::define_static_array(
                std::meta::bases_of(baseInfo, std::meta::access_context::unprivileged()))) {
                perform<info, std::meta::type_of(b)>();
            }
        }

        /////////////////////////////////////////////////////////////////////////////////////

        template<typename T> requires (is_wrapper<T>::value)
        void reg_wrapper_converter_for_base_classes() {
            constexpr auto info = ^^T;
            constexpr auto raw_info = ^^raw_type_t<wrapper_mapper_t<T> >;
            template for (constexpr auto b: std::define_static_array(
                std::meta::bases_of(raw_info, std::meta::access_context::unprivileged()))) {
                perform<info, std::meta::type_of(b)>();
            }
        };

        /////////////////////////////////////////////////////////////////////////////////////
    } // end namespace detail
} // end namespace rttr

#endif // RTTR_REGISTER_WRAPPER_MAPPER_CONVERSION_H_
