//
// Created by bzh on 2026/9/13.
//

#ifndef RTTR_CTP_H
#define RTTR_CTP_H

//
// Created by bzh on 2026/7/27.
//

#include <iostream>
#include <ostream>
#include <meta>
#include <ranges>
#include <utility>
#include <flat_map>

namespace ctp {
    // The customization point to opt-in to having your type be usable as a
    // ctp::Param.
    //
    // Reflect<T> must provide a target_type, a serialize() with the
    // following signature, and one of three forms of deserialization function:
    //
    //
    //  struct {
    //      using target_type = ????;
    //      static consteval auto serialize(Serializer&, T const&) -> void;
    //
    //      // Option 1. Take all of the serialized infos as constant template parameters
    //      template <std::meta::info... Is>
    //      static consteval auto deserialize() -> target_type;
    //
    //      // Option 2: Take all of the serialized infos as function parameters
    //      static consteval auto deserialize(std::meta::info...) -> target_type;
    //
    //      // Option 3: Take the splices of all of the serialized infos as function
    //      // parameters
    //      static consteval auto deserialize_constants(auto&&...) -> target_type;
    //
    //  };

    template<class T>
    struct Reflect {
        using target_type = T;

        static T reduction(const target_type &t) requires (std::is_structural_v<T>) {
            return t;
        }
    };

    namespace impl {
        template<class T>
        using custom_target = Reflect<T>::target_type;
    }

    // The target type for a given T. For structural types, the target is always
    // T in order to be consistent with regular C++20 template parameters.
    //using target = std::conditional_t<std::is_structural_v<T>, T, impl::custom_target<T> >;
    template<class T>
    using target = [:is_structural_type(^^T) ? ^^T : substitute(^^impl::custom_target, {^^T}):];

    // For those cases where references need to be preserved,
    // target_or_ref<T&> is T& but target_or_ref<T> is target<T>
    template<class T>
    using target_or_ref = [:std::meta::is_lvalue_reference_type(^^T)
                                ? ^^T
                                : substitute(^^target, {^^T}):];


    namespace impl {
        // This is the singular (private) object that will be
        // constructed from the serialization-deserialization round trip.
        template<class T, std::meta::info... Is>
        inline constexpr target<T> the_object = [] {
            if constexpr (requires { Reflect<T>::template deserialize<Is...>(); }) {
                return Reflect<T>::template deserialize<Is...>();
            }
            else if constexpr (requires { Reflect<T>::deserialize(Is...); }) {
                return Reflect<T>::deserialize(Is...);
            }
            else {
                return Reflect<T>::deserialize_constants([:Is:]...);
            }
        }();

        // This is the singular (private) object that will be used for reflect_constant_array
        template<class T, std::meta::info... Is>
        inline constexpr target<T> the_array[] = {[:Is:]...};

        // The default/simple approach to serialization, using Serializer
        template<class T>
        consteval auto default_serialize(T const &v) -> std::meta::info;
    }

    inline constexpr auto normalize =
            []<class T>([[maybe_unused]] T &v) -> void {
        if constexpr (requires { std::is_string_literal(v); }) {
            char const *global = std::define_static_string(std::string_view(v));
            v = static_cast<T>(global);
        }
    };

    // Extension of std::meta::reflect_constant that is customizable via Reflect<T>.
    // For scalar types, returns a reflection representing a value.
    // For class types, returns a reflection representing an object.
    inline constexpr auto reflect_constant =
            []<class T>(T v) {
        if constexpr (is_structural_type(^^T)) {
            normalize(v);
            return std::meta::reflect_constant(v);
        }
        else {
            // For non-structural types, customize via Reflect<T>::serialize
            return impl::default_serialize(v);
        }
    };

    // Extension of std::meta::reflect_constant_array. Returns a reflection representing
    // an array of target<T>, where T is the value type of the range.
    inline constexpr auto reflect_constant_array =
            []<std::ranges::input_range R>(R &&r) {
        std::vector<std::meta::info> elems = {^^std::ranges::range_value_t < R >};
        for (auto &&e: r) {
            elems.push_back(reflect_constant(reflect_constant(e)));
        }
        return substitute(^^impl::the_array, elems);
    };

