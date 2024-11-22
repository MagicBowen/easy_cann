/**
* Copyright (c) wangbo@joycode.art 2024
*/

#ifndef TYPELIST_H
#define TYPELIST_H

#include <cstddef>
#include "conditional.h"

namespace asl {

/////////////////////////////////////////////////////////////////////////////////////
// TypeList 定义
template <typename... Ts>
struct TypeList {};

/////////////////////////////////////////////////////////////////////////////////////
// IsEmpty 元结构
template <typename List>
struct TypeList_IsEmpty;

template <>
struct TypeList_IsEmpty<TypeList<>> {
    static constexpr bool value = true;
};

template <typename... Ts>
struct TypeList_IsEmpty<TypeList<Ts...>> {
    static constexpr bool value = false;
};

/////////////////////////////////////////////////////////////////////////////////////
// Size 元结构
template <typename List>
struct TypeList_Size;

template <typename... Ts>
struct TypeList_Size<TypeList<Ts...>> {
    static constexpr size_t value = sizeof...(Ts);
};

/////////////////////////////////////////////////////////////////////////////////////
// Get 元结构
template <typename TypeList, std::size_t N>
struct TypeList_Get;

template <typename T, typename... Ts, std::size_t N>
struct TypeList_Get<TypeList<T, Ts...>, N> {
    using type = typename TypeList_Get<TypeList<Ts...>, N - 1>::type;
};

template <typename T, typename... Ts>
struct TypeList_Get<TypeList<T, Ts...>, 0> {
    using type = T;
};

template <std::size_t N>
struct TypeList_Get<TypeList<>, N> {
    static_assert(N < 0, "Index out of bounds in TypeList_Get");
};

/////////////////////////////////////////////////////////////////////////////////////
// ByteOffset 元结构
template <typename TypeList, std::size_t N>
struct TypeList_ByteOffset;

template <std::size_t N>
struct TypeList_ByteOffset<TypeList<>, N> {
    static_assert(N < 0, "Index out of range for empty TypeList.");
    static constexpr std::size_t value = 0;
};

template <typename Head, typename... Tail>
struct TypeList_ByteOffset<TypeList<Head, Tail...>, 0> {
    static constexpr std::size_t value = 0;
};

template <typename Head, typename... Tail, std::size_t N>
struct TypeList_ByteOffset<TypeList<Head, Tail...>, N> {
    static_assert(N < sizeof...(Tail) + 1, "Index out of range for TypeList.");
    static constexpr std::size_t value = sizeof(Head) + TypeList_ByteOffset<TypeList<Tail...>, N - 1>::value;
};

/////////////////////////////////////////////////////////////////////////////////////
// Prepend 元结构
template <typename T, typename List>
struct TypeList_Prepend;

template <typename T, typename... Ts>
struct TypeList_Prepend<T, TypeList<Ts...>> {
    using type = TypeList<T, Ts...>;
};

/////////////////////////////////////////////////////////////////////////////////////
// Filter 元结构
template <typename List, template <typename> class Pred>
struct TypeList_Filter;

template <template <typename> class Pred>
struct TypeList_Filter<TypeList<>, Pred> {
    using type = TypeList<>;
};

template <typename Head, typename... Tail, template <typename> class Pred>
struct TypeList_Filter<TypeList<Head, Tail...>, Pred> {
private:
    using TailFiltered = typename TypeList_Filter<TypeList<Tail...>, Pred>::type;

public:
    using type = typename Conditional<
        Pred<Head>::value,
        typename TypeList_Prepend<Head, TailFiltered>::type,
        TailFiltered
    >::type;
};

/////////////////////////////////////////////////////////////////////////////////////
// Map 元结构
template <typename List, template <typename> class Mapper>
struct TypeList_Map;

template <template <typename> class Mapper>
struct TypeList_Map<TypeList<>, Mapper> {
    using type = TypeList<>;
};

template <typename Head, typename... Tail, template <typename> class Mapper>
struct TypeList_Map<TypeList<Head, Tail...>, Mapper> {
private:
    using MappedHead = typename Mapper<Head>::type;
    using MappedTail = typename TypeList_Map<TypeList<Tail...>, Mapper>::type;

public:
    using type = typename TypeList_Prepend<MappedHead, MappedTail>::type;
};

/////////////////////////////////////////////////////////////////////////////////////
// Reduce 元结构
template <typename List, typename Init, template <typename, typename> class Reducer>
struct TypeList_Reduce;

template <typename Init, template <typename, typename> class Reducer>
struct TypeList_Reduce<TypeList<>, Init, Reducer> {
    using type = Init;
};

template <typename Head, typename... Tail, typename Init, template <typename, typename> class Reducer>
struct TypeList_Reduce<TypeList<Head, Tail...>, Init, Reducer> {
private:
    using NewInit = typename Reducer<Init, Head>::type;
    using ReducedTail = typename TypeList_Reduce<TypeList<Tail...>, NewInit, Reducer>::type;

public:
    using type = ReducedTail;
};

/////////////////////////////////////////////////////////////////////////////////////
// Apply 元结构
template <typename List, template <typename, size_t> class Func, size_t Index = 0>
struct TypeList_Apply;

template <template <typename, size_t> class Func, size_t Index>
struct TypeList_Apply<TypeList<>, Func, Index> {
    static constexpr void execute() {}
};

template <typename Head, typename... Tail, template <typename, size_t> class Func, size_t Index>
struct TypeList_Apply<TypeList<Head, Tail...>, Func, Index> {
    static constexpr void execute() {
        Func<Head, Index>::execute();
        TypeList_Apply<TypeList<Tail...>, Func, Index + 1>::execute();
    }
};

}

#endif
