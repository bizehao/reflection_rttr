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

#ifndef RTTR_REGISTRATION_H_
#define RTTR_REGISTRATION_H_

#include "rttr/detail/base/core_prerequisites.h"
#include "rttr/policy.h"
#include "rttr/variant.h"
#include "rttr/detail/property/property_wrapper.h"
#include "rttr/detail/method/method_wrapper.h"
#include "rttr/detail/enumeration/enumeration_wrapper.h"
#include "rttr/detail/constructor/constructor_wrapper.h"
#include "rttr/detail/destructor/destructor_wrapper.h"
#include "rttr/detail/metadata/metadata.h"
#include <meta>

namespace rttr {
    namespace detail {
    }

    /*!
 * The \ref registration class is the entry point for the manual registration of reflection information
 * to the type system. It is possible to register *constructors*, *properties*, *methods* and *enumerations*.
 *
 * See following example for a typical usage:
 *
 * Put the \ref RTTR_ENABLE() macro inside the class declaration, when you will inherit from this class,
 * otherwise you can omit the macro.
 * \code{.cpp}
 *  #include <rttr/type>
 *  struct Mesh
 *  {
 *      Mesh();
 *      Mesh(const std::string& name)
 *      Vector3d getPosition() const;
 *      void setPosition(const Vector3d& pos);
 *      enum E_TransformSpace
 *      {
 *          TS_LOCAL,
 *          TS_PARENT,
 *          TS_WORLD
 *      };
 *
 *      void setDirection(const Vector3 &vec, E_TransformSpace ts = TS_LOCAL);
 *      RTTR_ENABLE()
 *  private:
 *      Vector3d _pos;
 *  };
 * \endcode
 *
 * Then in a cpp file, register the constructors, properties and methods of the class \p Mesh.
 *
 * \code{.cpp}
 *  #include "Mesh.cpp"
 *  #include <rttr/registration>
 *  using namespace rttr;
 *
 *  // register the class Mesh before main is called
 *  RTTR_REGISTRATION
 *  {
 *    using namespace rttr;
 *    registration::class_<Mesh>("Mesh")
 *      .constructor<>()
 *      .constructor<const string&>()
 *      .enumeration<E_TransformSpace>("E_TransformSpace")
 *      (
 *          value("TS_LOCAL", TS_LOCAL),
 *          value("TS_PARENT", TS_PARENT),
 *          value("TS_WORLD", TS_WORLD),
 *
 *          metadata("GUI_DESCR", "This enum describes the transformation.")
 *      )
 *      .property("pos", &Mesh::getPosition, &Mesh::setPosition)
 *      (
 *          metadata("GUI_LABEL", "Position."),
 *          metadata("GUI_DESCR", "The position of the mesh."),
 *      )
 *      .method("setDirection", &Mesh::setDirection);
 *  }
 * \endcode
 *
 * \remark See the usage of `()` operator to add additional \ref rttr::metadata(variant, variant) "meta data", \ref policy "policies" or
 *         \ref rttr::value "enum values".
 *
 */
    class RTTR_API registration {
    private:
        static consteval std::string_view get_class_mem_identifier(std::meta::info info) {
            auto rename_tag_vec = std::meta::annotations_of_with_type(info, ^^class_op::rename_tag);
            if (rename_tag_vec.empty()) {
                return std::meta::identifier_of(info);
            }
            else {
                auto op_rename = rename_tag_vec[0];
                return std::meta::extract<class_op::rename_tag>(op_rename)._name;
            }
        }

        template<typename E>
            requires (std::is_enum_v<E>)
        static consteval E get_policy(std::meta::info info) {
            auto policy_tag_vec = std::meta::annotations_of_with_type(info, ^^class_op::policy_tag<E>);
            if (policy_tag_vec.empty()) {
                return E::op_default;
            }
            else {
                auto op_policy = policy_tag_vec[0];
                return std::meta::extract<class_op::policy_tag<E> >(op_policy)._policy;
            }
        }

        static consteval bool check_get_set_fun(std::meta::info GS) {
            if (std::meta::is_function(GS)) {
                if (std::meta::is_class_member(GS)) {
                    if (std::meta::is_static_member(GS)) {
                        return true;
                    }
                    else {
                        return false;
                    }
                }
                else {
                    return true;
                }
            }
            else {
                return false;
            }
        }

        static consteval std::string_view get_full_identifier(std::meta::info entry) {
            if (!std::meta::has_identifier(entry)) {
                throw std::invalid_argument{
                    std::string{"invalid identifier: "} + std::meta::display_string_of(entry)
                };
            }

            if (std::meta::has_parent(entry)) {
                auto parent_scope = std::meta::parent_of(entry);
                if (parent_scope != ^^::) {
                    std::string full_name = std::string(std::meta::display_string_of(parent_scope)) +
                                            "::" + std::string(std::meta::identifier_of(entry));
                    return std::string_view{std::define_static_string(full_name)};
                }
            }

            return std::meta::identifier_of(entry);
        }

