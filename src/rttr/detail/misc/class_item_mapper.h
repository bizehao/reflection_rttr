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

#ifndef RTTR_CLASS_ITEM_MAPPER_H_
#define RTTR_CLASS_ITEM_MAPPER_H_

#include "rttr/detail/base/core_prerequisites.h"

#include <vector>
#include <type_traits>

namespace rttr {
    class property;
    class method;
    class constructor;
    class enumeration;
    class destructor;

    namespace detail {
        class property_wrapper_base;
        class method_wrapper_base;
        class constructor_wrapper_base;
        class destructor_wrapper_base;
        class enumeration_wrapper_base;

        template<typename T>
        struct class_item_to_wrapper {
            using type = void;
        };

        template<>
        struct class_item_to_wrapper<property> {
            using type = property_wrapper_base;
        };

        template<>
        struct class_item_to_wrapper<method> {
            using type = method_wrapper_base;
        };

        template<>
        struct class_item_to_wrapper<constructor> {
            using type = constructor_wrapper_base;
        };

        template<>
        struct class_item_to_wrapper<destructor> {
            using type = destructor_wrapper_base;
        };

        template<>
        struct class_item_to_wrapper<enumeration> {
            using type = enumeration_wrapper_base;
        };

        template<typename T>
        using class_item_to_wrapper_t = typename class_item_to_wrapper<T>::type;


        template<typename T>
        T create_item(const class_item_to_wrapper_t<T> *wrapper);

        template<typename T>
        T create_invalid_item();
    } // end namespace detail
} // end namespace rttr

#endif // RTTR_CLASS_ITEM_MAPPER_H_
