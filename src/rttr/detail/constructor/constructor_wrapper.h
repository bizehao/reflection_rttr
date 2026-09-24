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

#ifndef RTTR_CONSTRUCTOR_WRAPPER_H_
#define RTTR_CONSTRUCTOR_WRAPPER_H_

#include "rttr/detail/base/core_prerequisites.h"
#include "rttr/detail/constructor/constructor_wrapper_base.h"
#include "rttr/detail/misc/utility.h"
#include "rttr/detail/misc/function_traits.h"
#include "rttr/argument.h"
#include "rttr/variant.h"
#include "rttr/policy.h"
#include "rttr/detail/parameter_info/parameter_info_wrapper.h"
#include "rttr/detail/metadata/metadata_handler.h"
#include <vector>
#include <utility>
#include <type_traits>

namespace rttr {
    namespace detail {
        template<std::meta::info Ctor, ctor_policy op>
        class constructor_wrapper : public constructor_wrapper_base, public metadata_handler {
            using Class_Type = [:std::meta::parent_of(Ctor):];
            using instanciated_type =
            [:
                []() {
                    if (op == ctor_policy::as_raw_ptr)
                        return std::meta::add_pointer(std::meta::parent_of(Ctor));
                    if (op == ctor_policy::as_std_shared_ptr)
                        return std::meta::substitute(^^std::shared_ptr, {std::meta::parent_of(Ctor)});
                    //op == ctor_policy::as_object
                    return std::meta::parent_of(Ctor);
                }()
            :];
            static constexpr auto params = std::define_static_array(std::meta::parameters_of(Ctor));
            static constexpr auto default_args_num = std::ranges::count_if(params,
                                                                           [](std::meta::info const &i) {
                                                                               return std::meta::has_default_argument(
                                                                                   i);
                                                                           });

        public:
            constructor_wrapper(std::vector<metadata> meta_datas) RTTR_NOEXCEPT : metadata_handler{
                std::move(meta_datas)
            } {
                template for (constexpr auto I: std::views::indices(params.size())) {
                    auto ptr = new parameter_info_wrapper < params[I], I>
                    {
                    };
                    m_param_infos.push_back(std::unique_ptr<parameter_info_wrapper_base>{ptr});
                    m_param_info_list.emplace_back(parameter_info{ptr});
                }

                init();
            }

            variant get_metadata(const std::string_view &key) const override {
                return metadata_handler::get_metadata(key);
            }

            bool is_valid() const override RTTR_NOEXCEPT {
                return true;
            }

            rttr::type get_instantiated_type() const override RTTR_NOEXCEPT {
                return rttr::type::get<instanciated_type>();
            }

            rttr::type get_declaring_type() const override RTTR_NOEXCEPT {
                return rttr::type::get<typename raw_type<Class_Type>::type>();
            }

            std::vector<bool> get_is_reference() const override RTTR_NOEXCEPT {
                auto result = []() consteval {
                    auto tmp = std::views::transform(params,
                                                     [](auto info) {
                                                         return std::meta::is_reference_type(
                                                             std::meta::type_of(info));
                                                     });
                    return std::define_static_array(tmp);
                }();

                return std::vector<bool>(result.begin(), result.end());
            }

            std::vector<bool> get_is_const() const override RTTR_NOEXCEPT {
                auto result = []() consteval {
                    auto tmp = std::views::transform(params,
                                                     [](auto info) {
                                                         return std::meta::is_const_type(
                                                             std::meta::remove_reference(
                                                                 std::meta::type_of(info)));
                                                     });
                    return std::define_static_array(tmp);
                }();

                return std::vector<bool>(result.begin(), result.end());
            }

            std::span<const parameter_info> get_parameter_infos() const override RTTR_NOEXCEPT {
                return std::span<const parameter_info>(m_param_info_list.data(),
                                                       m_param_info_list.size());
            }

            variant invoke() const override {
                return invoke_impl({});
            }

            variant invoke(argument &arg1) const override {
                return invoke_impl({arg1});
            }

            variant invoke(argument &arg1, argument &arg2) const override {
                return invoke_impl({arg1, arg2});
            }

            variant invoke(argument &arg1, argument &arg2, argument &arg3) const override {
                return invoke_impl({arg1, arg2, arg3});
            }

            variant invoke(argument &arg1, argument &arg2, argument &arg3, argument &arg4) const override {
                return invoke_impl({arg1, arg2, arg3, arg4});
            }

            variant invoke(argument &arg1,
                           argument &arg2,
                           argument &arg3,
                           argument &arg4,
                           argument &arg5) const override {
                return invoke_impl({arg1, arg2, arg3, arg4, arg5});
            }

            variant invoke(argument &arg1,
                           argument &arg2,
                           argument &arg3,
                           argument &arg4,
                           argument &arg5,
                           argument &arg6) const override {
                return invoke_impl({arg1, arg2, arg3, arg4, arg5, arg6});
            }

            variant invoke_variadic(std::vector<argument> &arg_list) const override {
                return invoke_impl(arg_list);
            }

        private:
            template<std::size_t... Is>
            static variant override_fun(std::vector<argument> args, std::index_sequence<Is...>) {
                //check args type
                bool check = detail::check_all_true(
                    args[Is].template is_type<typename [:std::meta::type_of(params[Is]):]>()...);
                if (!check) {
                    return variant{};
                }

                if constexpr (op == ctor_policy::as_raw_ptr) {
                    return variant{
                        new Class_Type{
                            args[Is].get_value<typename [:std::meta::type_of(params[Is]):]>()...
                        }
                    };
                }
                else if constexpr (op == ctor_policy::as_std_shared_ptr) {
                    return variant{
                        std::make_shared<Class_Type>(
                            args[Is].get_value<typename [:std::meta::type_of(params[Is]):]>()...)
                    };
                }
                else {
                    //op == ctor_policy::as_object
                    return variant{
                        Class_Type{args[Is].get_value<typename [:std::meta::type_of(params[Is]):]>()...}
                    };
                }
            }

            static variant invoke_impl(std::vector<argument> args) {
                constexpr std::size_t at_most_args_num = params.size() - default_args_num;
                if (args.size() < at_most_args_num || args.size() > params.size()) {
                    return variant{};
                }

                template for (constexpr std::size_t Js: std::views::iota(
                    at_most_args_num,
                    params.size() + 1)) {
                    if (Js == args.size()) {
                        return override_fun(std::move(args), std::make_index_sequence < Js >
                        {
                        });
                    }
                }
                return variant{};
            }

            std::vector<std::unique_ptr<parameter_info_wrapper_base> > m_param_infos;
            std::vector<parameter_info> m_param_info_list;
        };
    } // end namespace detail
} // end namespace rttr

#endif // RTTR_CONSTRUCTOR_WRAPPER_H_