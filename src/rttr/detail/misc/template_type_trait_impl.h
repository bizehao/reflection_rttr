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

#ifndef RTTR_TEMPLATE_TYPE_TRAIT_IMPL_H_
#define RTTR_TEMPLATE_TYPE_TRAIT_IMPL_H_

#include "rttr/type.h"

namespace rttr {
    template<auto T>
    struct constant_value {
    };

    namespace detail {
        template<typename T>
        struct template_type_trait {
            static std::vector<::rttr::type> get_template_arguments() {
                static auto result = []() {
                    std::vector<::rttr::type> tmp;
                    if constexpr (value) {
                        template for (constexpr auto arg: std::define_static_array(
                            std::meta::template_arguments_of(^^T))) {
                            if constexpr (std::meta::is_type(arg)) {
                                tmp.push_back(rttr::type::get<typename [:arg:]>());
                            }
                            else {
                                constexpr auto v = [:arg:];
                                tmp.push_back(rttr::type::get<constant_value<v> >());
                            }
                        }
                    }
                    return tmp;
                }();
                return result;
            }

            static constexpr bool value = std::meta::has_template_arguments(^^T);
        };

        template<auto _Xv>
        struct template_type_trait<constant_value<_Xv> > {
            static std::vector<::rttr::type> get_template_arguments() {
                return {};
            }

            static constexpr bool value = true;
        };
    }
}
#endif // RTTR_TEMPLATE_TYPE_TRAIT_IMPL_H_
