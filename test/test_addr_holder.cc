#include "catch2/catch.hpp"
#include <iostream>

template<int N, int M>
struct AddrHolder {
    template<typename... Args>
    AddrHolder(Args... args) {
        static_assert(sizeof...(Args) == N + M, "args size is wrong!");

        int* args_array[N + M] = { args... };

        for (std::size_t i = 0; i < N; ++i) {
            in_addrs[i] = args_array[i];
        }

        for (std::size_t i = 0; i < M; ++i) {
            out_addrs[i] = args_array[N + i];
        }
    }

    int* in_addrs[N];
    int* out_addrs[M];
};

SCENARIO("Test addr holder") {
    int a = 10, b = 20, c = 30, d = 40, e = 50;

    AddrHolder<2, 3> holder(&a, &b, &c, &d, &e);

    std::cout << "in_addrs: ";
    for (int i = 0; i < 2; ++i) {
        std::cout << *(holder.in_addrs[i]) << " ";
    }

    std::cout << "\nout_addrs: ";
    for (int i = 0; i < 3; ++i) {
        std::cout << *(holder.out_addrs[i]) << " ";
    }

    std::cout << std::endl;
}