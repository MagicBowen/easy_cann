
/**
* Copyright (c) wangbo@joycode.art 2024
*/

#ifndef ASL_FORWARD_H
#define ASL_FORWARD_H

#include <type_traits>

namespace asl {

template<typename T>
constexpr T&& forward(std::remove_reference_t<T>& param) noexcept {
    return static_cast<T&&>(param);
}

template<typename T>
constexpr T&& forward(std::remove_reference_t<T>&& param) noexcept {
    static_assert(!std::is_lvalue_reference<T>::value, "Invalid forward of an rvalue as an lvalue");
    return static_cast<T&&>(param);
}

}

#endif