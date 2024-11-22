#include "catch2/catch.hpp"
#include "tuple.h"
#include "index_seq.h"
#include "type_list.h"
#include <iostream>

using namespace asl;

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

// 测试用例自定义 Tensor 类型
template<typename T>
struct Tensor{
    T* data;
    int size;
};

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

// 模拟单个 Tensor 的初始化逻辑：入参为类型 T 对应的 Tensor 变量，以及对应的 T 所在的 index
template<typename T>
Tensor<T>& InitTensor(Tensor<T>& tensor, std::size_t index) {
    tensor.data = nullptr;
    tensor.size = static_cast<int>(index);
    return tensor;
}

// 模拟单个 Tensor 的处理逻辑：入参为类型 T 对应的 Tensor 变量
template<typename T>
void HandleAllTensors(T& tensor) {
    // 单个 Tensor 的处理逻辑
    std::cout << "Tensor of type " << typeid(T).name() << " initialized with size " << tensor.size << std::endl;
}

// 对应于没有 Tensor 时候的空处理逻辑
void HandleAllTensors() {
}

// 所有 Tensor 的处理入口逻辑：递归完成对每个 Tensor 的处理
template<typename T, typename... Ts>
void HandleAllTensors(T& first, Ts&... rest) {
    HandleAllTensors(first);
    HandleAllTensors(rest...);
}

template <typename List, std::size_t... Is>
void ProcessTypeListImpl(IndexSequence<Is...>) {
    // 定义一个 Tuple 用于存储所有 Tensor 对象
    Tuple< Tensor<typename TypeList_Get<List, Is>::type>...> tensors;
    
    // 初始化每个 Tensor
    int dummy[] = { 0, (InitTensor(TupleElemGet<Is>(tensors), Is), 0)... };
    (void)dummy; // 避免未使用变量警告
    
    // 传递所有初始化后的 Tensor 给 HandleAllTensors
    HandleAllTensors(TupleElemGet<Is>(tensors)...);
}

template <typename List>
void ProcessTypeList() {
    ProcessTypeListImpl<List>(MakeIndexSequence<TypeList_Size<List>::value>{});
}

/////////////////////////////////////////////////////////////////////////////////////
struct TypeA {};
struct TypeB {};
struct TypeC {};

SCENARIO("Test tuple generation") {
    using InputTypes = TypeList<TypeA, TypeB, TypeC>;
    ProcessTypeList<InputTypes>();
}