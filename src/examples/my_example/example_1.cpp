#include <format>
#include <iostream>
#include <print>
#include <memory>
#include <string>
#include <vector>
#include <rttr/registration>
#include <rttr/type>

using namespace rttr;

// ============================================================================
// 1. 枚举定义 —— 通过 do_enumeration 自动注册
// ============================================================================
enum class ShapeType
{
    Circle,
    Rectangle,
    Triangle
};

// ============================================================================
// 2. 全局自由函数 —— 通过 do_method 注册，附带元数据
// ============================================================================
[[=metadata("DESC", std::string{"计算两数之和"})]]
int add_numbers(int a, int b)
{
    return a + b;
}

[[=metadata("DESC", "问候函数")]]
std::string greet(const std::string& name)
{
    return std::format("Hello, {}!", name);
}

// 2b. 重载函数 —— 通过 do_method_overload 注册
//     说明：do_method_overload 需要通过 static_cast 指定具体函数指针类型，
//           用于消除重载歧义；注册后可通过统一名字调用不同重载版本。
//     注意：使用 do_method_overload 时，默认参数和参数名将失效。
// ============================================================================
namespace example_detail
{
// 保存最后一次调用结果，用于演示
std::string last_format_result;
}

[[=metadata("DESC", "格式化 int 值")]]
std::string format_value(int v)
{
    example_detail::last_format_result = std::format("int: {}", v);
    return example_detail::last_format_result;
}

[[=metadata("DESC", "格式化 double 值")]]
std::string format_value(double v)
{
    example_detail::last_format_result = std::format("double: {:.3}", v);
    return example_detail::last_format_result;
}

[[=metadata("DESC", "格式化 string 值")]]
std::string format_value(const std::string& s)
{
    example_detail::last_format_result = std::format("string: \"{}\"", s);
    return example_detail::last_format_result;
}

// ============================================================================
// 3. 基类 Shape —— 演示 do_class 自动注册所有成员
//    展示：构造函数策略、重命名、忽略、元数据、属性/方法
// ============================================================================
struct Shape
{
    // 为默认构造函数指定 raw_ptr 策略，create() 将返回 Shape*
    [[=rttr::class_op::ctor_policy(rttr::policy::ctor::op_as_raw_ptr)]]
    Shape()
    {
        std::println("  [Shape()] 默认构造");
    }

    // 带参构造函数（默认 as_object 策略，返回值拷贝）
    Shape(std::string name) : m_name(std::move(name))
    {
        std::println("  [Shape(name)] 带参构造: {}", m_name);
    }

    virtual ~Shape()
    {
        std::println("  [~Shape()] 析构: {}", m_name);
    }

    // ---- 成员变量属性（do_class 自动注册为 property）----
    std::string m_name = "unnamed";

    // 使用 rename 注解在反射中重命名属性
    [[=rttr::class_op::rename("visible")]]
    bool m_visible = true;

    // 使用元数据标注属性（mutable 允许在 const 方法中修改）
    [[=metadata("UNIT", "pixels")]]
    mutable double m_area = 0.0;

    // 使用 ignore 注解跳过注册（不会出现在反射信息中）
    [[=rttr::class_op::ignore]]
    int m_internal_id = 0;

    // ---- 成员方法（do_class 自动注册为 method）----
    virtual double compute_area() const
    {
        m_area = 0.0;
        return m_area;
    }

    void set_name(const std::string& name)
    {
        m_name = name;
    }

    const std::string& get_name() const
    {
        return m_name;
    }

    // 静态方法
    static std::string category()
    {
        return "2D Shape";
    }

    // 为方法指定策略：丢弃返回值
    [[=rttr::class_op::method_policy(rttr::policy::method::op_discard_return)]]
    const std::string& debug_info() const
    {
        static std::string info = "Shape debug info";
        return info;
    }

    RTTR_ENABLE()
};

// ============================================================================
// 4. 派生类 Circle —— 演示类层次结构
// ============================================================================
struct Circle : Shape
{
    // 默认构造函数使用 shared_ptr 策略
    [[=rttr::class_op::ctor_policy(rttr::policy::ctor::op_as_std_shared_ptr)]]
    Circle()
    {
        std::println("  [Circle()] 默认构造");
    }