        template<std::meta::info info>
        static std::vector<detail::metadata> get_target_metadata() {
            std::vector<detail::metadata> result;
            static constexpr auto metadata_tag_vec = std::define_static_array(
                std::meta::annotations_of(info));
            template for (constexpr auto it: metadata_tag_vec) {
                if constexpr (constexpr auto t = std::meta::type_of(it);
                    std::meta::has_template_arguments(t) && std::meta::template_of(t) == ^^metadata_tag) {
                    using T = [:t:];
                    constexpr auto obj = std::meta::extract<T>(it);
                    constexpr auto key = obj.key;
                    auto r = std::ranges::find_if(result, [](detail::metadata &it) { return it.get_key() == key; });
                    if (r == result.end()) {
                        auto value = obj.data.reduction();
                        result.emplace_back(key, value);
                    }
                    else {
                        //重复key会被排除
                    }
                }
            }
            return result;
        }

    public:
        /*!
     * The \ref class_ is used to register classes to RTTR.
     */
        template<std::meta::info ClassRefl>
            requires (std::meta::is_class_type(ClassRefl))
        static void do_class(std::string_view name = get_full_identifier(ClassRefl)) {
            using Clazz = [:ClassRefl:];
            auto t = type::get<Clazz>();
            detail::type_register::custom_name(t, name);

            if (auto metadata = get_target_metadata<ClassRefl>(); !metadata.empty()) {
                detail::type_register::metadata(t, std::move(metadata));
            }

            //unprivileged unchecked
            constexpr auto access_context = std::meta::access_context::unprivileged();

            template for (constexpr std::meta::info member: std::define_static_array(
                std::meta::nonstatic_data_members_of(ClassRefl, access_context))) {
                if constexpr (std::meta::has_identifier(member)) {
                    if constexpr (std::meta::annotations_of_with_type(member, ^^class_op::ignore_tag).
                        empty()) {
                        do_property<member, get_policy<policy::prop>(member)>(
                            get_class_mem_identifier(member));
                    }
                }
            }

            template for (constexpr std::meta::info member: std::define_static_array(
                std::meta::static_data_members_of(ClassRefl, access_context))) {
                if constexpr (std::meta::has_identifier(member)) {
                    if constexpr (std::meta::annotations_of_with_type(member, ^^class_op::ignore_tag).
                        empty()) {
                        do_property<member, get_policy<policy::prop>(member)>(
                            get_class_mem_identifier(member));
                    }
                }
            }

            //check ctor policy,  op_as_object and copy_constructible_type
            constexpr auto check_ctor_policy = [](detail::ctor_policy policy) static consteval {
                if (policy == detail::ctor_policy::as_object && !std::meta::is_copy_constructible_type(
                        ClassRefl)) {
                    throw std::invalid_argument{
                        std::string{
                            "is not copy_constructible_type, only policy need to as_raw_ptr or as_std_shared_ptr of "
                        }
                        + std::meta::display_string_of(ClassRefl)
                    };
                }
            };

            template for (constexpr std::meta::info member: std::define_static_array(
                std::meta::members_of(ClassRefl, access_context))) {
                if constexpr (std::meta::is_function(member) && !std::meta::is_deleted(member)) {
                    if constexpr (std::meta::annotations_of_with_type(member, ^^class_op::ignore_tag).
                        empty()) {
                        if constexpr (std::meta::is_constructor(member)) {
                            constexpr auto policy = policy::transform(get_policy<policy::ctor>(member));
                            check_ctor_policy(policy);
                            detail::store_item<Clazz>(
                                std::make_unique<detail::constructor_wrapper<member, policy> >(
                                    get_target_metadata<member>()));
                        }
                        else if constexpr (std::meta::is_destructor(member)) {
                            detail::store_item<Clazz>(
                                std::make_unique<detail::destructor_wrapper<member> >(
                                    get_target_metadata<member>()));
                        }
                        else if constexpr (std::meta::is_operator_function(member)) {
                            constexpr auto operator_name = []() static consteval {
                                auto ss = std::meta::display_string_of(member);
                                auto b = ss.find("operator") + 8;
                                auto e = ss.find_last_of("(");
                                return ss.substr(b, e - b);
                            }();

                            if constexpr(!std::meta::is_defaulted(member))
                            {
                                do_method<member, get_policy<policy::method>(member)>(operator_name);
                            }
                        }
                        else if constexpr (std::meta::is_conversion_function(member)) {
                            constexpr auto rt = std::meta::return_type_of(member);
                            auto convert_fun = [](const Clazz &obj, bool &ok) static -> [:rt:] {
                                ok = true;
                                return static_cast<[:rt:]>(obj);
                            };
                            type::register_converter_func(convert_fun);
                        }
                        else {
                            //normal mem function
                            //排除限定符为&&的成员函数
                            if constexpr (!detail::is_mem_fun_rv_qualifier < typename [:
                                              std::meta::type_of(member):] >) {
                                do_method<member, get_policy<policy::method>(member)>(
                                    get_class_mem_identifier(member));
                            }
                        }
                    }
                }
            }
        }

