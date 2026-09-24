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

#include <iostream>
#include <memory>
#include <functional>

#include <catch/catch.hpp>

using namespace rttr;


/////////////////////////////////////////////////////////////////////////////////////////
// test derived properties

namespace ns_property {
    struct top {
        virtual ~top() {
        }

        top() : _p1(12) {
        }

        [[=rttr::class_op::rename("top")]]
        int _p1;

        RTTR_ENABLE()
    };

    /////////////////////////////////////////////////////////////////////////////////////////

    struct left : virtual top {
        left() : _p2(true) {
        }

        ~left() override = default;

        [[=rttr::class_op::rename("left")]]
        bool _p2;

        RTTR_ENABLE()
    };

    /////////////////////////////////////////////////////////////////////////////////////////

    struct right : virtual top {
        right() : _p3(true) {
        }

        ~right() override = default;

        [[=rttr::class_op::rename("right")]]
        bool _p3;

        RTTR_ENABLE()
    };

    /////////////////////////////////////////////////////////////////////////////////////////

    struct right_2 {
        virtual ~right_2() {
        }

        right_2() : _p4(true) {
        }

        [[=rttr::class_op::rename("right_2")]]
        bool _p4;

        RTTR_ENABLE()
    };

    /////////////////////////////////////////////////////////////////////////////////////////

    struct bottom : left, right, right_2 {
        bottom() : _p5(23.0) {
        }

        ~bottom() override = default;

        [[=rttr::class_op::rename("bottom")]]
        double _p5;

        RTTR_ENABLE()
    };
}

/////////////////////////////////////////////////////////////////////////////////////////

struct base_prop_not_registered {
    base_prop_not_registered() : value(100) {
    }

    int value;
};

struct derived_registered_prop : base_prop_not_registered {
};

/////////////////////////////////////////////////////////////////////////////////////////

struct base_class_with_props {
    base_class_with_props() : value(100) {
    }

    int value;

    RTTR_ENABLE()
};

struct derived_class_without_registered_props : base_class_with_props {
    RTTR_ENABLE()
};

/////////////////////////////////////////////////////////////////////////////////////////

static double g_name;

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

RTTR_REGISTRATION {
    registration::do_class < ^^ns_property::top > ("ns_property::top");;

    registration::do_class < ^^ns_property::left > ("ns_property::left");

    registration::do_class < ^^ns_property::right > ("ns_property::right");

    registration::do_class < ^^ns_property::right_2 > ("ns_property::right_2");

    registration::do_class < ^^ns_property::bottom > ("ns_property::bottom");

    registration::do_class < ^^base_prop_not_registered > ("base_prop_not_registered");
    registration::do_class < ^^derived_registered_prop > ("derived_registered_prop");


    registration::do_class < ^^base_class_with_props > ("base_class_with_props");

    registration::do_class < ^^derived_class_without_registered_props > ("derived_class_without_registered_props");
}

/////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE (
"property - get_properties()"
,
"[property]"
)
{
    type t = type::get<ns_property::bottom>();
    auto range = t.get_properties();
    REQUIRE(range.size() == 5);

    std::vector<property> props(range.begin(), range.end());
    REQUIRE(props.size() == 5);

    CHECK(props[0].get_name() == "top");
    CHECK(props[1].get_name() == "left");
    CHECK(props[2].get_name() == "right");
    CHECK(props[3].get_name() == "right_2");
    CHECK(props[4].get_name() == "bottom");
}

/////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE (
"property - class - inheritance - invoke"
,
"[property]"
)
{
    type t = type::get<ns_property::bottom>();


    ns_property::bottom instance;
    ns_property::top& top = instance;
    // try access from top instance a property in the most derived class (bottom)
    property base_prop = t.get_property("top");

    variant ret = base_prop.get_value(top);
    REQUIRE(ret.is_type<int>() == true);
    CHECK(ret.get_value<int>() == 12);
    // try to change the value
    base_prop.set_value(top, 2000);
    CHECK(instance._p1 == 2000);

    // and now the other way around, from bottom a top property
    property bottom_prop = t.get_property("bottom");
    ret = bottom_prop.get_value(instance);
    REQUIRE(ret.is_type<double>() == true);
    CHECK(ret.get_value<double>() == 23.0);
    // try to change the value
    bottom_prop.set_value(top, 42.0);
    CHECK(instance._p5 == 42.0);
}

/////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE (
"property - base class not registered"
,
"[property]"
)
{
    type t_prop = type::get<derived_registered_prop>();
    property prop = t_prop.get_property("value");
    derived_registered_prop obj;

    auto ret = prop.set_value(obj, 23);

    CHECK(ret == true);
    CHECK(obj.value == 23);

    auto base_type = type::get<base_prop_not_registered>();

    CHECK(t_prop.is_derived_from(base_type) == true);

    auto range = base_type.get_derived_classes();

    REQUIRE(range.size() == 1);
    CHECK(*range.begin() == t_prop);
}

/////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE (
"property - check inheritance of probs"
,
"[property]"
)
{
    // base class has registered properties, the derived class not
    type t_prop = type::get<derived_class_without_registered_props>();
    auto prop_range = t_prop.get_properties();
    REQUIRE(prop_range.size() == 1);

    CHECK((*prop_range.begin()).get_name() == "value");
}

/////////////////////////////////////////////////////////////////////////////////////////