    // Extension of std::define_static_object, except based on ctp::reflect_constant
    inline constexpr auto define_static_object =
            []<class T>(T const &v) -> target<T> const & {
        // ctp::reflect_constant gives us a reflection representing an object
        // UNLESS T is a scalar type, in which case we have to do something else
        if constexpr (is_class_type(^^T)) {
            return std::meta::extract<target<T> const &>(reflect_constant(v));
        }
        else {
            // should be std::define_static_object but not implemented in clang
            //return std::define_static_array(std::span(std::addressof(v), 1))[0];
            return *std::define_static_object(v);
        }
    };
}

namespace ctp {
    // For a lot of types, the easiest way to do serialization is just to push a
    // bunch of reflections and then get them all back out as function parameters.
    // This API is provided as a convenience, and is used by providing both:
    //
    //      auto Reflect<T>::serialize(Serializer&, T const&) -> void;
    //      auto Reflect<T>::deserialize(std::meta::info...) -> target_type;
    class Serializer {
        std::vector<std::meta::info> parts;

    public:
        explicit consteval Serializer(std::meta::info type) {
            parts.push_back(type);
        }

        // Push another reflection
        consteval auto push(std::meta::info r) -> void {
            parts.push_back(std::meta::reflect_constant(r));
        }

        // Push a ctp-reflectable value
        template<class T>
        consteval auto push_constant(T const &v) -> void {
            push(reflect_constant(v));
        }

        // Push an object (for when the identity of the object, as opposed to its value, matters)
        // e.g. this is for reference members
        template<class T>
        consteval auto push_object(T &&o) -> void {
            push(std::meta::reflect_object(o));
        }

        // Equivalent to: push_object(o) if type is an lvalue reference, otherwise push_constant(o)
        template<class T>
        consteval auto push_constant_or_object(std::meta::info type, T &&o) -> void {
            if (is_lvalue_reference_type(type)) {
                push_object(o);
            }
            else {
                push_constant(o);
            }
        }

        // Returns: A reflection representing an object of type target<T>,
        // where T is the type the Serializer was constructed from, that is
        // initialized with Reflect<T>::dserialize(r...) where {r...} is the
        // sequence of reflections that were push()-ed onto this Serializer
        consteval auto finalize() const -> std::meta::info {
            return object_of(substitute(^^impl::the_object, parts));
        }
    };

    namespace impl {
        template<class T>
        consteval auto default_serialize(T const &v) -> std::meta::info {
            Serializer s{^^T};
            Reflect<T>::serialize(s, v);
            return s.finalize();
        }
    }
}

namespace ctp {
    // The main user-facing interface: ctp::Param<T> is the way to have a constant
    // template parameter of type T(~ish).
    template<class T>
    struct Param {
        using type = target<T>;
        type const &value;

        consteval Param(T const &v) : value(define_static_object(v)) {
        }

        consteval operator type const &() const {
            return value;
        }

        consteval auto get() const -> type const & {
            return value;
        }

        consteval auto operator*() const -> type const & {
            return value;
        }

        consteval auto operator->() const -> type const * {
            return std::addressof(value);
        }

        T reduction() const {
            return Reflect<T>::reduction(value);
        }
    };

    template<class T> requires (std::meta::is_structural_type(^^T))
    struct Param<T> {
        using type = T;
        T value;

        consteval Param(T const &v) : value(v) {
            ctp::normalize(value);
        }

        consteval operator T const &() const {
            return value;
        }

        consteval auto get() const -> T const & {
            return value;
        }

        consteval auto operator*() const -> T const & {
            return value;
        }

        consteval auto operator->() const -> T const * {
            return std::addressof(value);
        }

        T reduction() const {
            return value;
        }
    };

    template<class T>
    Param(T) -> Param<T>;
}

namespace ctp {
    struct string_ref {
        const char *_data;
        std::size_t _size;

        constexpr string_ref(const char *d, std::size_t s) : _data{d}, _size{s} {
        }

