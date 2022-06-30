#pragma once

#include <stdint.h>

template<typename T>
class Handle
{
public:
    Handle()
        : m_index(0)
        , m_generation(0)
    {}
    
    bool isValid() const noexcept { return m_generation != 0; }

private:
    explicit Handle(uint32_t index, uint32_t generation)
        : m_index(index)
        , m_generation(generation)
    {}

    uint32_t m_index;
    uint32_t m_generation;

    template<typename U, typename V>
    friend class Pool;
};
