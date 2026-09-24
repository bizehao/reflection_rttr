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

#ifndef RTTR_FUNCTION_TRAITS_H_
#define RTTR_FUNCTION_TRAITS_H_

#include "rttr/detail/base/core_prerequisites.h"
#include "rttr/detail/misc/misc_type_traits.h"

#include <functional>
#include <tuple>
#include <type_traits>

namespace rttr {
    namespace detail {
        /////////////////////////////////////////////////////////////////////////////////////

        template<typename T>
        struct is_function_ptr
                : std::integral_constant<bool,
                    std::is_pointer_v<T> && std::is_function_v<std::remove_pointer_t<T> >> {
        };

        /////////////////////////////////////////////////////////////////////////////////////

        template<typename T>
        using is_functor_impl = std::bool_constant<requires() { &T::operator(); }>;

        /*!
 * \brief Returns true whether the given type T is a functor.
 *        i.e. func(...); That can be free function, lambdas or function objects.
 */
        template<typename T>
        struct is_functor : std::conditional_t<std::is_class_v<T>, is_functor_impl<T>, std::false_type> {
        };

        template<typename R, typename... Args>
        struct is_functor<R (*)(Args...)> : std::true_type {
        };

        template<typename R, typename... Args>
        struct is_functor<R (&)(Args...)> : std::true_type {
        };

#ifndef RTTR_NO_CXX17_NOEXCEPT_FUNC_TYPE
        template<typename R, typename... Args>
        struct is_functor<R (*)(Args...) noexcept> : std::true_type {
        };

        template<typename R, typename... Args>
        struct is_functor<R (&)(Args...) noexcept> : std::true_type {
        };
#endif

        /////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

        template<typename T>
        struct function_traits : function_traits<decltype(&T::operator())> {
        };

        template<typename R, typename... Args>
        struct function_traits<R(Args...)> {
            static RTTR_CONSTEXPR_OR_CONST size_t arg_count = sizeof...(Args);

            using return_type = R;
            using arg_types = std::tuple<Args...>;
        };

        template<typename R, typename... Args>
        struct function_traits<R (*)(Args...)> : function_traits<R(Args...)> {
        };

        template<typename R, typename... Args>
        struct function_traits<R (&)(Args...)> : function_traits<R(Args...)> {
        };

        template<typename R, typename C, typename... Args>
        struct function_traits<R (C::*)(Args...)> : function_traits<R(Args...)> {
            using class_type = C;
        };

        template<typename R, typename C, typename... Args>
        struct function_traits<R (C::*)(Args...) const> : function_traits<R(Args...)> {
            using class_type = C;
        };

        template<typename R, typename C, typename... Args>
        struct function_traits<R (C::*)(Args...) volatile> : function_traits<R(Args...)> {
            using class_type = C;
        };

        template<typename R, typename C, typename... Args>
        struct function_traits<R (C::*)(Args...) const volatile> : function_traits<R(Args...)> {
            using class_type = C;
        };

        template<typename R, typename C, typename... Args>
        struct function_traits<R (C::*)(Args...) &> : function_traits<R(Args...)> {
            using class_type = C;
        };

        template<typename R, typename C, typename... Args>
        struct function_traits<R (C::*)(Args...) const &> : function_traits<R(Args...)> {
            using class_type = C;
        };

        template<typename R, typename C, typename... Args>
        struct function_traits<R (C::*)(Args...) volatile &> : function_traits<R(Args...)> {
            using class_type = C;
        };

        template<typename R, typename C, typename... Args>
        struct function_traits<R (C::*)(Args...) const volatile &> : function_traits<R(Args...)> {
            using class_type = C;
        };

        template<typename R, typename C, typename... Args>
        struct function_traits<R (C::*)(Args...) &&> : function_traits<R(Args...)> {
            using class_type = C;
        };

        template<typename R, typename C, typename... Args>
        struct function_traits<R (C::*)(Args...) const &&> : function_traits<R(Args...)> {
            using class_type = C;
        };

