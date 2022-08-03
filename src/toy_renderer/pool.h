#pragma once

#include "handle.h"

#include <vector>

template<typename T, typename H>
class Pool
{
public:
    template<typename... Args>
    Handle<H> emplace(Args &&...args)
    {
        if (m_freeListPosition >= m_size)
            grow();

        Handle<H> handle;
        handle.m_index = m_freeList[m_freeListPosition];
        handle.m_generation = m_generations[handle.m_index];
        --m_freeListPosition;

        // Use placement new
        new (m_data + handle.m_index) T(std::forward<Args>(args)...);

        return handle;
    }

    Handle<H> insert(const T &data)
    {
        return emplace(data);
    }

private:
    void grow();

    std::vector<T> m_data;
    std::vector<uint32_t> m_generations;
    std::vector<uint32_t> m_freeList;
    uint32_t m_freeListPosition{ 0 };
    uint32_t m_size{ 0 };
};
