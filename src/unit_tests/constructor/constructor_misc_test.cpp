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

struct ctor_misc_test {
    [[=class_op::ctor_policy(policy::ctor::op_as_raw_ptr)]]
    ctor_misc_test (std::in_place_index_t<0>){}

    [[=class_op::ctor_policy(policy::ctor::op_as_std_shared_ptr)]]
    ctor_misc_test (std::in_place_index_t<1>){}

    [[=class_op::ctor_policy(policy::ctor::op_as_object)]]
    ctor_misc_test (std::in_place_index_t<2>){}

    ctor_misc_test() {
    }

    ctor_misc_test(int value) {
    }
};

struct not_copyable_ctor {
    [[=class_op::ctor_policy(policy::ctor::op_as_raw_ptr)]]
    not_copyable_ctor (std::in_place_index_t<0>){}

    [[=class_op::ctor_policy(policy::ctor::op_as_std_shared_ptr)]]
    not_copyable_ctor (std::in_place_index_t<1>){}

    // [[=class_op::ctor_policy(policy::ctor::op_as_object)]]
    // not_copyable_ctor(std::in_place_index_t<2>){}

private:
    not_copyable_ctor(const not_copyable_ctor &);
};

enum class E_MetaData {
    SCRIPTABLE = 0,
    TOOL_TIP = 1,
    DESCRIPTION = 2
};

RTTR_REGISTRATION {
    registration::do_class<^^ctor_misc_test>();
    registration::do_class<^^not_copyable_ctor>();
    registration::do_enumeration<^^E_MetaData>();
}

////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE (
"constructor - default ctor binding type"
,
"[constructor]"
)
{
    variant var = type::get<not_copyable_ctor>().create({std::in_place_index<1>});

    CHECK(var.get_type() == type::get<std::shared_ptr<not_copyable_ctor>>());
}

////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE (
"constructor - get_instantiated_type"
,
"[constructor]"
)
{
    registration::do_class<^^ctor_misc_test>();

    auto range = type::get<ctor_misc_test>().get_constructors();
    std::vector<constructor> ctor_list(range.cbegin(), range.cend());
    REQUIRE(ctor_list.size() >= 4);

    CHECK(ctor_list[0].get_instantiated_type() == type::get<ctor_misc_test*>());
    CHECK(ctor_list[1].get_instantiated_type() == type::get<std::shared_ptr<ctor_misc_test>>());
    CHECK(ctor_list[2].get_instantiated_type() == type::get<ctor_misc_test>());
    CHECK(ctor_list[3].get_instantiated_type() == type::get<ctor_misc_test>());
    //negative test
    CHECK(type::get_by_name("").get_constructor().get_instantiated_type().is_valid() == false);
}

////////////////////////////////////////////////////////////////////////////////////////


TEST_CASE (
"constructor - get_signature"
,
"[constructor]"
)
{
    registration::do_class<^^ctor_misc_test>();
    auto range = type::get<ctor_misc_test>().get_constructors();
    std::vector<constructor> ctor_list(range.cbegin(), range.cend());
    REQUIRE(ctor_list.size() >= 5);

    CHECK(ctor_list[3].get_signature() == "ctor_misc_test( )");
    CHECK(ctor_list[4].get_signature() == "ctor_misc_test( int )");

    //negative test
    CHECK(type::get_by_name("").get_constructor().get_signature() == "");
}

////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE (
"constructor - get_parameter_infos"
,
"[constructor]"
)
{
    registration::do_class<^^ctor_misc_test>();

    auto range = type::get<ctor_misc_test>().get_constructors();
    std::vector<constructor> ctor_list(range.cbegin(), range.cend());
    REQUIRE(ctor_list.size() >= 5);
    constructor ctor = ctor_list[4];

    REQUIRE(ctor.get_parameter_infos().size() == 1);
    auto info = *ctor.get_parameter_infos().begin();
    CHECK(info.get_type() == type::get<int>());
    CHECK(info.has_default_value() == false);
}

////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE (
"ctor - get_declaring_type"
,
"[constructor]"
)
{
    registration::do_class<^^ctor_misc_test>();

    auto range = type::get<ctor_misc_test>().get_constructors();
    std::vector<constructor> ctor_list(range.cbegin(), range.cend());
    REQUIRE(ctor_list.size() >= 5);

    CHECK(ctor_list[0].get_declaring_type() == type::get<ctor_misc_test>());

    //negative test
    CHECK(type::get_by_name("").get_constructor().get_declaring_type().is_valid() == false);
}

////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE (
"constructor - compare - type"
,
"[constructor]"
)
{

    registration::do_class<^^ctor_misc_test>();

    constructor ctor1 = type::get<ctor_misc_test>().get_constructor();
    constructor ctor2 = type::get<ctor_misc_test>().get_constructor();

    CHECK(ctor1 == ctor2);

    auto range = type::get<ctor_misc_test>().get_constructors();
    std::vector<constructor> ctor_list(range.cbegin(), range.cend());
    REQUIRE(ctor_list.size() >= 2);

    CHECK(ctor_list[0] != ctor_list[1]);
}

/////////////////////////////////////////////////////////////////////////////////////////
