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

#ifndef RTTR_COMPARE_LESS_IMPL_H_
#define RTTR_COMPARE_LESS_IMPL_H_

#include "rttr/type.h"
#include "rttr/detail/comparison/compare_array_less.h"
#include <type_traits>

namespace rttr
{
namespace detail
{

/////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
struct remove_reference_wrapper_impl
{
    using type = T;
};

template <typename T>
struct remove_reference_wrapper_impl<std::reference_wrapper<T> >
{
    using type = remove_reference_wrapper_impl<T>::type;
};

template <typename T>
using remove_reference_wrapper = remove_reference_wrapper_impl<T>::type;
//todo:bzh mingw:bug requires() { array == array }; 数组比较
template <typename T>
RTTR_INLINE bool compare_less_than(const T& lhs, const T& rhs, int& result)
{
    if constexpr (is_less_than_comparable<remove_reference_wrapper<T> >::value)
    {
        result = (lhs < rhs ? -1 : ((rhs < lhs) ? 1 : 0));
        return true;
    }
    else
    {
        if constexpr (std::is_array_v<T>)
        {
            result = compare_array_less(lhs, rhs) ? -1 : 1;
            return true;
        }
        else
        {
            return compare_types_less_than(&lhs, &rhs, type::get<T>(), result);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////

} // end namespace detail
} // end namespace rttr

#endif // RTTR_COMPARE_LESS_IMPL_H_