    Circle(std::string name, double radius) : Shape(std::move(name)), m_radius(radius)
    {
        std::println("  [Circle(name, r)] 构造: radius={}", m_radius);
    }

    double m_radius = 0.0;

    // 重写虚函数
    double compute_area() const override
    {
        m_area = 3.14159265 * m_radius * m_radius;
        return m_area;
    }

    // Circle 独有方法
    [[=metadata("DESC", "获取半径")]]
    double get_radius() const
    {
        return m_radius;
    }

    void set_radius(double r)
    {
        m_radius = r;
    }

    RTTR_ENABLE()
};

// ============================================================================
// 5. 注册块 —— 使用 C++26 反射元编程自动注册
// ============================================================================
RTTR_REGISTRATION
{
    // 注册枚举
    registration::do_enumeration<^^ShapeType>("ShapeType");

    // 注册基类
    registration::do_class<^^Shape>("Shape");

    // 注册派生类
    registration::do_class<^^Circle>("Circle");

    // 注册全局函数
    registration::do_method<^^add_numbers>("add_numbers");
    registration::do_method<^^greet>("greet");

    type::register_converter_func([](const char* src, bool& status)
    {
        status = true;
        return std::string{src};
    });

    // 注册重载函数 —— 使用 do_method_overload，通过 static_cast 消除重载歧义
    // 注意：重载版本使用相同的注册名 "format_value"，调用时按参数类型匹配
    // 第二个参数 std::in_place_type<void> 用于元数据类型占位（void 表示无元数据）
    registration::do_method_overload<
        static_cast<std::string(*)(int)>(&format_value)>(
        "format_value");
    registration::do_method_overload<
        static_cast<std::string(*)(double)>(&format_value)>(
        "format_value");
    registration::do_method_overload<
        static_cast<std::string(*)(const std::string&)>(&format_value)>(
        "format_value");
}

// ============================================================================
// 辅助打印函数
// ============================================================================
static void print_type_info(const type& t)
{
    std::println("  类型名: {}", t.get_name());
    std::println("  大小: {} 字节", t.get_sizeof());
    std::println("  是类: {}", t.is_class());
    std::println("  是枚举: {}", t.is_enumeration());
}

static void print_properties(const type& t)
{
    std::println("  属性列表:");
    for (auto& prop : t.get_properties())
    {
        std::println("    - {} : {}{}",
                     prop.get_name(),
                     prop.get_type().get_name(),
                     prop.is_readonly() ? " (只读)" : "");
        // 打印元数据
        auto meta_desc = prop.get_metadata("DESC");
        if (meta_desc)
            std::println("      元数据 DESC: {}", meta_desc.to_string());
        auto meta_unit = prop.get_metadata("UNIT");
        if (meta_unit)
            std::println("      元数据 UNIT: {}", meta_unit.to_string());
    }
}

static void print_methods(const type& t)
{
    std::println("  方法列表:");
    for (auto& meth : t.get_methods())
    {
        std::println("    - {} : {} [签名: {}]",
                     meth.get_name(),
                     meth.get_return_type().get_name(),
                     meth.get_signature());
        if (meth.is_static())
            std::println("      (静态方法)");
    }
}

