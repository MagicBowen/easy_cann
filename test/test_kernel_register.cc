#include "catch2/catch.hpp"
#include <type_traits>
#include <iostream>
#include "type_list.h"
#include "forward.h"
#include "tuple.h"
#include "index_seq.h"

using namespace asl;

using Addr = unsigned char*;

template<typename T>
struct Tensor {
    T* data;
    std::size_t size;
};

template <typename T>
struct TypeToTensor {
    using type = Tensor<T>;
};

template <typename List>
struct TensorTuple {
private:
    using Tensors = typename TypeList_Map<List, TypeToTensor>::type;
public:
    using type = typename TupleFromTypeList<Tensors>::type;
};

/////////////////////////////////////////////////////////////////////////////
enum class ParamType {
    INPUT,
    OUTPUT,
    TEMP,
};

template<ParamType PT, typename ... Ts>
struct ParamTypes{
    using types = TypeList<Ts...>;
    static constexpr ParamType usage = PT;
};

template<typename ... Ts>
using Input = ParamTypes<ParamType::INPUT, Ts...>;

template<typename ... Ts>
using Output = ParamTypes<ParamType::OUTPUT, Ts...>;

template<typename ... Ts>
using Temp = ParamTypes<ParamType::TEMP, Ts...>;

///////////////////////////////////////////////////////////////////////////////
enum class KernelType {
    ELEM_WISE,
    REDUCE,
};

// template <KernelType KT, typename OP, typename ...>
// struct KernelExecutor;

template <typename OP, typename INPUT_TYPES, typename OUTPUT_TYPES, typename TEMP_TYPES = Temp<>>
class ElemWise {

    static_assert(INPUT_TYPES::usage  == ParamType::INPUT, "INPUT_TYPES should be INPUT");
    static_assert(OUTPUT_TYPES::usage == ParamType::OUTPUT, "OUTPUT_TYPES should be OUTPUT");
    static_assert(TEMP_TYPES::usage   == ParamType::TEMP, "TEMP_TYPES should be TEMP");

    using INPUTS  = typename INPUT_TYPES::types;
    using OUTPUTS = typename OUTPUT_TYPES::types;
    using TEMPS   = typename TEMP_TYPES::types;

    static constexpr std::size_t INPUT_COUNT  = TypeList_Size<INPUTS>::value;
    static constexpr std::size_t OUTPUT_COUNT = TypeList_Size<OUTPUTS>::value;
    static constexpr std::size_t TEMP_COUNT   = TypeList_Size<TEMPS>::value;
    static constexpr std::size_t ADDR_COUNT   = INPUT_COUNT + OUTPUT_COUNT;

public:
    // template <typename... Args>
    // KernelExecutor(Args&&... args) {

    //     static_assert(sizeof...(Args) == INPUT_COUNT + OUTPUT_COUNT, "args size is wrong!");

    //     FillAddrs(asl::forward<Args>(args)...);

    //     FillOffsets<INPUTS>(inOffsets_);
    //     FillOffsets<OUTPUTS>(outOffsets_);
    //     FillOffsets<TEMPS>(tempOffsets_);
    // }

    template <typename... Args>
    void Run(Args&&... args) {
        
        static_assert(sizeof...(Args) > ADDR_COUNT, "args size is wrong!");

        auto argsTuple = ForwardAsTuple(std::forward<Args>(args)...);

        unsigned int count = TupleElemGet<ADDR_COUNT>(argsTuple);

        FillAddrs(argsTuple, MakeIndexSequence<ADDR_COUNT>{});

        FillOffsets<INPUTS>(inOffsets_);
        FillOffsets<OUTPUTS>(outOffsets_);
        FillOffsets<TEMPS>(tempOffsets_);

        typename TensorTuple<INPUTS>::type inTensors;
        typename TensorTuple<OUTPUTS>::type outTensors;
        typename TensorTuple<TEMPS>::type tempTensors;
    
        InitInputTensors(inTensors, count, MakeIndexSequence<INPUT_COUNT>{});
        InitOutputTensors(outTensors, count, MakeIndexSequence<OUTPUT_COUNT>{});
        InitTempTensors(tempTensors, count, MakeIndexSequence<TEMP_COUNT>{});

        Compute(inTensors, outTensors, tempTensors, argsTuple,
                MakeIndexSequence<INPUT_COUNT>{}, 
                MakeIndexSequence<OUTPUT_COUNT>{}, 
                MakeIndexSequence<TEMP_COUNT>{}, 
                MakeIndexSequence<sizeof...(Args) - ADDR_COUNT - 1>{},
                count);
    }

private:
    template <typename TUPLE, std::size_t... Is>
    void InitInputTensors(TUPLE& tuple, std::size_t cnt, IndexSequence<Is...>) {
        // 初始化每个 Tensor
        int dummy[] = { 0, (InitInputTensor(TupleElemGet<Is>(tuple), cnt, Is), 0)... };
        (void)dummy; // 避免未使用变量警告
    }

    template <typename TUPLE, std::size_t... Is>
    void InitOutputTensors(TUPLE& tuple, std::size_t cnt, IndexSequence<Is...>) {
        int dummy[] = { 0, (InitOutputTensor(TupleElemGet<Is>(tuple), cnt, Is), 0)... };
        (void)dummy;
    }

    template <typename TUPLE, std::size_t... Is>
    void InitTempTensors(TUPLE& tuple, std::size_t cnt, IndexSequence<Is...>) {
        int dummy[] = { 0, (InitTempTensor(TupleElemGet<Is>(tuple), cnt, Is), 0)... };
        (void)dummy;
    }

