#pragma once

#include <stdint.h>
#include <string>

#pragma once

#define SERENITY_MAKE_API_VERSION(variant, major, minor, patch) \
    ((((uint32_t)(variant)) << 29) | (((uint32_t)(major)) << 22) | (((uint32_t)(minor)) << 12) | ((uint32_t)(patch)))

namespace ToyRenderer {

constexpr uint32_t maxAdapterNameSize = 256U;
constexpr uint32_t UuidSize = 16U;

using DeviceSize = uint64_t;
using Flags = uint32_t;

struct Extent2D {
    uint32_t width;
    uint32_t height;
};

struct Extent3D {
    uint32_t width;
    uint32_t height;
    uint32_t depth;
};

struct Offset2D {
    int32_t x;
    int32_t y;
};

struct Offset3D {
    int32_t x;
    int32_t y;
    int32_t z;
};

struct Rect2D {
    Offset2D offset;
    Extent2D extent;
};

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

inline std::string adapterDeviceTypeToString(AdapterDeviceType deviceType)
{
    switch (deviceType) {
    case ToyRenderer::AdapterDeviceType::Other:
        return "Other Device Type";

    case ToyRenderer::AdapterDeviceType::IntegratedGpu:
        return "Integrated GPU";

    case ToyRenderer::AdapterDeviceType::DiscreteGpu:
        return "Discrete GPU";

    case ToyRenderer::AdapterDeviceType::VirtualGpu:
        return "Virtual GPU";

    case ToyRenderer::AdapterDeviceType::Cpu:
        return "CPU";

    default:
        return "Unknown device type";
    }
}

enum class QueueFlagBits {
    GraphicsBit = 0x00000001,
    ComputeBit = 0x00000002,
    TransferBit = 0x00000004,
    SparseBindingBit = 0x00000008,
    ProtectedBit = 0x00000010,
    VideoDecodeBit = 0x00000020,
    VideoEncodeBit = 0x00000040,
    MaxEnum = 0x7FFFFFFF
};
using QueueFlags = Flags;
// TODO: Use Flags<QueueFlagBits>

} // namespace ToyRenderer