// ============================================================================
// 主函数 —— 演示各项功能
// ============================================================================
int main()
{
    std::println("╔════════════════════════════════════════════════════════════╗");
    std::println("║          RTTR 运行时类型反射库 —— 功能演示                  ║");
    std::println("╚════════════════════════════════════════════════════════════╝\n");

    // ──────────────────────────────────────────────────────────────
    std::println("【1】类型检索与基本信息");
    std::println("─────────────────────────────────────────────────");
    {
        type t_shape = type::get<Shape>();
        type t_circle = type::get<Circle>();
        type t_by_name = type::get_by_name("Circle");

        std::println("type::get<Shape>():");
        print_type_info(t_shape);
        std::println();
        std::println("type::get<Circle>():");
        print_type_info(t_circle);
        std::println();
        std::println("type::get_by_name(\"Circle\") == type::get<Circle>(): {}",
                     t_by_name == t_circle);
    }

    // ──────────────────────────────────────────────────────────────
    std::println("\n【2】枚举反射");
    std::println("─────────────────────────────────────────────────");
    {
        type t_enum = type::get<ShapeType>();
        std::println("是否枚举: {}", t_enum.is_enumeration());

        enumeration enum_obj = t_enum.get_enumeration();
        std::println("枚举名: {}", enum_obj.get_name());

        // 遍历枚举值
        std::println("枚举值:");
        for (auto& val : enum_obj.get_values())
        {
            auto name = enum_obj.value_to_name(val);
            std::println("  {} = {}", name, val.to_int());
        }

        // 名字 ↔ 值 转换
        variant v = enum_obj.name_to_value("Circle");
        std::println("name_to_value(\"Circle\") = {}", v.to_int());
        std::println("value_to_name(ShapeType::Triangle) = {}",
                     enum_obj.value_to_name(ShapeType::Triangle));
    }

    // ──────────────────────────────────────────────────────────────
    std::println("\n【3】属性与方法的运行时遍历");
    std::println("─────────────────────────────────────────────────");
    {
        type t_circle = type::get<Circle>();

        std::println("Circle 类属性:");
        print_properties(t_circle);
        std::println();
        std::println("Circle 类方法:");
        print_methods(t_circle);
    }

    // ──────────────────────────────────────────────────────────────
    std::println("\n【4】通过构造函数创建实例");
    std::println("─────────────────────────────────────────────────");
    {
        type t_circle = type::get<Circle>();

        // 4a. 使用默认构造函数（shared_ptr 策略）
        std::println("a) 默认构造 (shared_ptr 策略):");
        variant var1 = t_circle.create({});
        std::println("  返回类型: {}", var1.get_type().get_name());
        std::println("  是否 wrapper: {}", var1.get_type().is_wrapper());
        std::println("  解包类型: {}", var1.get_type().get_wrapped_type().get_name());

        // 4b. 使用带参构造函数（as_object 策略）
        std::println("\nb) 带参构造 (as_object 策略):");
        type t_shape = type::get<Shape>();
        variant var2 = t_shape.create({std::string("MyShape")});
        std::println("  返回类型: {}", var2.get_type().get_name());

        // 4c. 使用 constructor 类精确查找
        std::println("\nc) 通过 constructor 查找:");
        auto ctors = t_circle.get_constructors();
        std::println("  Circle 注册的构造函数数量: {}", ctors.size());
        for (auto& ctor : ctors)
        {
            std::println("    签名: {}", ctor.get_signature());
        }
    }

    // ──────────────────────────────────────────────────────────────
    std::println("\n【5】属性读写操作");
    std::println("─────────────────────────────────────────────────");
    {
        Circle circle("TestCircle", 5.0);
        type t = type::get(circle);

        // 读取属性
        property prop_name = t.get_property("m_name");
        property prop_radius = t.get_property("m_radius");
        property prop_visible = t.get_property("visible"); // 注意 rename 后的名字
        property prop_hidden = t.get_property("m_internal_id"); // 被 ignore

        std::println("m_name = {}", prop_name.get_value(circle).to_string());
        std::println("m_radius = {}", prop_radius.get_value(circle).to_double());
        std::println("visible = {}", prop_visible.get_value(circle).to_bool());
        std::println("m_internal_id 是否已注册: {}", prop_hidden.is_valid());

        // 写入属性
        prop_name.set_value(circle, std::string("UpdatedCircle"));
        prop_radius.set_value(circle, 10.0);
        prop_visible.set_value(circle, false);

        std::println("\n修改后:");
        std::println("  m_name = {}", circle.m_name);
        std::println("  m_radius = {}", circle.m_radius);
        std::println("  visible (m_visible) = {}", circle.m_visible);
    }

    // ──────────────────────────────────────────────────────────────
    std::println("\n【6】方法调用");
    std::println("─────────────────────────────────────────────────");
    {
        Circle circle("MethodTest", 3.0);
        type t = type::get(circle);

        // 6a. 调用带返回值的方法
        method meth_area = t.get_method("compute_area");
        variant result = meth_area.invoke(circle);
        std::println("compute_area() = {:.4}", result.to_double());

        // 6b. 调用带参数的方法
        method meth_set_name = t.get_method("set_name");
        meth_set_name.invoke(circle, std::string("RenamedByReflection"));
        std::println("set_name 后 m_name = {}", circle.m_name);

        // 6c. 调用 const 方法
        method meth_get_name = t.get_method("get_name");
        variant name = meth_get_name.invoke(circle);
        std::println("get_name() = {}", name.to_string());

        // 6d. 调用静态方法
        method meth_category = t.get_method("category");
        variant cat = meth_category.invoke(instance{});
        std::println("category() = {}", cat.to_string());

        // 6e. 调用有元数据的方法
        method meth_get_radius = t.get_method("get_radius");
        auto meta = meth_get_radius.get_metadata("DESC");
        if (meta)
            std::println("get_radius 元数据 DESC = {}", meta.to_string());
        variant r = meth_get_radius.invoke(circle);
        std::println("get_radius() = {}", r.to_double());
    }

    // ──────────────────────────────────────────────────────────────
    std::println("\n【7】元数据查询");
    std::println("─────────────────────────────────────────────────");
    {
        type t_shape = type::get<Shape>();

        // 查询属性的元数据
        property prop_area = t_shape.get_property("m_area");
        auto unit = prop_area.get_metadata("UNIT");
        if (unit)
        {
            const char* mm = nullptr;
            unit.convert(mm);
            std::println("属性 m_area 元数据 UNIT = {}", unit.to_string());
        }

        // 查询方法的元数据（通过全局函数）
        method meth = type::get_global_method("add_numbers");
        auto desc = meth.get_metadata("DESC");
        if (desc)
            std::println("方法 add_numbers 元数据 DESC = {}", desc.to_string());

        // 查询类型的元数据
        // (do_class 会将类级别的 [[=metadata(...)]] 注解注册为类型元数据)
    }

    // ──────────────────────────────────────────────────────────────
    std::println("\n【8】类层次结构");
    std::println("─────────────────────────────────────────────────");
    {
        type t_circle = type::get<Circle>();
        type t_shape = type::get<Shape>();

        std::println("Circle 是否派生自 Shape: {}", t_circle.is_derived_from(t_shape));
        std::println("Shape 是否 Circle 的基类: {}", t_shape.is_base_of(t_circle));

        std::println("\nCircle 的基类:");
        for (auto& base : t_circle.get_base_classes())
            std::println("  - {}", base.get_name());

        std::println("\nShape 的派生类:");
        for (auto& derived : t_shape.get_derived_classes())
            std::println("  - {}", derived.get_name());
    }

    // ──────────────────────────────────────────────────────────────
    std::println("\n【9】Variant 类型转换");
    std::println("─────────────────────────────────────────────────");
    {
        // int → string
        variant v1 = 42;
        std::println("int → string: {} → {}", v1.get_type().get_name(), v1.to_string());

        // string → int
        variant v2 = std::string("123");
        bool ok = false;
        int n = v2.convert<int>(&ok);
        std::println("string → int: \"123\" → {} (成功: {})", n, ok);

        // double → int
        variant v3 = 3.14;
        std::println("double → int: 3.14 → {}", v3.to_int());

        // bool → string
        variant v4 = true;
        std::println("bool → string: true → {}", v4.to_string());

        // 检查能否转换
        variant v5 = std::string("hello");
        std::println("\"hello\" 能否转 int: {}", v5.can_convert<int>() ? "是" : "否");
        std::println("\"hello\" 能否转 string: {}", v5.can_convert<std::string>() ? "是" : "否");
    }

    // ──────────────────────────────────────────────────────────────
    std::println("\n【10】全局方法调用");
    std::println("─────────────────────────────────────────────────");
    {
        // 通过名字查找全局方法
        method meth_add = type::get_global_method("add_numbers");
        if (meth_add)
        {
            variant result = meth_add.invoke(instance{}, 10, 20);
            std::println("add_numbers(10, 20) = {}", result.to_int());
        }

        method meth_greet = type::get_global_method("greet");
        if (meth_greet)
        {
            variant result = meth_greet.invoke(instance{}, std::string("RTTR"));
            std::println("greet(\"RTTR\") = {}", result.to_string());
        }

        // 列出所有全局方法
        auto global_methods = type::get_global_methods();
        std::println("\n已注册全局方法数量: {}", global_methods.size());
    }

    // ──────────────────────────────────────────────────────────────
    std::println("\n【11】rttr_cast 跨类型转换");
    std::println("─────────────────────────────────────────────────");
    {
        // 使用 raw_ptr 策略创建 Shape 实例
        type t_shape = type::get<Shape>();
        variant var_shape = t_shape.create({});
        Shape* shape_ptr = var_shape.get_value<Shape*>();

        // 通过基类指针设置属性
        property prop = t_shape.get_property("m_name");
        prop.set_value(*shape_ptr, std::string("CastTest"));
        std::println("基类 Shape* m_name = {}", shape_ptr->m_name);

        // 清理
        t_shape.destroy(var_shape);
        std::println("已销毁实例");
    }

    // ──────────────────────────────────────────────────────────────
    std::println("\n【12】通过 type::invoke 快捷调用");
    std::println("─────────────────────────────────────────────────");
    {
        Circle circle("InvokeTest", 7.0);

        // 使用 type::invoke 快捷调用方法
        variant result = type::get(circle).invoke("compute_area", circle, {});
        std::println("invoke(\"compute_area\") = {:.4}", result.to_double());

        // 使用全局 invoke
        variant global_result = type::invoke("add_numbers", {15, 27});
        std::println("全局 invoke(\"add_numbers\", {{15, 27}}) = {}", global_result.to_int());
    }

    // ──────────────────────────────────────────────────────────────
    std::println("\n【13】do_method_overload 重载函数调用");
    std::println("─────────────────────────────────────────────────");
    {
        // 查找所有名为 "format_value" 的全局方法
        // do_method_overload 注册的多个重载版本共享同一名字
        std::vector<method> overloads;
        for (auto& m : type::get_global_methods())
        {
            if (m.get_name() == "format_value")
                overloads.push_back(m);
        }
        std::println("已注册的 format_value 重载数量: {}", overloads.size());
        for (auto& m : overloads)
            std::println("  - 签名: {}", m.get_signature());

        // 通过参数类型匹配选择对应重载版本，结果通过全局变量取回
        // int 重载
        example_detail::last_format_result.clear();
        type::invoke("format_value", {42});
        std::println("format_value(42)        = {}", example_detail::last_format_result);

        // double 重载
        example_detail::last_format_result.clear();
        type::invoke("format_value", {3.14});
        std::println("format_value(3.14)     = {}", example_detail::last_format_result);

        // string 重载
        example_detail::last_format_result.clear();
        std::string arg = "hello";
        type::invoke("format_value", {arg});
        std::println("format_value(\"hello\")  = {}", example_detail::last_format_result);

        // 通过 method::invoke 显式调用某个重载版本
        std::println("\n通过 method::invoke 逐个调用:");
        for (auto& m : overloads)
        {
            example_detail::last_format_result.clear();
            // 尝试以 int 参数调用，只有 int 重载版本能匹配
            variant res = m.invoke(instance{}, 100);
            bool matched = res.is_valid() && !example_detail::last_format_result.empty();
            std::println("  重载 [{}] -> {}",
                         m.get_signature(),
                         matched ? example_detail::last_format_result : "参数不匹配");
        }
    }

    std::println("\n╔════════════════════════════════════════════════════════════╗");
    std::println("║                     演示结束                                ║");
    std::println("╚════════════════════════════════════════════════════════════╝");

    return 0;
}