    template <typename T>
    Tensor<T>& InitInputTensor(Tensor<T>& tensor, std::size_t cnt, std::size_t index) {
        tensor.data = reinterpret_cast<T*>(inAddrs_[index] + inOffsets_[index] * cnt);
        tensor.size = sizeof(T) * cnt;
        return tensor;
    }

    template <typename T>
    Tensor<T>& InitOutputTensor(Tensor<T>& tensor, std::size_t cnt, std::size_t index) {
        tensor.data = reinterpret_cast<T*>(outAddrs_[index] + outOffsets_[index] * cnt);
        tensor.size = sizeof(T) * cnt;
        return tensor;
    }

    template <typename T>
    Tensor<T>& InitTempTensor(Tensor<T>& tensor, std::size_t cnt, std::size_t index) {
        tensor.size = sizeof(T) * cnt;
        return tensor;
    }

    template<typename IN_TUPLE, typename OUT_TUPLE, typename TMP_TUPLE, typename ArgsType, 
            std::size_t... I1, std::size_t... I2,  std::size_t... I3, std::size_t... I4>
    void Compute(IN_TUPLE& inTensors, OUT_TUPLE& outTensors, TMP_TUPLE& tempTensors, ArgsType&& args,
                 IndexSequence<I1...>, IndexSequence<I2...>, IndexSequence<I3...>, IndexSequence<I4...>, 
                 std::size_t cnt) {
        op_(TupleElemGet<I1>(inTensors)..., 
            TupleElemGet<I2>(outTensors)..., 
            TupleElemGet<I3>(tempTensors)..., 
            cnt, 
            TupleElemGet<ADDR_COUNT + 1 + I4>(std::forward<ArgsType>(args))...);
    }

private:
    template <typename TupleType, std::size_t... Is>
    void FillAddrs(TupleType& tuple, IndexSequence<Is...>) {
        Addr argsArr[ADDR_COUNT] = { TupleElemGet<Is>(tuple)... };
        for (std::size_t i = 0; i < INPUT_COUNT; ++i) {
            inAddrs_[i] = argsArr[i];
        }
        for (std::size_t i = 0; i < OUTPUT_COUNT; ++i) {
            outAddrs_[i] = argsArr[INPUT_COUNT + i];
        }
    }

    // 填充 offset 到数组
    template <typename List, std::size_t... Is>
    constexpr void FillOffsetsImpl(std::size_t* offsets, IndexSequence<Is...>) {
        ((offsets[Is] = TypeList_ByteOffset<List, Is>::value), ...);
    }

    template <typename List>
    constexpr void FillOffsets(std::size_t* offsets) {
        constexpr std::size_t count = TypeList_Size<List>::value;
        FillOffsetsImpl<List>(offsets, MakeIndexSequence<count>{}); 
    }

private:
    OP op_;

    Addr inAddrs_[INPUT_COUNT];
    Addr outAddrs_[OUTPUT_COUNT];

    std::size_t inOffsets_[INPUT_COUNT];
    std::size_t outOffsets_[OUTPUT_COUNT];
    std::size_t tempOffsets_[TEMP_COUNT];
};

///////////////////////////////////////////////////////////////////////////////
struct KernelAdd {
    template <typename T>
    void operator()(Tensor<T> x, Tensor<T> y, Tensor<T> z, std::size_t cnt) {
        std::cout << "Add: x.size = " << x.size << ", x.type = " << typeid(T).name()
                  << ", y.size = " << y.size << ", y.type = " << typeid(T).name()
                  << ", z.size = " << z.size << ", z.type = " << typeid(T).name()
                  << ", cnt = " << cnt << std::endl;
    }
};

struct KernelSub {
    template<typename T1, typename T2, typename T3, typename T4>
    void operator()(Tensor<T1> x, Tensor<T2> y, Tensor<T3> z, Tensor<T4> tmpTensor, std::size_t cnt, bool cond) {
        std::cout << "Sub: x.size = " << x.size << ", x.type = " << typeid(T1).name()
                  << ", y.size = " << y.size << ", y.type = " << typeid(T2).name()
                  << ", z.size = " << z.size << ", z.type = " << typeid(T3).name()
                  << ", tmpTensor.size = " << tmpTensor.size << ", tmpTensor.type = " << typeid(T4).name()
                  << ", cnt = " << cnt 
                  << ", cond = " << cond << std::endl;
    }
};

struct KernelTriple {
    template<typename T1, typename T2, typename T3, typename T4, typename T5>
    void operator()(Tensor<T1> x, Tensor<T2> y, Tensor<T3> z, Tensor<T4> d, Tensor<T5> tmpTensor, std::size_t cnt, const std::string& log) {
        std::cout << "Sub: x.size = " << x.size << ", x.type = " << typeid(T1).name()
                  << ", y.size = " << y.size << ", y.type = " << typeid(T2).name()
                  << ", z.size = " << z.size << ", z.type = " << typeid(T3).name()
                  << ", d.size = " << d.size << ", d.type = " << typeid(T4).name()
                  << ", tmpTensor.size = " << tmpTensor.size << ", tmpTensor.type = " << typeid(T5).name()
                  << ", cnt = " << cnt 
                  << ", log = " << log << std::endl;
    }
};

SCENARIO("Test kernel register by template") {
    unsigned char x[10];
    unsigned char y[10];
    unsigned char z[10];
    unsigned char d[10];

    ElemWise<KernelAdd, Input<int, int>, Output<int>> AddKernel;
    AddKernel.Run(x, y, z, 10);

    ElemWise<KernelSub, Input<int, char>, Output<float>, Temp<float>> SubKernel;
    SubKernel.Run(x, y, z, 10, true);

    ElemWise<KernelTriple, Input<char, int, long long>, Output<float>, Temp<unsigned short>> TripleKernel;
    TripleKernel.Run(x, y, z, d, 10, "hello");
}
