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

#ifndef RTTR_ENUMERATION_WRAPPER_H_
#define RTTR_ENUMERATION_WRAPPER_H_

#include "rttr/detail/base/core_prerequisites.h"
#include "rttr/detail/enumeration/enumeration_wrapper_base.h"
#include "rttr/detail/enumeration/enum_data.h"
#include "rttr/argument.h"
#include "rttr/variant.h"
#include "rttr/detail/metadata/metadata_handler.h"
#include <utility>
#include <type_traits>
#include <string_view>

namespace rttr {
    namespace detail {
        template<std::meta::info Enum>
        class enumeration_wrapper : public enumeration_wrapper_base, public metadata_handler {
            static constexpr auto enumerators = std::define_static_array(std::meta::enumerators_of(Enum));
            static constexpr auto N = enumerators.size();
            using Enum_Type = [:Enum:];

        public:
            enumeration_wrapper(std::vector<metadata> meta_datas) RTTR_NOEXCEPT : metadata_handler{
                std::move(meta_datas)
            } {
                static_assert(std::is_enum_v<Enum_Type>,
                              "No enum type provided, please create an instance of this class only for enum types!");

                template for (constexpr std::size_t index: std::views::indices(enumerators.size())) {
                    constexpr std::meta::info enumerator = enumerators[index];
                    m_enum_names[index] = std::meta::identifier_of(enumerator);
                    m_enum_values[index] = [:enumerator:];
                    m_enum_variant_values[index] = [:enumerator:];
                }
                constexpr std::meta::info parent = std::meta::parent_of(Enum);
                if constexpr (std::meta::is_type(parent) && std::meta::is_class_type(parent)) {
                    set_declaring_type(type::get<typename [:parent:]>());
                }
            }

            variant get_metadata(const std::string_view &key) const override {
                return metadata_handler::get_metadata(key);
            }

            bool is_valid() const RTTR_NOEXCEPT { return true; }
            rttr::type get_type() const RTTR_NOEXCEPT { return type::get<Enum_Type>(); }

            rttr::type get_underlying_type() const RTTR_NOEXCEPT {
                return rttr::type::get<typename std::underlying_type<Enum_Type>::type>();
            }

            std::span<const std::string_view> get_names() const RTTR_NOEXCEPT {
                return std::span<const std::string_view>(m_enum_names.data(), N);
            }

            std::span<const variant> get_values() const RTTR_NOEXCEPT {
                return std::span<const variant>(m_enum_variant_values.data(), N);
            }

            std::string_view value_to_name(argument &value) const {
                if (!value.is_type<Enum_Type>() &&
                    !value.is_type<typename std::underlying_type<Enum_Type>::type>()) {
                    return std::string_view();
                }

                const Enum_Type enum_value = value.get_value<Enum_Type>();
                int index = 0;
                for (const auto &item: m_enum_values) {
                    if (item == enum_value)
                        return m_enum_names[index];

                    ++index;
                }

                return std::string_view();
            }

            variant name_to_value(std::string_view name) const {
                int index = 0;
                for (const auto &item: m_enum_names) {
                    if (item == name)
                        return m_enum_values[index];

                    ++index;
                }
                return variant();
            }

        private:
            std::array<std::string_view, N> m_enum_names;
            std::array<Enum_Type, N> m_enum_values;
            std::array<variant, N> m_enum_variant_values;
        };
    } // end namespace detail
} // end namespace rttr

#endif // RTTR_ENUMERATION_WRAPPER_H_