        template<std::size_t N>
        constexpr string_ref(const char (&it)[N]) : _data{it}, _size{N - 1} {
        }

        [[nodiscard]] constexpr const char *data() const noexcept {
            return _data;
        }

        [[nodiscard]] constexpr std::size_t size() const noexcept {
            return _size;
        }

        constexpr operator std::string() const {
            return std::string{_data, _size};
        }

        constexpr operator std::string_view() const {
            return std::string_view{_data, _size};
        }

        constexpr auto operator<=>(const string_ref &it) const {
            return std::string_view{_data, _size} <=> std::string_view{it._data, it._size};
        }

        constexpr auto operator==(const string_ref &it) const {
            return std::string_view{_data, _size} == std::string_view{it._data, it._size};
        }
    };

    template<>
    struct Reflect<std::string> {
        using target_type = string_ref;

        static consteval void serialize(Serializer &ser, std::string const &str) {
            ser.push(std::meta::reflect_constant_string(str));
        }

        static consteval target_type deserialize(std::meta::info r) {
            return string_ref{
                std::meta::extract<char const *>(r),
                std::meta::extent(type_of(r)) - 1
            };
        }

        static constexpr std::string reduction(const target_type &t) {
            return std::string{t};
        }
    };

    template<class T>
    struct Reflect<std::vector<T> > {
        using target_type = std::span<target<T> const>;

        static consteval void serialize(Serializer &s, std::vector<T> const &v) {
            s.push(reflect_constant_array(v));
        }

        static consteval target_type deserialize(std::meta::info r) {
            return std::span(extract<target<T> const *>(r), extent(type_of(r)));
        }

        static constexpr std::vector<T> reduction(const target_type &t) {
            std::vector<T> vec;
            vec.reserve(t.size());
            for (auto &it: t) {
                vec.push_back(Reflect<T>::reduction(it));
            }
            return vec;
        }
    };

    template<class T>
    struct Reflect<std::optional<T> > {
        using target_type = std::optional<target<T> >;

        static consteval void serialize(Serializer &s, std::optional<T> const &o) {
            if (o) {
                s.push_constant(*o);
            }
        }

        static consteval target_type deserialize_constants() {
            return {};
        }

        static consteval target_type deserialize_constants(target_or_ref<T> const &v) {
            return v;
        }

        static constexpr std::optional<T> reduction(const target_type &t) {
            if (t.has_value()) {
                return Reflect<T>::reduction(t.value());
            }
            else {
                return std::nullopt;
            }
        }
    };

    template<class... Ts>
    struct Reflect<std::tuple<Ts...> > {
        using target_type = std::tuple<target_or_ref<Ts>...>;

        static consteval void serialize(Serializer &s, std::tuple<Ts...> const &t) {
            auto &[...elems] = t;
            (s.push_constant_or_object(^^Ts, elems), ...);
        }

        static consteval target_type deserialize_constants(target_or_ref<Ts> const &... vs) {
            return target_type(vs...);
        }

        static constexpr std::tuple<Ts...> reduction(const target_type &t) {
            return [&]<std::size_t... I>(std::index_sequence<I...>) {
                return std::tuple < Ts...>
                {
                    Reflect<Ts>::reduction(std::get < I > (t))...
                };
            }(std::make_index_sequence < sizeof...(Ts) >
            {
            });
        }
    };

    template<class... Ts>
    struct Reflect<std::variant<Ts...> > {
        using target_type = std::variant<target<Ts>...>;

        static consteval void serialize(Serializer &s, std::variant<Ts...> const &v) {
            s.push_constant(v.index());
            // visit should work, but can't because of LWG4197
            template for (constexpr size_t I: std::views::iota(0zu, sizeof...(Ts))) {
                if (I == v.index()) {
                    s.push_constant(std::get < I > (v));
                    return;
                }
            }
        }

        template<std::meta::info I, std::meta::info R>
        static consteval target_type deserialize() {
            return target_type(std::in_place_index < ([:I:]) >, [:R:]);
        }

