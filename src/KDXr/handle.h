/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDXr/utils/hash_utils.h>

#include <stdint.h>

namespace KDXr {

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
bool operator==(const KDXr::Handle<T> &lhs, const KDXr::Handle<T> &rhs)
{
    return lhs.index() == rhs.index() && lhs.generation() == rhs.generation();
}

template<typename T>
bool operator!=(const KDXr::Handle<T> &lhs, const KDXr::Handle<T> &rhs)
{
    return !(lhs == rhs);
}

template<typename T>
bool operator<(const KDXr::Handle<T> &lhs, const KDXr::Handle<T> &rhs)
{
    return lhs.index() < rhs.index();
}

} // namespace KDXr

namespace std {

template<typename T>
struct hash<KDXr::Handle<T>> {
    size_t operator()(const KDXr::Handle<T> &handle) const
    {
        uint64_t hash = 0;

        KDXr::hash_combine(hash, handle.index());
        KDXr::hash_combine(hash, handle.generation());

        return hash;
    }
};

} // namespace std
