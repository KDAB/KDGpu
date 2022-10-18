#pragma once

#include "handle.h"

#include <assert.h>
#include <limits.h>
#include <vector>

/*
    The templated Pool class allows to store a collection of objects of type T
    in a contiguous array and reference them via a typed index object of type
    Handle<H> where H is a simple tag type.

    This is useful for example if we have user-facing platform-independent code
    that wants to manage a set of Buffer objects say. If we create a pool in our
    backend Vulkan-specific code like:

    struct Buffer_t;
    using BufferPool = Pool<VkBuffer, Buffer_t>;

    Then in user-facing code we can work entirely with Handle<Buffer_t> index
    objects. This can then be used in the Vulkan-specific backend code to lookup
    the corresponding Vulkan-specific object e.g. the VkBuffer.

    Different platform-specific backends can then just use different Pool
    specializations as needed. The user-facing code will remain unchanged if
    we introduce a new graphics API backend such as Metal or Direct3D 12.
*/
template<typename T, typename H>
class Pool
{
public:
    Pool() noexcept
        : m_data(), m_generations(), m_freeIndices(), m_capacity(0)
    {
    }

    explicit Pool(std::uint32_t size)
        : m_data(), m_generations(), m_freeIndices(), m_capacity(size)
    {
        m_data.reserve(size);
        m_generations.reserve(size);
        m_freeIndices.reserve(size);
    }

    Pool(Pool const &other) = delete;
    Pool &operator=(Pool const &other) = delete;

    Pool(Pool &&other) noexcept
        : m_data(std::move(other.m_data))
        , m_generations(std::move(other.m_generations))
        , m_freeIndices(std::move(other.m_freeIndices))
        , m_capacity(std::move(other.m_capacity))
    {
        other.m_data = {};
        other.m_generations = {};
        other.m_freeIndices = {};
        other.m_capacity = 0;
    }

    Pool &operator=(Pool &&other) noexcept
    {
        m_data = std::move(other.m_data);
        m_generations = std::move(other.m_generations);
        m_freeIndices = std::move(other.m_freeIndices);
        m_capacity = std::move(other.m_capacity);

        other.m_data = {};
        other.m_generations = {};
        other.m_freeIndices = {};
        other.m_capacity = 0;

        return *this;
    }

    uint32_t capacity() const noexcept { return m_capacity; }
    uint32_t size() const noexcept { return m_data.size() - m_freeIndices.size(); }

    T *get(const Handle<H> &handle) noexcept
    {
        if (!canUseHandle(handle))
            return nullptr;
        return &m_data[handle.m_index];
    }

    const T *get(const Handle<H> &handle) const noexcept
    {
        if (!canUseHandle(handle))
            return nullptr;
        return &m_data[handle.m_index];
    }

    template<typename... Args>
    Handle<H> emplace(Args &&...args)
    {
        if (m_data.size() >= m_capacity)
            growCapacity();

        if (m_freeIndices.size() > 0) {
            // We have a gap in the m_data vector, use that.
            Handle<H> handle;
            handle.m_index = m_freeIndices.back();
            m_freeIndices.pop_back();
            handle.m_generation = m_generations[handle.m_index].generation; // The generation was already bumped when this entry was removed
            m_generations[handle.m_index].isAlive = true;

            // Use placement new
            new (m_data.data() + handle.m_index) T(std::forward<Args>(args)...);

            return handle;
        } else {
            // No gaps in the m_data vector, add a new element at the end
            m_data.emplace_back(args...);
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

        // Explicitly destroy the data object (counterpart of placement new)
        m_data[handle.m_index].~T();

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
        if (entryIndex >= capacity() || m_generations[entryIndex].isAlive == false)
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
