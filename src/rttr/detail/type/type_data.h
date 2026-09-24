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

#ifndef RTTR_TYPE_DATA_H_
#define RTTR_TYPE_DATA_H_

#include "rttr/detail/metadata/metadata.h"
#include "rttr/detail/misc/misc_type_traits.h"
#include "rttr/detail/misc/function_traits.h"
#include "rttr/detail/misc/template_type_trait.h"
#include "rttr/detail/type/base_classes.h"
#include "rttr/detail/type/get_derived_info_func.h"
#include "rttr/detail/type/get_create_variant_func.h"
#include "rttr/detail/type/type_register.h"
#include "rttr/detail/misc/utility.h"
#include "rttr/destructor.h"
#include "rttr/method.h"
#include "rttr/property.h"
#include "rttr/constructor.h"
#include "rttr/destructor.h"
#include "rttr/variant.h"


#include <type_traits>
#include <bitset>


namespace rttr {
    namespace detail {
        struct type_data;

        template<typename T>
RTTR_INLINE type_data &get_type_data() RTTR_NOEXCEPT;

RTTR_LOCAL type_data *get_invalid_type_data() RTTR_NOEXCEPT;

        static rttr::type get_invalid_type() RTTR_NOEXCEPT;

        using rttr_cast_func = void*(*)(void *);
        using get_derived_info_func = derived_info(*)(void *);

        class enumeration_wrapper_base;

        /////////////////////////////////////////////////////////////////////////////////////////

        struct RTTR_LOCAL class_data {
            class_data(get_derived_info_func func, std::vector<type> nested_types)
                : m_derived_info_func(func),
                  m_nested_types(nested_types),
                  m_dtor(create_invalid_item<destructor>()) {
            }

            get_derived_info_func m_derived_info_func;
            std::vector<type> m_base_types;
            std::vector<type> m_derived_types;
            std::vector<rttr_cast_func> m_conversion_list;
            std::vector<property> m_properties;
            std::vector<method> m_methods;
            std::vector<constructor> m_ctors;
            std::vector<type> m_nested_types;
            destructor m_dtor;
        };

        enum class type_trait_infos : std::size_t {
            is_class = 0,
            is_enum,
            is_array,
            is_pointer,
            is_arithmetic,
            is_function_pointer,
            is_member_object_pointer,
            is_member_function_pointer,
            is_associative_container,
            is_sequential_container,
            is_template_instantiation,

            TYPE_TRAIT_COUNT
        };

        using type_traits = std::bitset<static_cast<std::size_t>(type_trait_infos::TYPE_TRAIT_COUNT)>;

        /////////////////////////////////////////////////////////////////////////////////////////

        namespace impl {
            using create_variant_func = decltype(&create_invalid_variant_policy::create_variant);
            using get_base_types_func = decltype(&base_classes<int>::get_types);
            using create_wrapper_func = void(*)(const argument &arg, variant &var);
            using get_metadata_func = std::vector<detail::metadata>& (*)();
            using get_class_data_func = class_data& (*)();
        } // end namespace impl

        /////////////////////////////////////////////////////////////////////////////////////////

        struct RTTR_LOCAL type_data {
            type_data *raw_type_data;
            type_data *wrapped_type;
            type_data *array_raw_type;

            std::string name;
            std::string type_name;

            std::size_t get_sizeof;
            std::size_t get_pointer_dimension;

            impl::create_variant_func create_variant;

            // this info should not be stored, its just temporarily,
            // thats why we store it as function pointer
            impl::get_base_types_func get_base_types;


            enumeration_wrapper_base *enum_wrapper;
            impl::get_metadata_func get_metadata;
            impl::create_wrapper_func create_wrapper;

            bool is_valid;

    RTTR_FORCE_INLINE bool type_trait_value(type_trait_infos type_trait) const RTTR_NOEXCEPT {
                return m_type_traits.test(static_cast<std::size_t>(type_trait));
            }


            type_traits m_type_traits;
            class_data m_class_data;
        };

        /////////////////////////////////////////////////////////////////////////////////

        template<typename Wrapper, typename Wrapped_Type>
RTTR_LOCAL RTTR_INLINE void create_wrapper(const argument &arg, variant &var) {
            if (arg.get_type() != rttr::type::get<Wrapped_Type>())
                return;

            auto &wrapped_type = arg.get_value<Wrapped_Type>();
            var = wrapper_mapper<Wrapper>::create(wrapped_type);
        }