        /*!
     * \brief Register a global property with read write access.
     *
     * \param name  The name of the property.
     * \param acc   The accessor to the property; a pointer to a variable.
     *
     * \remark The name of the property has to be unique, otherwise it will not be registered.
     *
     * \see property, type::get_global_property(),
     *                \ref type::get_property_value(string_view) "type::get_property_value()",
     *                \ref type::set_property_value(string_view, argument) "type::set_property_value"
     *
     * \return A \ref bind object, in order to chain more calls.
     */
        //global var; static class member; non-static class member
        template<std::meta::info Var, policy::prop PolicyOp = policy::prop::op_default>
            requires (!detail::is_lambda(Var) && (
                          std::meta::is_variable(Var) || std::meta::is_nonstatic_data_member(Var)))
        static void do_property(std::string_view name = get_full_identifier(Var)) {
            using T = [:std::meta::is_class_member(Var)
                            ? std::meta::parent_of(Var)
                            : ^^detail::invalid_type:];
            constexpr auto pair = policy::transform(PolicyOp);
            if constexpr (std::meta::is_const(Var)) {
                detail::store_item<T>(
                    std::make_unique<detail::property_wrapper_object<
                        Var, pair.first, detail::prop_policy::read_only> >(
                        name,
                        get_target_metadata<Var>()));
            }
            else {
                detail::store_item<T>(
                    std::make_unique<detail::property_wrapper_object<
                        Var, pair.first, pair.second> >(name, get_target_metadata<Var>()));
            }
        }

        /*!
     * \brief Register a global read only property.
     *
     * \param name  The name of the property.
     * \param acc   The accessor to the property; this can be a pointer to a variable,
     *              a pointer to a function or a std::function.
     *
     * \remark The name of the property has to be unique, otherwise it will not be registered.
     *
     * \see property, type::get_global_property(),
     *                \ref type::get_property_value(string_view) "type::get_property_value()",
     *                \ref type::set_property_value(string_view, argument) "type::set_property_value"
     *
     * \return A \ref bind object, in order to chain more calls.
     */
        template<std::meta::info Var, policy::prop PolicyOp = policy::prop::op_default>
            requires (!detail::is_lambda(Var) && (
                          std::meta::is_variable(Var) || std::meta::is_nonstatic_data_member(Var)))
        static void do_property_readonly(std::string_view name = get_full_identifier(Var)) {
            using T = [:std::meta::is_class_member(Var)
                            ? std::meta::parent_of(Var)
                            : ^^detail::invalid_type:];
            constexpr auto pair = policy::transform(PolicyOp, true);
            detail::store_item<T>(
                std::make_unique<detail::property_wrapper_object<Var, pair.first, pair.second> >(
                    name,
                    get_target_metadata<Var>()));
        }

        /*!
     * \brief Register a property to this class.
     *
     * \param name   The name of the property.
     * \param getter The getter accessor to the property; this can be a pointer to a function or a std::function.
     * \param setter The setter accessor to the property; this can be a pointer to a function or a std::function.
     *
     * \remark The name of the property has to be unique, otherwise it will not be registered.
     *
     * \see property, type::get_global_property(),
     *                \ref type::get_property_value(string_view) "type::get_property_value()",
     *                \ref type::set_property_value(string_view, argument) "type::set_property_value"
     *
     * \return A \ref bind object, in order to chain more calls.
     */
        template<std::meta::info GetVar, std::meta::info SetVar, policy::prop PolicyOp =
                policy::prop::op_default>
            requires ((check_get_set_fun(GetVar) || detail::is_lambda(GetVar)) && (
                          check_get_set_fun(SetVar) || detail::is_lambda(SetVar)))
        static void do_property(std::string_view name) {
            constexpr auto pair = policy::transform(PolicyOp);

            auto l_metadata= get_target_metadata<GetVar>();
            auto r_metadata = get_target_metadata<SetVar>();
            l_metadata.insert(l_metadata.end(), r_metadata.begin(), r_metadata.end());

            detail::store_item<detail::invalid_type>(
                std::make_unique<detail::property_wrapper_function<
                    GetVar, SetVar, pair.first, pair.second> >(name, l_metadata));
        }

