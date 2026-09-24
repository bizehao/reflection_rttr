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

#include <rttr/registration>
#include <catch/catch.hpp>

using namespace rttr;

struct ctor_default_arg_invoke_test {
    ctor_default_arg_invoke_test(int a) {
    };

    ctor_default_arg_invoke_test(int a, int b) {
    };

    ctor_default_arg_invoke_test(int a, int b, int c) {
    };

    ctor_default_arg_invoke_test(int a, int b, int c, int d) {
    };

    ctor_default_arg_invoke_test(int a, int b, int c, int d, int e) {
    };

    ctor_default_arg_invoke_test(int a, int b, int c, int d, int e, int f) {
    };

    ctor_default_arg_invoke_test(int a, int b, int c, int d, int e, int f, int g) {
    };
};

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

RTTR_REGISTRATION {
    registration::do_class<^^ctor_default_arg_invoke_test>();
}

////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE (
"constructor - default argument test (real ctor)"
,
"[constructor]"
)
{
    struct ctor_default_arg_test {
        [[=class_op::ctor_policy(policy::ctor::op_as_std_shared_ptr)]]
        ctor_default_arg_test(int value1, bool value2 = true, const std::string& text = std::string("default text"))
        {
        }
    };

    registration::do_class<^^ctor_default_arg_test>();

    type t = type::get<ctor_default_arg_test>();
    REQUIRE(t.is_valid() == true);

    auto range = t.get_constructors();
    std::vector<constructor> ctor_list(range.cbegin(), range.cend());
    REQUIRE(ctor_list.size() >= 1);

    constructor ctor = ctor_list[0];
    using create_type = std::shared_ptr<ctor_default_arg_test>;
    // too less arguments for invoke
    variant var = ctor.invoke();
    CHECK(var.is_valid() == false);

    // too much arguments
    var = ctor.invoke(23, true, std::string("default text"), 43.0);
    CHECK(var.is_valid() == false);

    // using 2 default arguments
    var = ctor.invoke(23);
    REQUIRE(var.is_type<create_type>() == true);
    CHECK(var.get_value<create_type>().get() != nullptr);

    // using 1 default arguments
    var = ctor.invoke(23, true);
    REQUIRE(var.is_type<create_type>() == true);
    CHECK(var.get_value<create_type>().get() != nullptr);

    // using NO default arguments
    var = ctor.invoke(23, true, std::string("default text"));
    REQUIRE(var.is_type<create_type>() == true);
    CHECK(var.get_value<create_type>().get() != nullptr);
}

////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE (
"constructor - default argument test (func ctor)"
,
"[constructor]"
)
{
    struct ctor_default_arg_test {
        [[=class_op::ctor_policy(policy::ctor::op_as_std_shared_ptr)]]
        ctor_default_arg_test(int value1, bool value2 = true, const std::string& text = std::string("default text"))
        {
        }
    };

    registration::do_class<^^ctor_default_arg_test>();

    type t = type::get<ctor_default_arg_test>();
    REQUIRE(t.is_valid() == true);

    auto range = t.get_constructors();
    std::vector<constructor> ctor_list(range.cbegin(), range.cend());
    REQUIRE(ctor_list.size() >= 1);
    constructor ctor = ctor_list[0];
    using create_type = std::shared_ptr<ctor_default_arg_test>;
    // too less arguments for invoke
    variant var = ctor.invoke();
    CHECK(var.is_valid() == false);

    // too much arguments
    var = ctor.invoke(23, true, std::string("default text"), 43.0);
    CHECK(var.is_valid() == false);

    // using 2 default arguments
    var = ctor.invoke(23);
    REQUIRE(var.is_type<create_type>() == true);
    CHECK(var.get_value<create_type>().get() != nullptr);

    // using 1 default arguments
    var = ctor.invoke(23, true);
    REQUIRE(var.is_type<create_type>() == true);
    CHECK(var.get_value<create_type>().get() != nullptr);

    // using NO default arguments
    var = ctor.invoke(23, true, std::string("default text"));
    REQUIRE(var.is_type<create_type>() == true);
    CHECK(var.get_value<create_type>().get() != nullptr);
}

