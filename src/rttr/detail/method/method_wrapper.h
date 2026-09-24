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

#ifndef RTTR_METHOD_WRAPPER_H_
#define RTTR_METHOD_WRAPPER_H_

#include "rttr/detail/base/core_prerequisites.h"
#include "rttr/detail/method/method_wrapper_base.h"
#include "rttr/detail/misc/function_traits.h"
#include "rttr/argument.h"
#include "rttr/instance.h"
#include "rttr/variant.h"
#include  "rttr/detail/policies/meth_policies.h"
#include "rttr/detail/parameter_info/parameter_info_wrapper.h"
#include "rttr/type.h"
#include "rttr/detail/metadata/metadata_handler.h"

#include <functional>
#include <string>
#include <array>
#include <span>
#include <ranges>
#include <span>

namespace rttr {
    namespace detail {
        template<std::meta::info Fun, method_policy op>
        class method_wrapper : public method_wrapper_base, public metadata_handler {
            using Declaring_Type = [:std::meta::is_class_member(Fun) ? std::meta::parent_of(Fun) : ^^invalid_type:];
            using Return_Type = [:is_lambda(Fun)
                                      ? std::meta::return_type_of(get_lambda_fun(Fun))
                                      : std::meta::return_type_of(Fun):];

            static constexpr auto params = []() {
                if (is_lambda(Fun)) {
                    return std::define_static_array(std::meta::parameters_of(get_lambda_fun(Fun)));
                }
                else {
                    return std::define_static_array(std::meta::parameters_of(Fun));
                }
            }();
            static constexpr auto default_args_num = std::ranges::count_if(params, [](std::meta::info const &i) {
                return std::meta::has_default_argument(i);
            });

