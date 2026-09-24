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

#ifndef RTTR_ARGUMENT_H_
#define RTTR_ARGUMENT_H_

#include "rttr/detail/base/core_prerequisites.h"
#include "rttr/detail/misc/misc_type_traits.h"

#include <type_traits>
#include <utility>

namespace rttr {
    class type;
    class variant;
    class variant_array;
    class instance;

    namespace {
        template<typename T>
        concept decay_arg_t = !std::is_same_v<argument, T>;

        template<typename T>
        concept is_variant = std::is_same_v<std::remove_cv_t<std::remove_reference_t<T> >, variant>;

        template<typename T>
        concept arg_value_t = !std::is_rvalue_reference_v<T> && !is_variant<T>;

        template<typename T>
        concept arg_rvalue_t = std::is_rvalue_reference_v<T> && !is_variant<T>;

        template<typename T>
        concept ptr_type = std::is_pointer_v<T>;

        template<typename T>
        concept non_ptr_type = !std::is_pointer_v<T>;

        template<typename T>
        concept is_variant_t = is_variant<T> && !std::is_rvalue_reference_v<T>;

        template<typename T>
        concept is_variant_ref_t = is_variant<T> && std::is_rvalue_reference_v<T>;
    }

    /*!
 * The \ref argument class is used for forwarding arguments to \ref property "properties" or \ref method "methods".
 *
 * \remark This class should never be explicitly instantiated.
 */
    class RTTR_API argument {
    public:
        RTTR_INLINE argument() RTTR_NOEXCEPT;

        RTTR_INLINE argument(argument &&arg) RTTR_NOEXCEPT;

        RTTR_INLINE argument(const argument &other) RTTR_NOEXCEPT;

        RTTR_INLINE argument(variant &var) RTTR_NOEXCEPT;

        RTTR_INLINE argument(const variant &var) RTTR_NOEXCEPT;

        template<decay_arg_t T>
        RTTR_INLINE argument(const T &data) RTTR_NOEXCEPT;

        template<decay_arg_t T>
        RTTR_INLINE argument(T &data) RTTR_NOEXCEPT;

    RTTR_INLINE argument &operator=(const argument &other) RTTR_NOEXCEPT;

    RTTR_INLINE type get_type() const RTTR_NOEXCEPT;
#ifdef DOXYGEN
        template<typename T>
    RTTR_INLINE bool is_type() const RTTR_NOEXCEPT;

        template<typename T>
    RTTR_INLINE T &get_value() const RTTR_NOEXCEPT;
#else
        template<ptr_type T>
    RTTR_INLINE bool is_type() const RTTR_NOEXCEPT;

        template<non_ptr_type T>
    RTTR_INLINE bool is_type() const RTTR_NOEXCEPT;

        template<arg_value_t T>
    RTTR_INLINE T &get_value() const RTTR_NOEXCEPT;

        template<arg_rvalue_t T>
        RTTR_INLINE std::remove_reference_t<T> &&get_value() const RTTR_NOEXCEPT;

        template<is_variant_t T>
    RTTR_INLINE T &get_value() const RTTR_NOEXCEPT;

        template<is_variant_ref_t T>
        RTTR_INLINE std::remove_reference_t<T> &&get_value() const RTTR_NOEXCEPT;
#endif

    private:
        const void *m_data;
        const variant *m_variant;
        const type m_type;
    };
} // end namespace rttr

#include "rttr/detail/impl/argument_impl.h"

#endif // RTTR_ARGUMENT_H_