/////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE (
"constructor - default argument test (invoke variadic; real ctor)"
,
"[constructor]"
)
{
    struct ctor_default_arg_test {
        [[=class_op::ctor_policy(policy::ctor::op_as_std_shared_ptr)]]
        ctor_default_arg_test(int value1, bool value2 = true, const std::string& text = std::string("default text"))
        {
        }
    };

    registration::do_class<^^ctor_default_arg_test>();

    type t = type::get<ctor_default_arg_test>();
    REQUIRE(t.is_valid() == true);

    auto range = t.get_constructors();
    std::vector<constructor> ctor_list(range.cbegin(), range.cend());
    REQUIRE(ctor_list.size() >= 1);
    constructor ctor = ctor_list[0];
    using create_type = std::shared_ptr<ctor_default_arg_test>;
    // too less arguments for invoke
    variant var = ctor.invoke();
    CHECK(var.is_valid() == false);
    var = ctor.invoke_variadic({23, true, std::string("default text"), 43.0});

    CHECK(var.is_valid() == false);

    // using 2 default arguments
    var = ctor.invoke_variadic({23});

    REQUIRE(var.is_type<create_type>() == true);
    CHECK(var.get_value<create_type>().get() != nullptr);

    // using 1 default arguments
    var = ctor.invoke_variadic({23, true});

    REQUIRE(var.is_type<create_type>() == true);
    CHECK(var.get_value<create_type>().get() != nullptr);

    // using NO default arguments
    var = ctor.invoke_variadic({23, true, std::string("default text")});

    REQUIRE(var.is_type<create_type>() == true);
    CHECK(var.get_value<create_type>().get() != nullptr);
}

/////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE (
"constructor - default argument test (invoke variadic; func ctor)"
,
"[constructor]"
)
{
    struct ctor_default_arg_test {
        [[=class_op::ctor_policy(policy::ctor::op_as_std_shared_ptr)]]
        ctor_default_arg_test(int value1, bool value2 = true, const std::string& text = std::string("default text"))
        {
        }
    };

    registration::do_class<^^ctor_default_arg_test>();
    type t = type::get<ctor_default_arg_test>();
    REQUIRE(t.is_valid() == true);

    auto range = t.get_constructors();
    std::vector<constructor> ctor_list(range.cbegin(), range.cend());
    REQUIRE(ctor_list.size() >= 1);
    constructor ctor = ctor_list[0];
    using create_type = std::shared_ptr<ctor_default_arg_test>;
    // too less arguments for invoke
    variant var = ctor.invoke();
    CHECK(var.is_valid() == false);
    // too much arguments
    var = ctor.invoke_variadic({23, true, std::string("default text"), 43.0});
    CHECK(var.is_valid() == false);

    // using 2 default arguments
    var = ctor.invoke_variadic({23});

    REQUIRE(var.is_type<create_type>() == true);
    CHECK(var.get_value<create_type>().get() != nullptr);

    // using 1 default arguments
    var = ctor.invoke_variadic({23, true});
    REQUIRE(var.is_type<create_type>() == true);
    CHECK(var.get_value<create_type>().get() != nullptr);

    // using NO default arguments
    var = ctor.invoke_variadic({23, true, std::string("default text")});
    REQUIRE(var.is_type<create_type>() == true);
    CHECK(var.get_value<create_type>().get() != nullptr);
}

////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE (
"constructor - default argument via type (real ctor)"
,
"[constructor]"
)
{
    struct ctor_default_arg_test {
        [[=class_op::ctor_policy(policy::ctor::op_as_std_shared_ptr)]]
        ctor_default_arg_test(int value1, bool value2, const std::string& text = std::string("default text"))
        {
        }
        [[=class_op::ctor_policy(policy::ctor::op_as_std_shared_ptr)]]
        ctor_default_arg_test(int value1 = 23)
        {
        }
    };

    registration::do_class<^^ctor_default_arg_test>();

    type t = type::get<ctor_default_arg_test>();
    REQUIRE(t.is_valid() == true);

    CHECK(t.create().is_valid()     == true);
    CHECK(t.create({23}).is_valid() == true); // this will not invoke the same ctor like above, the first registered
    CHECK(t.create({23, false}).is_valid() == true);
    CHECK(t.create({23, false, std::string("text")}).is_valid() == true);
    // too much arguments
    CHECK(t.create({23, false, std::string("text"), 42}).is_valid() == false);
}

/////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE (
"constructor - invoke ctor with defaults valid"
,
"[constructor]"
)
{
    auto range = type::get<ctor_default_arg_invoke_test>().get_constructors();
    std::vector<constructor> ctor_list(range.cbegin(), range.cend());
    REQUIRE(ctor_list.size() >= 7);

    CHECK(ctor_list[0].invoke(1).is_valid() == true);
    CHECK(ctor_list[1].invoke(1, 2).is_valid() == true);
    CHECK(ctor_list[2].invoke(1, 2, 3).is_valid() == true);
    CHECK(ctor_list[3].invoke(1, 2, 3, 4).is_valid() == true);
    CHECK(ctor_list[4].invoke(1, 2, 3, 4, 5).is_valid() == true);
    CHECK(ctor_list[5].invoke(1, 2, 3, 4, 5, 6).is_valid() == true);
    CHECK(ctor_list[6].invoke_variadic({1, 2, 3, 4, 5, 6, 7}).is_valid() == true);
}

////////////////////////////////////////////////////////////////////////////////////////