        public:
            method_wrapper(std::string_view name, std::vector<metadata> meta_datas) RTTR_NOEXCEPT
                : method_wrapper_base(name, type::get<Declaring_Type>()), metadata_handler{std::move(meta_datas)} {
                if constexpr (op == method_policy::return_as_ptr) {
                    static_assert(std::is_reference_v<Return_Type>,
                                  "op is return_ref_as_ptr, return result need is left reference");
                }

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

            bool is_valid() const RTTR_NOEXCEPT { return true; }

            bool is_static() const RTTR_NOEXCEPT {
                return !std::meta::is_class_member(Fun) || std::meta::is_static_member(Fun);
            }

            rttr::type get_return_type() const RTTR_NOEXCEPT {
                if constexpr (op == method_policy::return_as_ptr) {
                    return type::get<std::remove_reference_t<Return_Type> *>();
                }
                else if constexpr (op == method_policy::discard_return) {
                    return type::get<void>();
                }
                else {
                    return type::get<Return_Type>();
                }
            }

            std::vector<bool> get_is_reference() const RTTR_NOEXCEPT {
                std::vector<bool> rst;
                template for (constexpr auto it: params) {
                    rst.push_back(std::meta::is_reference_type(std::meta::type_of(it)));
                }
                return rst;
            }

            std::vector<bool> get_is_const() const RTTR_NOEXCEPT {
                auto result = []() consteval {
                    auto tmp = std::views::transform(params, [](auto info) {
                        return std::meta::is_const_type(std::meta::remove_reference(std::meta::type_of(info)));
                    });
                    return std::define_static_array(tmp);
                }();
                return std::vector<bool>(result.begin(), result.end());
            }

            std::span<const parameter_info> get_parameter_infos() const RTTR_NOEXCEPT {
                return std::span<const parameter_info>(m_param_info_list.data(), m_param_info_list.size());
            }

            variant invoke(instance &object) const {
                return invoke_impl(object, {});
            }

            variant invoke(instance &object, argument &arg1) const {
                return invoke_impl(object, {arg1});
            }

            variant invoke(instance &object, argument &arg1, argument &arg2) const {
                return invoke_impl(object, {arg1, arg2});
            }

            variant invoke(instance &object, argument &arg1, argument &arg2, argument &arg3) const {
                return invoke_impl(object, {arg1, arg2, arg3});
            }

            variant invoke(instance &object, argument &arg1, argument &arg2, argument &arg3, argument &arg4) const {
                return invoke_impl(object, {arg1, arg2, arg3, arg4});
            }

            variant invoke(instance &object, argument &arg1, argument &arg2, argument &arg3, argument &arg4,
                           argument &arg5) const {
                return invoke_impl(object, {arg1, arg2, arg3, arg4, arg5});
            }

            variant invoke(instance &object, argument &arg1, argument &arg2, argument &arg3, argument &arg4,
                           argument &arg5, argument &arg6) const {
                return invoke_impl(object, {arg1, arg2, arg3, arg4, arg5, arg6});
            }

            variant invoke_variadic(const instance &object, std::vector<argument> &args) const {
                return invoke_impl(object, args);
            }

        private:
            template<std::size_t... Is>
            static variant override_fun(const instance &obj, std::vector<argument> args, std::index_sequence<Is...>) {
                //check args type
                bool check = detail::check_all_true(
                    args[Is].template is_type<typename [:std::meta::type_of(params[Is]):]>()...);
                if (!check) {
                    return variant{};
                }

                constexpr bool is_mem_fun = std::meta::is_class_member(Fun) && !std::meta::is_static_member(Fun);
                if constexpr (std::is_same_v<Return_Type, void>) {
                    if constexpr (is_mem_fun) {
                        Declaring_Type *ptr = obj.try_convert<Declaring_Type>();
                        if (ptr) {
                            ptr->[:Fun:](args[Is].get_value<typename [:std::meta::type_of(params[Is]):]>()...);
                            return variant{void_variant_type{}};
                        }
                        else {
                            return variant{};
                        }
                    }
                    else {
                        [:Fun:](args[Is].get_value<typename [:std::meta::type_of(params[Is]):]>()...);
                        return variant{void_variant_type{}};
                    }
                }
                else {
                    if constexpr (is_mem_fun) {
                        Declaring_Type *ptr = obj.try_convert<Declaring_Type>();
                        if (ptr) {
                            if constexpr (op == method_policy::discard_return) {
                                ptr->[:Fun:](args[Is].get_value<typename [:std::meta::type_of(params[Is]):]>()...);
                                return variant{void_variant_type{}};
                            }
                            else if constexpr (op == method_policy::return_as_ptr) {
                                return variant{
                                    &ptr->[:Fun:](args[Is].get_value<typename [:std::meta::type_of(params[Is]):]>()...)
                                };
                            }
                            else {
                                return variant{
                                    ptr->[:Fun:](args[Is].get_value<typename [:std::meta::type_of(params[Is]):]>()...)
                                };
                            }
                        }
                        else {
                            return variant{};
                        }
                    }
                    else {
                        if constexpr (op == method_policy::discard_return) {
                            [:Fun:](args[Is].get_value<typename [:std::meta::type_of(params[Is]):]>()...);
                            return variant{void_variant_type{}};
                        }
                        else if constexpr (op == method_policy::return_as_ptr) {
                            return variant{
                                &[:Fun:](args[Is].get_value<typename [:std::meta::type_of(params[Is]):]>()...)
                            };
                        }
                        else {
                            return variant{
                                [:Fun:](args[Is].get_value<typename [:std::meta::type_of(params[Is]):]>()...)
                            };
                        }
                    }
                }
            }

            static variant invoke_impl(const instance &obj, std::vector<argument> args) {
                constexpr std::size_t at_most_args_num = params.size() - default_args_num;
                if (args.size() < at_most_args_num || args.size() > params.size()) {
                    return variant{};
                }

                template for (constexpr std::size_t Js: std::views::iota(at_most_args_num, params.size() + 1)) {
                    if (Js == args.size()) {
                        return override_fun(obj, std::move(args), std::make_index_sequence < Js >
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

#endif // RTTR_METHOD_WRAPPER_H_