        template<typename Wrapper>
RTTR_LOCAL RTTR_INLINE impl::create_wrapper_func get_create_wrapper_func() {
            using Tp = wrapper_mapper_t<Wrapper>;

            if constexpr (is_wrapper<Wrapper>::value &&
                          ::rttr::detail::is_copy_constructible<Wrapper>::value &&
                          std::is_default_constructible<Wrapper>::value &&
                          has_create_wrapper_func<Wrapper>::value) {
                return &create_wrapper<Wrapper, Tp>;
            }
            else {
                return nullptr;
            }
        }

        template<typename T>
RTTR_LOCAL RTTR_INLINE std::vector<detail::metadata> &get_metadata_func_impl() {
            static std::unique_ptr<std::vector<detail::metadata> > obj = std::make_unique<std::vector<
                detail::metadata> >();
            return (*obj.get());
        }

        /////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

        using type_trait_value = uint64_t;
#define TYPE_TRAIT_TO_BITSET_VALUE(trait) (static_cast<std::uint64_t>(std::trait<T>::value) << static_cast<std::size_t>(type_trait_infos::trait))
#define TYPE_TRAIT_TO_BITSET_VALUE_2(trait, enum_key) (static_cast<std::uint64_t>(trait<T>::value) << static_cast<std::size_t>(type_trait_infos::enum_key))
    } // end namespace detail
} // end namespace rttr


/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

namespace rttr {
    namespace detail {
        template<typename T>
        RTTR_LOCAL std::unique_ptr<type_data> make_type_data() {
            constexpr auto info = ^^T;
            std::string_view name;
            if constexpr (std::meta::has_identifier(info)) {
                name = std::meta::identifier_of(info);
            }
            else {
                name = std::meta::display_string_of(info);
            }
            auto type_name = std::meta::display_string_of(info);

            std::size_t get_sizeof = 0;
            if constexpr (std::meta::is_complete_type(info)) {
                if constexpr (!std::meta::is_function_type(info)) {
                    get_sizeof = std::meta::size_of(info);
                }
            }

            type_data *invalid_type_data = get_invalid_type().m_type_data;

            type_data *raw_type_data = invalid_type_data;
            if constexpr (!std::is_same_v<T, raw_type_t<T> >) {
                raw_type_data = rttr::type::get<raw_type_t<T> >().m_type_data;
            }

            type_data *wrapped_type_data = invalid_type_data;
            if constexpr (is_wrapper<T>::value) {
                wrapped_type_data = rttr::type::get<wrapper_mapper_t<T> >().m_type_data;
            }

            type_data *array_raw_type_data = invalid_type_data;
            if constexpr (std::is_array_v<T>) {
                array_raw_type_data = rttr::type::get<raw_array_type_t<T> >().m_type_data;
            }

            auto obj = std::make_unique<type_data>(type_data{
                raw_type_data,
                wrapped_type_data,
                array_raw_type_data,

                std::string{name},
                std::string{type_name},

                get_sizeof,
                pointer_count<T>::value,

                &create_variant_func<T>::create_variant,
                &base_classes<T>::get_types,
                nullptr,
                get_metadata_func_impl<T>,
                get_create_wrapper_func<T>(),

                true,
                type_trait_value{
                    TYPE_TRAIT_TO_BITSET_VALUE(is_class) |
                    TYPE_TRAIT_TO_BITSET_VALUE(is_enum) |
                    TYPE_TRAIT_TO_BITSET_VALUE(is_array) |
                    TYPE_TRAIT_TO_BITSET_VALUE(is_pointer) |
                    TYPE_TRAIT_TO_BITSET_VALUE(is_arithmetic) |
                    TYPE_TRAIT_TO_BITSET_VALUE_2(is_function_ptr, is_function_pointer) |
                    TYPE_TRAIT_TO_BITSET_VALUE(is_member_object_pointer) |
                    TYPE_TRAIT_TO_BITSET_VALUE(is_member_function_pointer) |
                    TYPE_TRAIT_TO_BITSET_VALUE_2(::rttr::detail::is_associative_container,
                                                 is_associative_container) |
                    TYPE_TRAIT_TO_BITSET_VALUE_2(::rttr::detail::is_sequential_container,
                                                 is_sequential_container) |
                    TYPE_TRAIT_TO_BITSET_VALUE_2(::rttr::detail::template_type_trait,
                                                 is_template_instantiation)
                },
                class_data(get_most_derived_info_func<T>(),
                           template_type_trait<T>::get_template_arguments())
            });
            return obj;
        }

        /////////////////////////////////////////////////////////////////////////////////////////
    } // end namespace detail
} // end namespace rttr

#endif // RTTR_TYPE_DATA_H_