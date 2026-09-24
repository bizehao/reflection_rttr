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

#ifndef RTTR_POLICY_H_
#define RTTR_POLICY_H_

#include "rttr/detail/base/core_prerequisites.h"
#include "rttr/detail/misc/misc_type_traits.h"
#include "rttr/detail/policies/ctor_policies.h"
#include "rttr/detail/policies/meth_policies.h"
#include "rttr/detail/policies/prop_policies.h"
#include <meta>
#include <rttr/detail/misc/ctp.h>

namespace rttr {
    /*!
 * The \ref policy class contains all policies that can be used during the registration of
 * reflection information.
 *
 * For easier usage, the policies are subdivided into following groups:
 * - for methods: \ref policy::meth
 * - for properties: \ref policy::prop
 * - for constructors: \ref policy::ctor
 *
 */
    struct RTTR_API policy {
        enum class method {
            op_default,
            op_return_ref_as_ptr,
            op_discard_return
        };

        static consteval detail::method_policy transform(method op) {
            detail::method_policy rst = detail::method_policy::default_invoke;
            if (op == method::op_return_ref_as_ptr) {
                rst = detail::method_policy::return_as_ptr;
            }
            else if (op == method::op_discard_return) {
                rst = detail::method_policy::discard_return;
            }
            return rst;
        }

        enum class prop {
            op_default,
            op_bind_as_ptr,
            op_as_reference_wrapper
        };

        static consteval std::pair<detail::prop_policy, detail::prop_policy> transform(
            prop op,
            bool readonly = false) {
            detail::prop_policy g = detail::prop_policy::return_as_copy;
            detail::prop_policy s =
                    readonly ? detail::prop_policy::read_only : detail::prop_policy::set_value;

            if (op == prop::op_bind_as_ptr) {
                g = detail::prop_policy::return_as_ptr;
                if (!readonly) {
                    s = detail::prop_policy::set_as_ptr;
                }
            }
            if (op == prop::op_as_reference_wrapper) {
                g = detail::prop_policy::get_as_ref_wrapper;
                if (!readonly) {
                    s = detail::prop_policy::set_as_ref_wrapper;
                }
            }
            return std::make_pair(g, s);
        }

        enum class ctor {
            op_as_raw_ptr,
            op_as_std_shared_ptr,
            op_as_object,
            op_default = op_as_object
        };

        static consteval detail::ctor_policy transform(ctor op) {
            detail::ctor_policy rst = detail::ctor_policy::as_raw_ptr;

            if (op == ctor::op_as_std_shared_ptr) {
                rst = detail::ctor_policy::as_std_shared_ptr;
            }
            if (op == ctor::op_as_object) {
                rst = detail::ctor_policy::as_object;
            }
            return rst;
        }
    };

    namespace class_op {
        struct rename_tag {
            const char *_name;
        };

        consteval rename_tag rename(std::string_view name) {
            // auto pp = std::meta::current_class();
            // std::string_view t = std::meta::identifier_of(pp);
            return rename_tag{std::define_static_string(name)};
        }

        template<typename E>
            requires(std::is_enum_v<E>)
        struct policy_tag {
            E _policy;
        };

        consteval policy_tag<policy::method> method_policy(policy::method ply) {
            return policy_tag{ply};
        }

        consteval policy_tag<policy::prop> prop_policy(policy::prop ply) {
            return policy_tag{ply};
        }

        consteval policy_tag<policy::ctor> ctor_policy(policy::ctor ply) {
            return policy_tag{ply};
        }

        struct ignore_tag {
        };

        constexpr ignore_tag ignore{};
    } // namespace class_op

    template<class T>
    struct metadata_tag {
        const char *key;
        ctp::Param<T> data;
    };

    template<class T>
    consteval auto metadata_impl(std::string_view key, ctp::Param<T> data) {
        return metadata_tag{std::define_static_string(key), std::move(data)};
    }

    template<class T>
    consteval auto metadata(std::string_view key, T data) {
        ctp::Param pdata = data;
        return metadata_impl(key, pdata);
    }
} // end namespace rttr

#endif // RTTR_POLICY_H_
