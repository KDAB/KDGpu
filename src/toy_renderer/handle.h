#pragma once

#include <toy_renderer/utils/hash_utils.h>

#include <stdint.h>

template<typename T>
class Handle
{
public:
    Handle()
        : m_index(0)
        , m_generation(0)
    {
    }

    bool isValid() const noexcept { return m_generation != 0; }

    uint32_t index() const noexcept { return m_index; }
    uint32_t generation() const noexcept { return m_generation; }

private:
    explicit Handle(uint32_t index, uint32_t generation)
        : m_index(index)
        , m_generation(generation)
    {
    }

    uint32_t m_index;
    uint32_t m_generation;

    template<typename U, typename V>
    friend class Pool;
};

template<typename T>
bool operator==(const Handle<T> &lhs, const Handle<T> &rhs)
{
    return lhs.index() == rhs.index() && lhs.generation() == rhs.generation();
}

template<typename T>
bool operator!=(const Handle<T> &lhs, const Handle<T> &rhs)
{
    return !(lhs == rhs);
}

namespace std {

template<typename T>
struct hash<Handle<T>> {
    size_t operator()(const Handle<T> &handle) const
    {
        uint64_t hash = 0;

        ToyRenderer::hash_combine(hash, handle.index());
        ToyRenderer::hash_combine(hash, handle.generation());

        return hash;
    }
};

} // namespace std
