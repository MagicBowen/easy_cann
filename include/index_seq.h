/**
* Copyright (c) wangbo@joycode.art 2024
*/

#ifndef INDEX_SEQ_H
#define INDEX_SEQ_H

#include <cstddef>

namespace asl {

template <std::size_t... Is>
struct IndexSequence {};

template <std::size_t N, std::size_t... Is>
struct MakeIndexSequenceImpl : MakeIndexSequenceImpl<N - 1, N - 1, Is...> {};

template <std::size_t... Is>
struct MakeIndexSequenceImpl<0, Is...> {
    using type = IndexSequence<Is...>;
};

template <std::size_t N>
using MakeIndexSequence = typename MakeIndexSequenceImpl<N>::type;

}

#endif
