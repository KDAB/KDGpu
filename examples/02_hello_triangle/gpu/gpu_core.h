#pragma once

#include <stdint.h>

namespace Gpu {

constexpr uint32_t maxAdapterNameSize = 256U;
constexpr uint32_t UuidSize = 16U;

using DeviceSize = uint64_t;
using Flags = uint32_t;

enum class SampleCountFlagBits {
    Samples1Bit = 0x00000001,
    Samples2Bit = 0x00000002,
    Samples4Bit = 0x00000004,
    Samples8Bit = 0x00000008,
    Samples16Bit = 0x00000010,
    Samples32Bit = 0x00000020,
    Samples64Bit = 0x00000040,
    MaxEnum = 0x7FFFFFFF
};
using SampleCountFlags = Flags;

enum class AdapterDeviceType {
    Other = 0,
    IntegratedGpu = 1,
    DiscreteGpu = 2,
    VirtualGpu = 3,
    Cpu = 4,
    MaxEnum = 0x7FFFFFFF
};

} // namespace Gpu
