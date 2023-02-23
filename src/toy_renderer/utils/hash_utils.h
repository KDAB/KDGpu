#pragma once

#include <functional>
#include <stdint.h>

namespace ToyRenderer {

template<class T>
inline void hash_combine(uint64_t &seed, const T &v)
{
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

} // namespace ToyRenderer
