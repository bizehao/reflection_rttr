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

#ifndef RTTR_PROPERTY_WRAPPER_FUNC_H_
#define RTTR_PROPERTY_WRAPPER_FUNC_H_

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
// global function getter/setter - function pointer
template<std::meta::info Getter, std::meta::info Setter, prop_policy GetPolicy, prop_policy
    SetPolicy>
class property_wrapper_function : public property_wrapper_base, public metadata_handler {
    using return_type = [:is_lambda(Getter)
                              ? std::meta::return_type_of(get_lambda_fun(Getter))
                              : std::meta::return_type_of(Getter):];
    using arg_type = [:[]() {
        if (Setter == ^^void) {
            return ^^void;
        }
        else {
            if (is_lambda(Getter)) {
                return std::meta::type_of(std::meta::parameters_of(get_lambda_fun(Setter))[0]);
            }
            else {
                return std::meta::type_of(std::meta::parameters_of(Setter)[0]);
            }
        }
    }():];

public:
    property_wrapper_function(std::string_view name, std::vector<metadata> meta_datas) RTTR_NOEXCEPT
        : property_wrapper_base(name, type::get<invalid_type>()),
          metadata_handler{std::move(meta_datas)} {
        constexpr auto t_Getter = is_lambda(Getter) ? get_lambda_fun(Getter) : Getter;
        constexpr auto t_Setter = is_lambda(Setter) ? get_lambda_fun(Setter) : Setter;
        static_assert(std::meta::parameters_of(t_Getter).empty(),
                      "Invalid number of argument, please provide a getter-function without arguments.")
                ;
        if constexpr (SetPolicy != prop_policy::read_only) {
            static_assert(std::meta::parameters_of(t_Setter).size() == 1,
                          "Invalid number of argument, please provide a setter-function with exactly one argument.")
                    ;
            static_assert(std::is_same_v<return_type, arg_type>,
                          "Please provide the same signature for getter and setter!");
        }
        if constexpr (GetPolicy == prop_policy::get_as_ref_wrapper || GetPolicy ==
                      prop_policy::return_as_ptr) {
            static_assert(std::is_reference_v<return_type>, "return_type need to reference_type");
        }
        init();
    }

    variant get_metadata(const std::string_view &key) const override {
        return metadata_handler::get_metadata(key);
    }

    bool is_valid() const override RTTR_NOEXCEPT {
        return true;
    }

    bool is_readonly() const override RTTR_NOEXCEPT {
        return SetPolicy == prop_policy::read_only;
    }

    bool is_static() const override RTTR_NOEXCEPT {
        return true;
    }

    rttr::type get_type() const override RTTR_NOEXCEPT {
        constexpr bool is_readonly = SetPolicy == prop_policy::read_only;
        if constexpr (GetPolicy == prop_policy::return_as_ptr) {
            if constexpr (is_readonly) {
                return type::get<std::add_const_t<std::remove_reference_t<return_type> > *>();
            }
            else {
                return type::get<std::remove_reference_t<return_type> *>();
            }
        }
        else if (GetPolicy == prop_policy::get_as_ref_wrapper) {
            if constexpr (is_readonly) {
                return type::get<std::reference_wrapper<std::add_const_t<std::remove_reference_t<
                    return_type> > > >();
            }
            else {
                return type::get<std::reference_wrapper<std::remove_reference_t<return_type> > >();
            }
        }
        else {
            return type::get<return_type>();
        }
    }

    bool set_value(instance &object, argument &arg) const override {
        if constexpr (SetPolicy == prop_policy::set_value) {
            if (arg.is_type<arg_type>()) {
                std::invoke([:Setter:], arg.get_value<arg_type>());
                return true;
            }
            return false;
        }
        else if constexpr (SetPolicy == prop_policy::set_as_ptr) {
            using arg_type_t = std::remove_reference_t<arg_type>;
            if (arg.is_type<arg_type_t *>()) {
                std::invoke([:Setter:], *arg.get_value<arg_type_t *>());
                return true;
            }
            return false;
        }
        else if constexpr (SetPolicy == prop_policy::set_as_ref_wrapper) {
            using arg_type_t = std::remove_reference_t<arg_type>;
            if (arg.is_type<std::reference_wrapper<arg_type_t> >()) {
                std::invoke([:Setter:], arg.get_value<std::reference_wrapper<arg_type_t> >().get());
                return true;
            }
            return false;
        }
        else {
            //prop_op_policy::read_only
            return false;
        }
    }

    variant get_value(instance &object) const override {
        if constexpr (GetPolicy == prop_policy::return_as_ptr) {
            if constexpr (SetPolicy == prop_policy::read_only) {
                const auto &v = std::invoke([:Getter:]);
                return variant(&v);
            }
            else {
                auto &v = std::invoke([:Getter:]);
                return variant(&v);
            }
        }
        else if constexpr (GetPolicy == prop_policy::get_as_ref_wrapper) {
            if constexpr (SetPolicy == prop_policy::read_only) {
                auto v = std::cref(std::invoke([:Getter:]));
                return variant(std::move(v));
            }
            else {
                auto v = std::ref(std::invoke([:Getter:]));
                return variant(std::move(v));
            }
        }
        else {
            //prop_op_policy::return_as_copy
            auto v = std::invoke([:Getter:]);
            return variant(std::move(v));
        }
    }
};

#endif // RTTR_PROPERTY_WRAPPER_FUNC_H_