/**
* Copyright (c) wangbo@joycode.art 2024
*/

#ifndef BOOL_H
#define BOOL_H

namespace asl {

template <class T, T V>
struct integral_constant {
    static constexpr const T value = V;
    typedef T value_type;
    typedef integral_constant type;
    inline constexpr operator value_type() const noexcept { return value; }
};

template <bool V>
using BoolType = integral_constant<bool, V>;

using FalseType = BoolType<false>;
using TrueType  = BoolType<true>;

template<typename T>
struct IsBoolType : FalseType {
    static constexpr bool value = false;
};

template<bool V>
struct IsBoolType<BoolType<V>> : TrueType {
    static constexpr bool value = true;
    static constexpr bool boolValue = V;
};

// Conditional 元结构
template <bool cond, typename Then, typename Else>
struct Conditional;

template <typename Then, typename Else>
struct Conditional<true, Then, Else> {
    using type = Then;
};

template <typename Then, typename Else>
struct Conditional<false, Then, Else> {
    using type = Else;
};

}

#endif
