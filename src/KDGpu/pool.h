#pragma once

#include "handle.h"

#include <assert.h>
#include <limits>
#include <vector>
#include <KDGpu/utils/logging.h>

namespace KDGpu {

/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

/**
 * @brief Pool
 * @internal
 */
template<typename T, typename H>
class Pool
{
public:
    Pool() noexcept
        : m_data(), m_generations(), m_freeIndices(), m_capacity(0)
    {
    }

    explicit Pool(uint32_t size)
        : m_data(), m_generations(), m_freeIndices(), m_capacity(size)
    {
        m_data.reserve(size);
        m_generations.reserve(size);
        m_freeIndices.reserve(size);
    }

    Pool(Pool const &other) = delete;
    Pool &operator=(Pool const &other) = delete;

    Pool(Pool &&other) noexcept
        : m_data(std::exchange(other.m_data, {}))
        , m_generations(std::exchange(other.m_generations, {}))
        , m_freeIndices(std::exchange(other.m_freeIndices, {}))
        , m_capacity(std::exchange(other.m_capacity, 0))
    {
    }

    Pool &operator=(Pool &&other) noexcept
    {
        m_data = std::exchange(other.m_data, {});
        m_generations = std::exchange(other.m_generations, {});
        m_freeIndices = std::exchange(other.m_freeIndices, {});
        m_capacity = std::exchange(other.m_capacity, 0);

        return *this;
    }

    uint32_t capacity() const noexcept { return m_capacity; }
    uint32_t size() const noexcept { return m_data.size() - m_freeIndices.size(); }

    T *get(const Handle<H> &handle) const noexcept
    {
        if (!canUseHandle(handle))
            return nullptr;
        return const_cast<T *>(&m_data[handle.m_index]);
    }

    template<typename... Args>
    Handle<H> emplace(Args &&...args)
    {
        if (size() >= m_capacity)
            growCapacity();

        if (m_freeIndices.size() > 0) {
            // We have a gap in the m_data vector, use that.
            Handle<H> handle;
            handle.m_index = m_freeIndices.back();
            m_freeIndices.pop_back();
            handle.m_generation = m_generations[handle.m_index].generation; // The generation was already bumped when this entry was removed
            m_generations[handle.m_index].isAlive = true;
            m_data[handle.m_index] = T(std::forward<Args>(args)...);

            return handle;
        } else {

            // SPDLOG_LOGGER_WARN(Logger::logger(), "Grow {} {} {}", __FUNCTION__, typeid(T).name(), m_data.size());

            // No gaps in the m_data vector, add a new element at the end
            m_data.emplace_back(std::forward<Args>(args)...);
            m_generations.emplace_back(GenerationEntry{ 1, true });

            Handle<H> handle(m_data.size() - 1, 1);
            return handle;
        }
    }

    Handle<H> insert(const T &data)
    {
        return emplace(data);
    }

    void remove(const Handle<H> &handle)
    {
        if (!canUseHandle(handle))
            return;

        // The contained data dtor is not called here, we simply mark the slot as available
        // for reuse. So if you need that, this Pool is not the Pool you are looking for.
        // The dtor will only be called when the entire pool goes out of scope.

        // Bump the generation so we know not to deref this data from any existing handles
        auto &generation = m_generations[handle.m_index];
        ++generation.generation;
        generation.isAlive = false;

        // Store the position of the unused gap in the data array
        m_freeIndices.push_back(handle.m_index);
    }

    void clear()
    {
        const uint32_t dataSize = static_cast<uint32_t>(m_data.size());
        for (uint32_t i = 0; i < dataSize; ++i) {
            const auto handle = handleForIndex(i);
            remove(handle);
        }
    }

    // Convert an entry index into a Handle<H>, if possible otherwise returns an invalid handle
    Handle<H> handleForIndex(uint32_t entryIndex) const
    {
        if (entryIndex >= m_generations.size() || m_generations[entryIndex].isAlive == false)
            return {};
        return Handle<H>{ entryIndex, m_generations[entryIndex].generation };
    }

private:
    bool canUseHandle(const Handle<H> &handle) const noexcept
    {
        return handle.m_index < m_data.size() && handle.m_generation == m_generations[handle.m_index].generation && m_generations[handle.m_index].isAlive;
    }

    void growCapacity();

    struct GenerationEntry {
        uint32_t generation{ 0 };
        bool isAlive{ false };
    };

    std::vector<T> m_data;
    std::vector<GenerationEntry> m_generations;
    std::vector<uint32_t> m_freeIndices;
    uint32_t m_capacity;
};

template<typename T, typename H>
void Pool<T, H>::growCapacity()
{
    // Keep it simple for now and just double the capacity when we need to grow
    m_capacity *= 2;
    if (m_capacity == 0)
        m_capacity = 1;
    assert(m_capacity < std::numeric_limits<uint32_t>::max());
    m_data.reserve(m_capacity);
    m_generations.reserve(m_capacity);
    m_freeIndices.reserve(m_capacity);
}

} // namespace KDGpu
