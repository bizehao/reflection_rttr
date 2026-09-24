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

#ifndef RTTR_REGISTRATION_IMPL_H_
#define RTTR_REGISTRATION_IMPL_H_

#include "rttr/detail/base/core_prerequisites.h"
#include "rttr/detail/constructor/constructor_wrapper.h"
#include "rttr/detail/destructor/destructor_wrapper.h"
#include "rttr/detail/enumeration/enumeration_wrapper.h"
#include "rttr/detail/method/method_wrapper.h"
#include "rttr/detail/property/property_wrapper.h"
#include "rttr/detail/misc/misc_type_traits.h"
#include "rttr/detail/misc/utility.h"
#include "rttr/detail/type/type_register.h"
#include "rttr/detail/registration/registration_state_saver.h"
#include "rttr/policy.h"
#include "rttr/enumeration.h"

#include <string>
#include <vector>

namespace rttr {
    /////////////////////////////////////////////////////////////////////////////////////////
} // end namespace rttr

#define RTTR_REGISTRATION                                                           \
static void rttr_auto_register_reflection_function_();                              \
namespace                                                                           \
{                                                                                   \
    struct rttr__auto__register__                                                   \
    {                                                                               \
        rttr__auto__register__()                                                    \
        {                                                                           \
            rttr_auto_register_reflection_function_();                              \
        }                                                                           \
    };                                                                              \
}                                                                                   \
static const rttr__auto__register__ RTTR_CAT(auto_register__, __LINE__);            \
static void rttr_auto_register_reflection_function_()


#if RTTR_COMPILER == RTTR_COMPILER_MSVC
#define RTTR_PLUGIN_REGISTRATION RTTR_REGISTRATION
#else
#define RTTR_PLUGIN_REGISTRATION                                                    \
static void rttr_auto_register_reflection_function_() RTTR_DECLARE_PLUGIN_CTOR;     \
static void rttr_auto_unregister_reflection_function() RTTR_DECLARE_PLUGIN_DTOR;    \
                                                                                    \
static void rttr_auto_unregister_reflection_function()                              \
{                                                                                   \
    rttr::detail::get_registration_manager().unregister();                          \
}                                                                                   \
static void rttr_auto_register_reflection_function_()
#endif


#endif // RTTR_REGISTRATION_IMPL_H_