        static constexpr std::variant<Ts...> reduction(const target_type &t) {
            template for (constexpr size_t I: std::views::iota(0zu, sizeof...(Ts))) {
                if (I == t.index()) {
                    return Reflect<Ts...[I]>::reduction(std::get < I > (t));
                }
            }
            return {};
        }
    };

    template<class T>
    struct Reflect<std::reference_wrapper<T> > {
        using target_type = std::reference_wrapper<T>;

        static consteval void serialize(Serializer &s, std::reference_wrapper<T> r) {
            s.push_object(r.get());
        }

        static consteval target_type deserialize_constants(T &r) {
            return r;
        }

        static constexpr std::reference_wrapper<T> reduction(const target_type &t) {
            return t;
        }
    };

    template<>
    struct Reflect<std::string_view> {
        using target_type = string_ref;

        static consteval void serialize(Serializer &s, std::string_view sv) {
            s.push_constant(sv.data());
            s.push_constant(sv.size());
        }

        static consteval target_type deserialize_constants(char const *data, size_t size) {
            return string_ref{data, size};
        }

        static constexpr std::string_view reduction(const target_type &t) {
            return t;
        }
    };

    template<class T, size_t N>
    struct Reflect<std::span<T, N> > {
        using target_type = std::span<T, N>;

        static consteval void serialize(Serializer &s, std::span<T, N> sp) {
            s.push_constant(sp.data());
            s.push_constant(sp.size());
        }

        static consteval target_type deserialize_constants(T const *data, size_t size) {
            return target_type(data, size);
        }

        static constexpr std::span<T, N> reduction(const target_type &t) {
            return t;
        }
    };

    template<class KT, class VT>
    struct Reflect<std::pair<KT, VT> > {
        using target_type = std::pair<target<KT>, target<VT> >;

        static consteval void serialize(Serializer &s, std::pair<KT, VT> sp) {
            s.push(reflect_constant(sp.first));
            s.push(reflect_constant(sp.second));
        }

        static consteval target_type deserialize(std::meta::info f, std::meta::info s) {
            return std::make_pair(extract<target<KT> >(f), extract<target<VT> >(s));
        }

        static constexpr std::pair<KT, VT> reduction(const target_type &t) {
            return std::make_pair(Reflect<KT>::reduction(t.first), Reflect<VT>::reduction(t.second));
        }
    };

    template<class KT, class VT>
    struct Reflect<std::flat_map<KT, VT> > {
        using target_type = std::span<const std::pair<target<KT>, target<VT> >>;

        static consteval void serialize(Serializer &s, std::flat_map<KT, VT> sp) {
            std::vector<std::pair<KT, VT> > vec;
            for (const auto &it: sp) {
                vec.push_back(std::move(it));
            }
            s.push(reflect_constant_array(std::move(vec)));
        }

        static consteval target_type deserialize(std::meta::info r) {
            return std::span(extract<std::pair<target<KT>, target<VT> > const *>(r), extent(type_of(r)));
        }

        static constexpr std::flat_map<KT, VT> reduction(const target_type &t) {
            std::flat_map<KT, VT> rst;
            for (auto &it: t) {
                rst.insert(std::make_pair(Reflect<KT>::reduction(it.first),
                                          Reflect<VT>::reduction(it.second)));
            }
            return rst;
        }
    };

    template<class T, std::size_t N>
    struct Reflect<std::array<T, N> > {
        using target_type = std::array<target<T>, N>;

        static consteval void serialize(Serializer &s, std::array<T, N> const &v) {
            s.push(reflect_constant_array(v));
        }

        static consteval target_type deserialize(std::meta::info r) {
            auto *ptr = extract<target<T> const *>(r);
            return [&]<std::size_t... I>(std::index_sequence<I...>) {
                return std::array<target<T>, N>{ptr[I]...};
            }(std::make_index_sequence < N >
            {
            });
        }

        static constexpr std::array<T, N> reduction(const target_type &t) {
            return [&]<std::size_t... I>(std::index_sequence<I...>) {
                return std::array<T, N>{Reflect<T>::reduction(t[I])...};
            }(std::make_index_sequence < N >
            {
            });
        }
    };
}

#endif // RTTR_CTP_H