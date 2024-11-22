#include "catch2/catch.hpp"
#include "type_list.h"
#include "forward.h"
#include <type_traits>
#include <iostream>

using namespace asl;

/////////////////////////////////////////////////////////////////////////////////////
SCENARIO("Test type list meta function with size and empty") {
    using NilTypes = TypeList<>;
    static_assert(TypeList_Size<NilTypes>::value == 0, "Size of NilTypes should be 0");
    static_assert(TypeList_IsEmpty<NilTypes>::value == true, "NilTypes should be empty");

    using InputTypes = TypeList<int, char, double>;

    static_assert(TypeList_Size<InputTypes>::value == 3, "Size of InputTypes should be 3");
    static_assert(TypeList_IsEmpty<InputTypes>::value == false, "InputTypes should not be empty");
}

/////////////////////////////////////////////////////////////////////////////////////
SCENARIO("Test type list for count offset of type in list") {
    using InputTypes = TypeList<int, float, char>;

    static_assert(TypeList_ByteOffset<InputTypes, 0>::value == 0, "Offset of int should be 0");
    static_assert(TypeList_ByteOffset<InputTypes, 1>::value == sizeof(int), "Offset of int should be sizeof(int)");
    static_assert(TypeList_ByteOffset<InputTypes, 2>::value == sizeof(int) + sizeof(float), "Offset of int should be 12");
}

/////////////////////////////////////////////////////////////////////////////////////
SCENARIO("Test type list meta function with filter") {
    using NilTypes = TypeList<>;
    using FilteredTypes1 = TypeList_Filter<NilTypes, std::is_integral>::type;
    static_assert(std::is_same_v<FilteredTypes1, NilTypes>, "FilteredTypes1 should be empty");

    using InputTypes = TypeList<int, char, double>;
    using FilteredTypes2 = TypeList_Filter<InputTypes, std::is_integral>::type;
    static_assert(std::is_same_v<FilteredTypes2, TypeList<int, char>>, "FilteredTypes2 should contain int and char");
}

/////////////////////////////////////////////////////////////////////////////////////
// 映射器：AddPointer
template <typename T>
struct AddPointer {
    using type = T*;
};

SCENARIO("Test type list meta function with map") {
    using NilTypes = TypeList<>;
    using MappedTypes1 = TypeList_Map<NilTypes, AddPointer>::type;
    static_assert(std::is_same_v<MappedTypes1, NilTypes>, "MappedTypes1 should be empty");

    using InputTypes = TypeList<int, char, double>;
    using MappedTypes2 = TypeList_Map<InputTypes, AddPointer>::type;
    static_assert(std::is_same_v<MappedTypes2, TypeList<int*, char*, double*>>, "MappedTypes2 should contain int*, char*, double*");
}

/////////////////////////////////////////////////////////////////////////////////////
// 归约器：SumSizes
template <size_t N>
struct SizeValue {
    static const size_t value = N;
};

template <typename Acc, typename T>
struct SumSizes {
    using type = SizeValue<Acc::value + sizeof(T)>;
};

SCENARIO("Test type list meta function with reduce") {
    using NilTypes = TypeList<>;
    using TotalSize1 = TypeList_Reduce<NilTypes, SizeValue<0>, SumSizes>::type;
    static_assert(TotalSize1::value == 0, "Total size should be 0");

    using InputTypes = TypeList<int, char, double>;
    using TotalSize2 = TypeList_Reduce<InputTypes, SizeValue<0>, SumSizes>::type;
    static_assert(TotalSize2::value == sizeof(int) + sizeof(char) + sizeof(double), "Total size should be sum of all types");
}

/////////////////////////////////////////////////////////////////////////////////////
// PrintTypeIndex
template <typename T, size_t Index>
struct PrintTypeIndex {
    static void execute() {
        std::cout << "Type " << Index << " has size " << sizeof(T) << "\n";
    }
};

SCENARIO("Test type list meta function with apply") {
    using NilTypes = TypeList<>;
    TypeList_Apply<NilTypes, PrintTypeIndex>::execute();

    using InputTypes = TypeList<int, char, double>;
    TypeList_Apply<InputTypes, PrintTypeIndex>::execute();
}

/////////////////////////////////////////////////////////////////////////////////////
struct ShowParams {
    void operator()(int a, char b, double c) const {
        std::cout << "int: " << a << ", char: " << (int)b << ", double: " << c << "\n";
    }
};

struct FunctionCaller {
    template <typename Func, typename... Args>
    static void call(Func func, Args&&... args) {
        func(asl::forward<Args>(args)...);
    }
};

// CreateAndCallRec 元结构
template <typename List, typename Func, typename... Args>
struct CreateAndCallRec;

template <typename Func, typename... Args>
struct CreateAndCallRec<TypeList<>, Func, Args...> {
    static void execute(Func func) {
        FunctionCaller::call(func, Args{}...);
    }
};

template <typename Head, typename... Tail, typename Func, typename... Args>
struct CreateAndCallRec<TypeList<Head, Tail...>, Func, Args...> {
    static void execute(Func func) {
        CreateAndCallRec<TypeList<Tail...>, Func, Args..., Head>::execute(func);
    }
};

SCENARIO("Test type list for pass variables to function") {
    using MyTypes = TypeList<int, char, double>;
    CreateAndCallRec<MyTypes, ShowParams>::execute(ShowParams{});
}