        template<typename R, typename C, typename... Args>
        struct function_traits<R (C::*)(Args...) volatile &&> : function_traits<R(Args...)> {
            using class_type = C;
        };

        template<typename R, typename C, typename... Args>
        struct function_traits<R (C::*)(Args...) const volatile &&> : function_traits<R(Args...)> {
            using class_type = C;
        };

#ifndef RTTR_NO_CXX17_NOEXCEPT_FUNC_TYPE
        template<typename R, typename... Args>
        struct function_traits<R (*)(Args...) noexcept> : function_traits<R(Args...)> {
        };

        template<typename R, typename... Args>
        struct function_traits<R (&)(Args...) noexcept> : function_traits<R(Args...)> {
        };

        template<typename R, typename C, typename... Args>
        struct function_traits<R (C::*)(Args...) noexcept> : function_traits<R(Args...)> {
            using class_type = C;
        };

        template<typename R, typename C, typename... Args>
        struct function_traits<R (C::*)(Args...) const noexcept> : function_traits<R(Args...)> {
            using class_type = C;
        };

        template<typename R, typename C, typename... Args>
        struct function_traits<R (C::*)(Args...) volatile noexcept> : function_traits<R(Args...)> {
            using class_type = C;
        };

        template<typename R, typename C, typename... Args>
        struct function_traits<R (C::*)(Args...) const volatile noexcept> : function_traits<R(Args...)> {
            using class_type = C;
        };

        template<typename R, typename C, typename... Args>
        struct function_traits<R (C::*)(Args...) & noexcept> : function_traits<R(Args...)> {
            using class_type = C;
        };

        template<typename R, typename C, typename... Args>
        struct function_traits<R (C::*)(Args...) const & noexcept> : function_traits<R(Args...)> {
            using class_type = C;
        };

        template<typename R, typename C, typename... Args>
        struct function_traits<R (C::*)(Args...) volatile & noexcept> : function_traits<R(Args...)> {
            using class_type = C;
        };

        template<typename R, typename C, typename... Args>
        struct function_traits<R (C::*)(Args...) const volatile & noexcept> : function_traits<R(Args...)> {
            using class_type = C;
        };

        template<typename R, typename C, typename... Args>
        struct function_traits<R (C::*)(Args...) && noexcept> : function_traits<R(Args...)> {
            using class_type = C;
        };

        template<typename R, typename C, typename... Args>
        struct function_traits<R (C::*)(Args...) const && noexcept> : function_traits<R(Args...)> {
            using class_type = C;
        };

        template<typename R, typename C, typename... Args>
        struct function_traits<R (C::*)(Args...) volatile && noexcept> : function_traits<R(Args...)> {
            using class_type = C;
        };

        template<typename R, typename C, typename... Args>
        struct function_traits<R (C::*)(Args...) const volatile && noexcept> : function_traits<R(Args...)> {
            using class_type = C;
        };
#endif

        template<typename T>
        struct function_traits<std::function<T> > : function_traits<T> {
        };

        /////////////////////////////////////////////////////////////////////////////////////
        // use it like e.g:
        // param_types<F, 0>::type

        template<typename F, size_t Index>
        struct param_types {
            using type = typename std::tuple_element<Index, typename function_traits<F>::arg_types>::type;
        };

        template<typename F, size_t Index>
        using param_types_t = typename param_types<F, Index>::type;

        /////////////////////////////////////////////////////////////////////////////////////

        template<typename F>
        struct is_void_func
                : std::conditional_t<std::is_same_v < typename function_traits<F>::return_type, void>,
                  std::true_type,
                  std::false_type>
        {
        };

        /////////////////////////////////////////////////////////////////////////////////////
        // returns an std::true_type, when the given type F is a function type; otherwise an
        // std::false_type.
        template<typename F>
        using is_function = std::integral_constant<bool,
            std::is_member_function_pointer_v<F>
            || std::is_function_v<F> || is_functor<F>::value>;

        /////////////////////////////////////////////////////////////////////////////////////
    } // end namespace detail
} // end namespace rttr

#endif // RTTR_FUNCTION_TRAITS_H_
