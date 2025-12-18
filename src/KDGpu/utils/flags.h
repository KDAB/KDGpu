/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <type_traits>

namespace KDGpu {

template<typename E>
class Flags
{
public:
    // Use int as storage type as MSVC is buggy and even if enum is unsigned
    // each enum entry will be signed.
    using FlagsInt = typename std::underlying_type<E>::type;
    using Enum = E;

    constexpr Flags() noexcept = default;

    constexpr Flags(E flag) noexcept
        : m_flags(FlagsInt(flag))
    {
    }

    constexpr explicit operator bool() const noexcept { return m_flags; }

    constexpr FlagsInt toInt() const noexcept { return m_flags; }
    constexpr static Flags fromInt(FlagsInt i) noexcept
    {
        Flags f;
        f.m_flags = i;
        return f;
    }

    constexpr Flags &operator&=(E mask) noexcept
    {
        m_flags &= FlagsInt(mask);
        return *this;
    }

    constexpr Flags &operator&=(Flags flags) noexcept
    {
        m_flags &= flags.m_flags;
        return *this;
    }

    constexpr Flags &operator|=(E mask) noexcept
    {
        m_flags |= FlagsInt(mask);
        return *this;
    }
    constexpr Flags &operator|=(Flags flags) noexcept
    {
        m_flags |= flags.m_flags;
        return *this;
    }

    constexpr Flags &operator^=(E mask) noexcept
    {
        m_flags ^= FlagsInt(mask);
        return *this;
    }
    constexpr Flags &operator^=(Flags flags) noexcept
    {
        m_flags ^= flags.m_flags;
        return *this;
    }

    constexpr Flags operator&(E mask) const noexcept { return Flags::fromInt(m_flags & FlagsInt(mask)); }
    constexpr Flags operator&(Flags flags) const noexcept { return Flags::fromInt(m_flags & flags.m_flags); }

    constexpr Flags operator|(E mask) const noexcept { return Flags::fromInt(m_flags | FlagsInt(mask)); }
    constexpr Flags operator|(Flags flags) const noexcept { return Flags::fromInt(FlagsInt(m_flags) | flags.m_flags); }

    constexpr Flags operator^(E mask) const noexcept { return Flags::fromInt(FlagsInt(m_flags) ^ FlagsInt(mask)); }
    constexpr Flags operator^(Flags flags) const noexcept { return Flags::fromInt(m_flags ^ flags.m_flags); }

    constexpr Flags operator~() const noexcept { return Flags::fromInt(~m_flags); }

    constexpr bool testFlag(E flag) const noexcept
    {
        const FlagsInt fInt = static_cast<FlagsInt>(flag);
        return ((m_flags & fInt) == fInt) && (fInt != 0 || m_flags == fInt);
    }

    constexpr Flags &setFlag(E flag, bool enabled = true) noexcept
    {
        if (enabled)
            *this |= flag;
        else
            *this &= ~Flags(flag);
        return *this;
    }

    constexpr void operator+(Flags) const noexcept = delete;
    constexpr void operator+(E) const noexcept = delete;
    constexpr void operator+(int) const noexcept = delete;
    constexpr void operator+(unsigned int) const noexcept = delete;

    constexpr void operator-(Flags) const noexcept = delete;
    constexpr void operator-(E) const noexcept = delete;
    constexpr void operator-(int) const noexcept = delete;
    constexpr void operator-(unsigned int) const noexcept = delete;

    friend constexpr bool operator==(Flags l, Flags r) noexcept { return l.m_flags == r.m_flags; }
    friend constexpr bool operator==(Flags l, E r) noexcept { return l == Flags(r); }
    friend constexpr bool operator==(E l, Flags r) noexcept { return Flags(l) == r; }

    friend constexpr bool operator!=(Flags l, Flags r) noexcept { return !(l == r); }
    friend constexpr bool operator!=(Flags l, E r) noexcept { return !(l == r); }
    friend constexpr bool operator!=(E l, Flags r) noexcept { return !(l == r); }

    // Operator < is not usually defined for flags, as it does not make sense in the context of bitwise flags.
    // However, sometimes it is useful if you want to use flags in sorted containers or algorithms.
    friend constexpr bool operator<(Flags l, Flags r) noexcept { return l.m_flags < r.m_flags; }

private:
    FlagsInt m_flags = FlagsInt(0);
};

} // namespace KDGpu

// clang-format off
#define OPERATORS_FOR_FLAGS(Flags)                                                                   \
    constexpr Flags operator|(Flags::Enum a, Flags::Enum b) noexcept { return Flags(a) | b; } \
    constexpr Flags operator|(Flags::Enum a, Flags b) noexcept { return b | a; }              \
    constexpr Flags operator&(Flags::Enum a, Flags::Enum b) noexcept { return Flags(a) & b; } \
    constexpr Flags operator&(Flags::Enum a, Flags b) noexcept { return b & a; }              \
    constexpr void operator+(Flags::Enum, Flags::Enum) noexcept = delete;                     \
    constexpr void operator+(Flags::Enum, Flags) noexcept = delete;                           \
    constexpr void operator+(int, Flags) noexcept = delete;                                   \
    constexpr void operator-(Flags::Enum, Flags::Enum) noexcept = delete;                     \
    constexpr void operator-(Flags::Enum, Flags) noexcept = delete;                           \
    constexpr void operator-(int, Flags) noexcept = delete;                                   \
    constexpr void operator+(int, Flags::Enum) noexcept = delete;                             \
    constexpr void operator+(Flags::Enum, int) noexcept = delete;                             \
    constexpr void operator-(int, Flags::Enum) noexcept = delete;                             \
    constexpr void operator-(Flags::Enum, int) noexcept = delete;                             \
    namespace std {                                                                                  \
        template<> struct hash<Flags> {                                                              \
            std::size_t operator()(Flags const &f) const noexcept { return std::size_t(f.toInt()); } \
        };                                                                                           \
    } // namespace std

// clang-format on
