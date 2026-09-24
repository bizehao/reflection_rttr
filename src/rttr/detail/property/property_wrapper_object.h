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

#ifndef RTTR_PROPERTY_WRAPPER_OBJECT_H_
#define RTTR_PROPERTY_WRAPPER_OBJECT_H_

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
// global property read write

template<std::meta::info Var, prop_policy GetPolicy, prop_policy SetPolicy>
class property_wrapper_object : public property_wrapper_base, public metadata_handler {
    using ParentType = [:std::meta::is_class_member(Var)
                             ? std::meta::parent_of(Var)
                             : ^^invalid_type:];
    using VarType = [:std::meta::type_of(Var):];

public:
    property_wrapper_object(std::string_view name, std::vector<metadata> meta_datas) RTTR_NOEXCEPT
        : property_wrapper_base(name, type::get<ParentType>()),
          metadata_handler{std::move(meta_datas)} {
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
        if constexpr (std::is_same_v<ParentType, invalid_type>) {
            return SetPolicy != prop_policy::set_as_ptr && SetPolicy !=
                   prop_policy::set_as_ref_wrapper;
        }
        else {
            return std::meta::is_static_member(Var);
        }
    }

    rttr::type get_type() const override RTTR_NOEXCEPT {
        if constexpr (GetPolicy == prop_policy::return_as_ptr) {
            if constexpr (SetPolicy == prop_policy::read_only) {
                return type::get<std::add_const_t<VarType> *>();
            }
            else {
                return type::get<VarType *>();
            }
        }
        else if constexpr (GetPolicy == prop_policy::get_as_ref_wrapper) {
            if constexpr (SetPolicy == prop_policy::read_only) {
                return type::get<std::reference_wrapper<std::add_const_t<VarType> > >();
            }
            else {
                return type::get<std::reference_wrapper<VarType> >();
            }
        }
        else {
            //prop_policy::return_as_copy
            return type::get<VarType>();
        }
    }

    bool set_value(instance &object, argument &arg) const override {
        if constexpr (std::is_same_v<ParentType, invalid_type>) {
            if constexpr (SetPolicy == prop_policy::set_value) {
                if (arg.is_type<VarType>())
                    return property_accessor<VarType>::set_value([:Var:], arg.get_value<VarType>());
                if constexpr (!std::is_array_v<VarType>) {
                    if constexpr (std::is_default_constructible<VarType>::value) {
                        if (arg.get_type().has_type_converter(rttr::type::get<VarType>())) {
                            variant v = arg.get_value<variant>();
                            if (v) {
                                bool r;
                                VarType rv = v.convert<VarType>(&r);
                                if (r) {
                                    return property_accessor<VarType>::set_value([:Var:], rv);
                                }
                            }
                        }
                    }
                }
                return false;
            }
            else if constexpr (SetPolicy == prop_policy::set_as_ptr) {
                if (arg.is_type<VarType *>()) {
                    [:Var:] = *arg.get_value<VarType *>();
                    return true;
                }
                return false;
            }
            else if constexpr (SetPolicy == prop_policy::set_as_ref_wrapper) {
                if (arg.is_type<std::reference_wrapper<VarType> >())
                    return property_accessor<VarType>::set_value(
                        [:Var:],
                        arg.get_value<std::reference_wrapper<VarType> >().get());
                return false;
            }
            else {
                //prop_policy::read_only
                return false;
            }
        }
        else {
            if constexpr (SetPolicy == prop_policy::set_value) {
                ParentType *ptr = object.try_convert<ParentType>();
                if (ptr && arg.is_type<VarType>()) {
                    return property_accessor<VarType>::set_value(
                        (ptr->[:Var:]),
                        arg.get_value<VarType>());
                }

                if constexpr (!std::is_array_v<VarType>) {
                    if constexpr (std::is_default_constructible<VarType>::value) {
                        if (ptr && arg.get_type().has_type_converter(rttr::type::get<VarType>())) {
                            variant v = arg.get_value<variant>();
                            if (v) {
                                bool r;
                                VarType rv = v.convert<VarType>(&r);
                                if (r) {
                                    return property_accessor<
                                        VarType>::set_value((ptr->[:Var:]), rv);
                                }
                            }
                        }
                    }
                }
                return false;
            }
            else if constexpr (SetPolicy == prop_policy::set_as_ptr) {
                ParentType *ptr = object.try_convert<ParentType>();
                if (ptr && arg.is_type<VarType *>()) {
                    return property_accessor<VarType>::set_value(
                        (ptr->[:Var:]),
                        *arg.get_value<VarType *>());
                }
                else {
                    return false;
                }
            }
            else if constexpr (SetPolicy == prop_policy::set_as_ref_wrapper) {
                ParentType *ptr = object.try_convert<ParentType>();
                if (ptr && arg.is_type<std::reference_wrapper<VarType> >())
                    return property_accessor<VarType>::set_value(
                        (ptr->[:Var:]),
                        arg.get_value<std::reference_wrapper<VarType> >().get());
                else
                    return false;
            }
            else {
                //prop_policy::read_only
                return false;
            }
        }
    }

    variant get_value(instance &object) const override {
        if constexpr (std::is_same_v<ParentType, invalid_type>) {
            if constexpr (GetPolicy == prop_policy::return_as_ptr) {
                auto v = &[:Var:];
                return variant(v);
            }
            else if (GetPolicy == prop_policy::get_as_ref_wrapper) {
                auto v = std::ref([:Var:]);
                return variant(v);
            }
            else {
                //prop_policy::return_as_copy
                auto v = [:Var:];
                return variant(std::move(v));
            }
        }
        else {
            if (ParentType *ptr = object.try_convert<ParentType>()) {
                if constexpr (GetPolicy == prop_policy::return_as_ptr) {
                    variant r(&ptr->[:Var:]);
                    return r;
                }
                else if constexpr (GetPolicy == prop_policy::get_as_ref_wrapper) {
                    variant r(std::ref(ptr->[:Var:]));
                    return r;
                }
                else {
                    variant r(ptr->[:Var:]);
                    return r;
                }
            }
            else {
                return variant();
            }
        }
    }
};

#endif // RTTR_PROPERTY_WRAPPER_OBJECT_H_