/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDFoundation/hashutils.h>
#include <cassert>

namespace KDGpu {

/**
 * @brief Handle
 * @ingroup public
 */
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

protected:
    explicit Handle(uint32_t index, uint32_t generation)
        : m_index(index)
        , m_generation(generation)
    {
    }

private:
    uint32_t m_index;
    uint32_t m_generation;

    template<typename U, typename V>
    friend class Pool;
};

template<typename T>
bool operator==(const KDGpu::Handle<T> &lhs, const KDGpu::Handle<T> &rhs)
{
    return lhs.index() == rhs.index() && lhs.generation() == rhs.generation();
}

template<typename T>
bool operator!=(const KDGpu::Handle<T> &lhs, const KDGpu::Handle<T> &rhs)
{
    return !(lhs == rhs);
}

template<typename T>
bool operator<(const KDGpu::Handle<T> &lhs, const KDGpu::Handle<T> &rhs)
{
    return lhs.index() < rhs.index();
}

template<typename T>
class RequiredHandle : public Handle<T>
{
public:
#ifdef KDGPU_STRICT_MODE
    RequiredHandle() = delete;
#endif
    using Handle<T>::Handle;

    RequiredHandle(const Handle<T> &handle)
        : Handle<T>(handle.index(), handle.generation())
    {
#ifdef KDGPU_STRICT_MODE
        assert(this->isValid());
#endif
    }

    template<typename U, typename = std::enable_if_t<!std::is_same_v<U, Handle<T>> && std::is_convertible_v<U, Handle<T>>>>
    RequiredHandle(const U &obj)
        : Handle<T>(Handle<T>(obj))
    {
#ifdef KDGPU_STRICT_MODE
        assert(this->isValid());
#endif
    }
};

template<typename T>
using OptionalHandle = Handle<T>;

} // namespace KDGpu

namespace std {

template<typename T>
struct hash<KDGpu::Handle<T>> {
    size_t operator()(const KDGpu::Handle<T> &handle) const
    {
        uint64_t hash = 0;

        KDFoundation::hash_combine(hash, handle.index());
        KDFoundation::hash_combine(hash, handle.generation());

        return hash;
    }
};

template<typename T>
struct hash<KDGpu::RequiredHandle<T>> {
    size_t operator()(const KDGpu::RequiredHandle<T> &handle) const
    {
        uint64_t hash = 0;

        KDFoundation::hash_combine(hash, handle.index());
        KDFoundation::hash_combine(hash, handle.generation());

        return hash;
    }
};

} // namespace std
