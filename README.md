[![Version](https://badge.fury.io/gh/rttrorg%2Frttr.svg)](https://github.com/rttrorg/rttr/releases/latest)
[![Travis status](https://travis-ci.org/rttrorg/rttr.svg?branch=master)](https://travis-ci.org/rttrorg/rttr)
[![Appveyor status](https://ci.appveyor.com/api/projects/status/github/rttrorg/rttr?svg=true&branch=master)](https://ci.appveyor.com/project/acki-m/rttr)
[![Coverage Status](https://coveralls.io/repos/rttrorg/rttr/badge.svg?branch=master&service=github)](https://coveralls.io/github/rttrorg/rttr)
[![CII Best Practices](https://bestpractices.coreinfrastructure.org/projects/783/badge)](https://bestpractices.coreinfrastructure.org/projects/783)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/9821799170644782ac8d7885d393e686)](https://www.codacy.com/app/acki-m/rttr?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=rttrorg/rttr&amp;utm_campaign=Badge_Grade)
[![Documentation](https://img.shields.io/badge/docs-latest-blue.svg)](http://www.rttr.org/doc/master/classes.html)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](https://raw.githubusercontent.com/rttrorg/rttr/master/LICENSE.txt)
[![Donate](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=JQ65KGGCSUZMS)

<a target="_blank"> 基于 C++26 静态反射的 rttr</a>
==

RTTR
====
> C++ 反射库（Reflection Library）

RTTR 是 *Run Time Type Reflection*（运行时类型反射）的缩写。
它描述了计算机程序在运行时检查和修改对象的能力。同时它也是这个库本身的名称，该库使用 C++ 编写并以开源形式发布。
更多信息请访问：<a target="_blank" href="http://www.rttr.org">www.rttr.org</a>

----------

本仓库是对原始 RTTR 项目的持续修改与演进：

- **原始 RTTR 仓库（C++11）**：<https://github.com/rttrorg/rttr>

----------

使用方法
----------

### 手动注册（Manual registration）

```cpp
#include <rttr/registration>
using namespace rttr;

struct MyStruct {
   MyStruct() {};
   void func(double) {};
   int data; 
};

RTTR_REGISTRATION
{
    registration::do_class<^^MyStruct>();
}
```

### 遍历成员（Iterate over members）

```cpp
type t = type::get<MyStruct>();
for (auto& prop : t.get_properties())
    std::cout << "name: " << prop.get_name();

for (auto& meth : t.get_methods())
    std::cout << "name: " << meth.get_name();
```

### 构造类型（Constructing types）

```cpp
type t = type::get_by_name("MyStruct");
variant var = t.create();    // 调用之前注册的构造函数

constructor ctor = t.get_constructor();  // 通过 constructor 类的另一种方式
var = ctor.invoke();
std::cout << var.get_type().get_name(); // 输出 'MyStruct'
```

### 设置 / 获取属性（Set/get properties）

```cpp
MyStruct obj;

property prop = type::get(obj).get_property("data");
prop.set_value(obj, 23);

variant var_prop = prop.get_value(obj);
std::cout << var_prop.to_int(); // 输出 '23'
```

### 调用方法（Invoke Methods）：

```cpp
MyStruct obj;

method meth = type::get(obj).get_method("func");
meth.invoke(obj, 42.0);

variant var = type::get(obj).create();
meth.invoke(var, 42.0);
```

特性
---------

- 反射构造函数、方法、数据成员和枚举
- 支持类的 *单继承*、*多继承* 和 *虚继承*
- 构造函数（任意参数数量）
- 方法（*虚函数*、*抽象函数*、*重载*、任意参数数量）
- 数组（包含原生数组；任意维度数量）
- 可从任意类层级调用类的属性和方法
- 无头文件污染；反射信息在 cpp 文件中创建，以最小化修改数据时的编译时间
- 无需在编译时拥有类型声明，即可操作自定义类型（对插件开发非常有用）
- 可为所有反射对象添加额外的 *元数据（metadata）*
- 可为方法或构造函数添加 *默认参数*
- 通过 *策略（policies）* 调整注册行为
- 最小化宏的使用
- **不**需要额外的第三方依赖
- **不**需要 RTTI；内置了一个更快、跨共享库工作的替代方案
- **不**使用异常（该特性有运行时成本，且在主机平台上通常会被禁用）
- **不**需要外部编译器或工具；本分支基于标准 C++26 静态反射（原始版本基于 C++11）

可移植性
-----------
已使用以下工具链测试和编译：

- GCC 16.2
- MinGW 16.2