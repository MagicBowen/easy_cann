/**
* Copyright (c) wangbo@joycode.art 2024
*/

#ifndef TUPLE_H
#define TUPLE_H

#include "type_list.h"
#include "forward.h"

namespace asl {

/////////////////////////////////////////////////////////////////////////////////////
template <typename... Ts>
struct Tuple {};

template <typename T, typename... Ts>
struct Tuple<T, Ts...> {
    T head;
    Tuple<Ts...> tail;
    
    Tuple() : head(), tail() {}
    Tuple(T&& h, Ts&&... ts) : head(asl::forward<T>(h)), tail(asl::forward<Ts>(ts)...) {}
};

template <>
struct Tuple<> {};

/////////////////////////////////////////////////////////////////////////////////////
template <typename... Ts>
Tuple<Ts&&...> ForwardAsTuple(Ts&&... ts) {
    return Tuple<Ts&&...>(std::forward<Ts>(ts)...);
}

/////////////////////////////////////////////////////////////////////////////////////
template <typename List>
struct TupleFromTypeList;

template <typename... Ts>
struct TupleFromTypeList<TypeList<Ts...>> {
    using type = Tuple<Ts...>;
};

/////////////////////////////////////////////////////////////////////////////////////
template <typename TupleType>
struct TupleSize;

template <typename... Ts>
struct TupleSize<Tuple<Ts...>> {
    static constexpr size_t value = sizeof...(Ts);
};

/////////////////////////////////////////////////////////////////////////////////////
// 获取 Tuple 中第 N 个元素的类型
template <std::size_t N, typename TupleType>
struct TupleElemType;

template <typename T, typename... Ts>
struct TupleElemType<0, Tuple<T, Ts...>> {
    using type = T;
};

template <std::size_t N, typename T, typename... Ts>
struct TupleElemType<N, Tuple<T, Ts...>> {
    static_assert(N -1 < sizeof...(Ts), "N overflow!");
    using type = typename TupleElemType<N - 1, Tuple<Ts...>>::type;
};

/////////////////////////////////////////////////////////////////////////////////////
// 获取特定索引的 Tuple 元素的引用
template <std::size_t N, typename TupleType>
typename TupleElemType<N, TupleType>::type& TupleElemGet(TupleType& tuple) {
    if constexpr (N == 0) {
        return tuple.head;
    } else {
        return TupleElemGet<N - 1>(tuple.tail);
    }
}

}

#endif