        template<std::meta::info GetVar, policy::prop PolicyOp = policy::prop::op_default>
            requires (check_get_set_fun(GetVar) || detail::is_lambda(GetVar))
        static void do_property_readonly(std::string_view name) {
            constexpr auto pair = policy::transform(PolicyOp, true);
            detail::store_item<detail::invalid_type>(
                std::make_unique<detail::property_wrapper_function<
                    GetVar, ^^void, pair.first, pair.second> >(name, get_target_metadata<GetVar>()));
        }

        /*!
     * \brief Register a method to this class.
     *
     * \param name      The name of the method.
     * \param function  The function accessor to this method; this can be a member function, a function or a std::function.
     *
     * \remark The method name does *not* have to be unique.
     *
     * \see method, type::get_global_method(),
     *              \ref type::invoke(string_view, std::vector<argument>) "type::invoke()"
     *
     * \return A \ref bind object, in order to chain more calls.
     */
        template<std::meta::info Fun, policy::method PolicyOp = policy::method::op_default>
            requires (std::meta::is_function(Fun) || detail::is_lambda(Fun))
        static void do_method(std::string_view name = get_full_identifier(Fun)) {
            constexpr auto p = policy::transform(PolicyOp);
            using T = [:std::meta::is_class_member(Fun)
                            ? std::meta::parent_of(Fun)
                            : ^^detail::invalid_type:];
            detail::store_item<T>(
                std::make_unique<detail::method_wrapper<Fun, p> >(name, get_target_metadata<Fun>()));
        }

        //默认参数和参数名字会失效
        template<auto Fun, policy::method PolicyOp = policy::method::op_default, typename MetaT = void>
            requires (detail::is_function_ptr<decltype(Fun)>::value)
        static void do_method_overload(std::string_view name,
                                       std::in_place_type_t<MetaT> = std::in_place_type_t<MetaT>{}) {
            constexpr auto p = policy::transform(PolicyOp);
            using Cur_Fun = detail::function_traits<decltype(Fun)>;
            static auto fun_lambda_wrapper = []<std::size_t... Is>(std::index_sequence<Is...>) static {
                return [](std::tuple_element_t<Is, typename Cur_Fun::arg_types>... args) {
                    return Fun(std::forward<std::tuple_element_t<Is, typename Cur_Fun::arg_types> >(args)...);
                };
            }(std::make_index_sequence<Cur_Fun::arg_count>{});

            detail::store_item<detail::invalid_type>(
                std::make_unique<detail::method_wrapper<^^fun_lambda_wrapper, p> >(
                    name,
                    get_target_metadata<^^MetaT>()));
        }

        /*!
     * \brief Register a global enumeration of type \p Enum_Type
     *
     * \param name The name of the enumeration.
     *
     * \remark The name of the enumeration has to be unique, otherwise it will not be registered.
     *
     * \see enumeration, type::get_enumeration()
     *
     * \return A \ref bind object, in order to chain more calls.
     */
        template<std::meta::info Enum>
            requires (std::meta::is_enum_type(Enum))
        static void do_enumeration(std::string_view name = get_full_identifier(Enum)) {
            auto t = type::template get<typename [:Enum:]>();
            detail::type_register::custom_name(t, name);
            detail::store_item<detail::invalid_type>(
                std::make_unique<detail::enumeration_wrapper<Enum> >(get_target_metadata<Enum>()));
        }
    };

    /////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

    /*!
 * \brief This is a helper function to register overloaded functions.
 *
 * Use it like following:
 * \code{.cpp}
 *
 * #include <rttr/registration>
 * #include <cmath>
 * using namespace rttr;
 *
 * RTTR_REGISTRATION
 * {
 *      registration::method("pow", select_overload<float(float, float)>(&pow));
 * }
 * \endcode
 *
 * \remark The method **cannot** be used with *MSVC x86* compiler, because of the different calling convention for global- and member-functions.
 * As workaround you have to explicitly cast to the function pointer:
 *
 * \code{.cpp}
 *
 * RTTR_REGISTRATION
 * {
 *      registration::method("pow", static_cast<float(*)(float, float)>(&pow));
 * }
 * \endcode
 *
 */
    template<typename Signature>
    constexpr Signature *select_overload(Signature *func) {
        return func;
    }

    /////////////////////////////////////////////////////////////////////////////////////////
} // end namespace rttr

#include "rttr/detail/registration/registration_impl.h"

#endif // RTTR_REGISTRATION_H_