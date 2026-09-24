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

#ifndef RTTR_BASE_CLASSES_H_
#define RTTR_BASE_CLASSES_H_
#include "rttr/detail/constructor/constructor_wrapper_base.h"
#include <meta>

namespace rttr {
    template<typename... U>
    struct type_list;

    namespace detail {
        struct base_class_info {
            base_class_info(type t, void *(*rttr_cast_func)(void *))
                : m_base_type(t), m_rttr_cast_func(rttr_cast_func) {
            }

            type m_base_type;

            void * (*m_rttr_cast_func)(void *);
        };

        /*!
 * If T has a type alias called \a 'base_class_list' then inherits from true_type, otherwise inherits from false_type.
 */
        using info_container = std::vector<detail::base_class_info>;

        /*!
 * This is the trick to make the replacement of `dynamic_cast` possible.
 *
 * For every \p DerivedType is a function pointer stored, which performs a
 * static_cast operation to it's BaseType.
 * The given \p ptr is a void* ptr and has to be of type \p DerivedType,
 * otherwise we would get undefined behavior.
 * Therefore the \ref get_derived_info() function is used. It will return this information.
 */
        template<typename DerivedType, typename BaseType>
        static void *rttr_cast_impl(void *ptr) {
            return static_cast<void *>(static_cast<BaseType *>(static_cast<DerivedType *>(ptr)));
        }

        /*!
 * This class fills from a given type_list the corresponding type objects into a std::vector.
 */
        template<std::meta::info info, std::meta::info baseInfo>
        static RTTR_INLINE void base_fill(info_container &vec) {
            using DerivedT = [:info:];
            using BaseT = [:baseInfo:];
            vec.emplace_back(type::get<BaseT>(), &rttr_cast_impl<DerivedT, BaseT>);

            template for (constexpr auto b: std::define_static_array(
                std::meta::bases_of(baseInfo, std::meta::access_context::unprivileged()))) {
                base_fill<info, std::meta::type_of(b)>(vec);
            }
        }

        template<typename T>
        struct RTTR_LOCAL base_classes {
            static RTTR_INLINE info_container get_types() {
                info_container result;
                return result;
            }
        };

        /*!
 * This helper trait returns a vector with type object of all base classes.
 * When there is no type_list defined or the class has no base class, an empty vector is returned.
 */
        template<typename T> requires (std::is_class_v<T>)
        struct RTTR_LOCAL base_classes<T> {
            static RTTR_INLINE info_container get_types()
            {
                info_container result;
                template for (constexpr auto b: std::define_static_array(
                    std::meta::bases_of(^^T, std::meta::access_context::unprivileged()))) {
                    base_fill<^^T, std::meta::type_of(b)>(result);
                }
                return result;
            }
        };
    } // end namespace detail
} // end namespace rttr

#endif // RTTR_BASE_CLASSES_H_
