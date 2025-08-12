/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/utils/flags.h>

#include <stdint.h>
#include <string>
#include <variant>

using HANDLE = void *;

// clang-format off
#define KDGPU_MAKE_API_VERSION(variant, major, minor, patch) \
    ((((uint32_t)(variant)) << 29) | (((uint32_t)(major)) << 22) | (((uint32_t)(minor)) << 12) | ((uint32_t)(patch)))

#define KDGPU_API_VERSION_VARIANT(version) ((uint32_t)(version) >> 29U)
#define KDGPU_API_VERSION_MAJOR(version) (((uint32_t)(version) >> 22U) & 0x7FU)
#define KDGPU_API_VERSION_MINOR(version) (((uint32_t)(version) >> 12U) & 0x3FFU)
#define KDGPU_API_VERSION_PATCH(version) ((uint32_t)(version) & 0xFFFU)
// clang-format on

namespace KDGpu {

/**
 * @defgroup public Public API
 *
 * Holds the Public API
 */

/*! \addtogroup public
 *  @{
 */

constexpr uint32_t maxAdapterNameSize = 256U;
constexpr uint32_t UuidSize = 16U;
constexpr uint32_t remainingArrayLayers = (~0U);
constexpr uint32_t remainingMipLevels = (~0U);
constexpr uint32_t IgnoreQueueType = (~0U);
constexpr uint64_t WholeSize = (~0ULL);
constexpr uint32_t ExternalSubpass = ~(0U);

using DeviceSize = uint64_t;
using SampleMask = uint32_t;
using TimestampIndex = uint32_t;
using BufferDeviceAddress = uint64_t;

using HandleOrFD = std::variant<std::monostate, int, HANDLE>;

struct MemoryHandle {
    HandleOrFD handle;
    size_t allocationSize{ 0 };
    size_t allocationOffset{ 0 };

    friend bool operator==(const MemoryHandle &, const MemoryHandle &) = default;
};

struct Extension {
    std::string name;
    uint32_t version{ 0 };

    friend bool operator==(const Extension &, const Extension &) = default;
};

struct Extent2D {
    uint32_t width{ 0 };
    uint32_t height{ 0 };

    friend bool operator==(const Extent2D &, const Extent2D &) = default;
};

struct Extent2Df {
    float width{ 0.0f };
    float height{ 0.0f };

    friend bool operator==(const Extent2Df &, const Extent2Df &) = default;
};

struct Extent3D {
    uint32_t width{ 0 };
    uint32_t height{ 0 };
    uint32_t depth{ 0 };

    friend bool operator==(const Extent3D &, const Extent3D &) = default;
};

struct Offset2D {
    int32_t x{ 0 };
    int32_t y{ 0 };

    friend bool operator==(const Offset2D &, const Offset2D &) = default;
};

struct Offset3D {
    int32_t x{ 0 };
    int32_t y{ 0 };
    int32_t z{ 0 };

    friend bool operator==(const Offset3D &, const Offset3D &) = default;
};

struct Rect2D {
    Offset2D offset;
    Extent2D extent;

    friend bool operator==(const Rect2D &, const Rect2D &) = default;
};

union ColorClearValue {
    float float32[4];
    int32_t int32[4];
    uint32_t uint32[4];

    friend bool operator==(const ColorClearValue &, const ColorClearValue &);
};

inline bool operator==(const ColorClearValue &lhs, const ColorClearValue &rhs)
{
    return lhs.uint32[0] == rhs.uint32[0] //
            && lhs.uint32[1] == rhs.uint32[1] //
            && lhs.uint32[2] == rhs.uint32[2] //
            && lhs.uint32[3] == rhs.uint32[3];
}

struct DepthStencilClearValue {
    float depthClearValue{ 1.0f };
    uint32_t stencilClearValue{ 0 };
};

struct Viewport {
    float x{ 0.0f };
    float y{ 0.0f };
    float width;
    float height;
    float minDepth{ 0.0f };
    float maxDepth{ 1.0f };

    friend bool operator==(const Viewport &, const Viewport &) = default;
};

struct SpecializationConstant {
    uint32_t constantId;
    std::variant<bool, int32_t, uint32_t, float, double> value;

    friend bool operator==(const SpecializationConstant &, const SpecializationConstant &) = default;
};

struct MemoryRequirement {
    DeviceSize size;
    DeviceSize alignment;
    int memoryTypeBits;
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
using SampleCountFlags = KDGpu::Flags<SampleCountFlagBits>;

enum class AdapterDeviceType {
    Other = 0,
    IntegratedGpu = 1,
    DiscreteGpu = 2,
    VirtualGpu = 3,
    Cpu = 4,
    Default = 0x7FFFFFFE,
    MaxEnum = 0x7FFFFFFF
};

inline std::string adapterDeviceTypeToString(AdapterDeviceType deviceType)
{
    switch (deviceType) {
    case KDGpu::AdapterDeviceType::Other:
        return "Other Device Type";

    case KDGpu::AdapterDeviceType::Default:
        return "Default (platform) GPU";

    case KDGpu::AdapterDeviceType::IntegratedGpu:
        return "Integrated GPU";

    case KDGpu::AdapterDeviceType::DiscreteGpu:
        return "Discrete GPU";

    case KDGpu::AdapterDeviceType::VirtualGpu:
        return "Virtual GPU";

    case KDGpu::AdapterDeviceType::Cpu:
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
using QueueFlags = KDGpu::Flags<QueueFlagBits>;

enum class PresentMode {
    Immediate = 0,
    Mailbox = 1,
    Fifo = 2,
    FifoRelaxed = 3,
    SharedDemandRefresh = 1000111000,
    SharedContinuousRefresh = 1000111001,
    MaxEnum = 0x7FFFFFFF
};

inline std::string presentModeToString(PresentMode presentMode)
{
    switch (presentMode) {
    case PresentMode::Immediate:
        return "Immediate";
    case PresentMode::Mailbox:
        return "Mailbox";
    case PresentMode::Fifo:
        return "Fifo";
    case PresentMode::FifoRelaxed:
        return "Fifo Relaxed";
    case PresentMode::SharedDemandRefresh:
        return "Shared Demand Refresh";
    case PresentMode::SharedContinuousRefresh:
        return "Shared Continuous Refresh";
    default:
        return "Unknown";
    }
}

enum class ColorSpace {
    SRgbNonlinear = 0,
    DisplayP3Nonlinear = 1000104001,
    ExtendedSRgbLinear = 1000104002,
    DisplayP3Linear = 1000104003,
    DciP3Nonlinear = 1000104004,
    Bt709Linear = 1000104005,
    Bt709Nonlinear = 1000104006,
    Bt2020Linear = 1000104007,
    Hdr10St2084 = 1000104008,
    Dolbyvision = 1000104009,
    Hdr10Hlg = 1000104010,
    AdobergbLinear = 1000104011,
    AdobergbNonlinear = 1000104012,
    PassThrough = 1000104013,
    ExtendedSRgbNonlinear_ext = 1000104014,
    DisplayNative = 1000213000,
    DciP3Linear = DisplayP3Linear,
    MaxEnum = 0x7fffffff
};

// TODO: Normalize case of enum elements
enum class Format {
    UNDEFINED = 0,
    R4G4_UNORM_PACK8 = 1,
    R4G4B4A4_UNORM_PACK16 = 2,
    B4G4R4A4_UNORM_PACK16 = 3,
    R5G6B5_UNORM_PACK16 = 4,
    B5G6R5_UNORM_PACK16 = 5,
    R5G5B5A1_UNORM_PACK16 = 6,
    B5G5R5A1_UNORM_PACK16 = 7,
    A1R5G5B5_UNORM_PACK16 = 8,
    R8_UNORM = 9,
    R8_SNORM = 10,
    R8_USCALED = 11,
    R8_SSCALED = 12,
    R8_UINT = 13,
    R8_SINT = 14,
    R8_SRGB = 15,
    R8G8_UNORM = 16,
    R8G8_SNORM = 17,
    R8G8_USCALED = 18,
    R8G8_SSCALED = 19,
    R8G8_UINT = 20,
    R8G8_SINT = 21,
    R8G8_SRGB = 22,
    R8G8B8_UNORM = 23,
    R8G8B8_SNORM = 24,
    R8G8B8_USCALED = 25,
    R8G8B8_SSCALED = 26,
    R8G8B8_UINT = 27,
    R8G8B8_SINT = 28,
    R8G8B8_SRGB = 29,
    B8G8R8_UNORM = 30,
    B8G8R8_SNORM = 31,
    B8G8R8_USCALED = 32,
    B8G8R8_SSCALED = 33,
    B8G8R8_UINT = 34,
    B8G8R8_SINT = 35,
    B8G8R8_SRGB = 36,
    R8G8B8A8_UNORM = 37,
    R8G8B8A8_SNORM = 38,
    R8G8B8A8_USCALED = 39,
    R8G8B8A8_SSCALED = 40,
    R8G8B8A8_UINT = 41,
    R8G8B8A8_SINT = 42,
    R8G8B8A8_SRGB = 43,
    B8G8R8A8_UNORM = 44,
    B8G8R8A8_SNORM = 45,
    B8G8R8A8_USCALED = 46,
    B8G8R8A8_SSCALED = 47,
    B8G8R8A8_UINT = 48,
    B8G8R8A8_SINT = 49,
    B8G8R8A8_SRGB = 50,
    A8B8G8R8_UNORM_PACK32 = 51,
    A8B8G8R8_SNORM_PACK32 = 52,
    A8B8G8R8_USCALED_PACK32 = 53,
    A8B8G8R8_SSCALED_PACK32 = 54,
    A8B8G8R8_UINT_PACK32 = 55,
    A8B8G8R8_SINT_PACK32 = 56,
    A8B8G8R8_SRGB_PACK32 = 57,
    A2R10G10B10_UNORM_PACK32 = 58,
    A2R10G10B10_SNORM_PACK32 = 59,
    A2R10G10B10_USCALED_PACK32 = 60,
    A2R10G10B10_SSCALED_PACK32 = 61,
    A2R10G10B10_UINT_PACK32 = 62,
    A2R10G10B10_SINT_PACK32 = 63,
    A2B10G10R10_UNORM_PACK32 = 64,
    A2B10G10R10_SNORM_PACK32 = 65,
    A2B10G10R10_USCALED_PACK32 = 66,
    A2B10G10R10_SSCALED_PACK32 = 67,
    A2B10G10R10_UINT_PACK32 = 68,
    A2B10G10R10_SINT_PACK32 = 69,
    R16_UNORM = 70,
    R16_SNORM = 71,
    R16_USCALED = 72,
    R16_SSCALED = 73,
    R16_UINT = 74,
    R16_SINT = 75,
    R16_SFLOAT = 76,
    R16G16_UNORM = 77,
    R16G16_SNORM = 78,
    R16G16_USCALED = 79,
    R16G16_SSCALED = 80,
    R16G16_UINT = 81,
    R16G16_SINT = 82,
    R16G16_SFLOAT = 83,
    R16G16B16_UNORM = 84,
    R16G16B16_SNORM = 85,
    R16G16B16_USCALED = 86,
    R16G16B16_SSCALED = 87,
    R16G16B16_UINT = 88,
    R16G16B16_SINT = 89,
    R16G16B16_SFLOAT = 90,
    R16G16B16A16_UNORM = 91,
    R16G16B16A16_SNORM = 92,
    R16G16B16A16_USCALED = 93,
    R16G16B16A16_SSCALED = 94,
    R16G16B16A16_UINT = 95,
    R16G16B16A16_SINT = 96,
    R16G16B16A16_SFLOAT = 97,
    R32_UINT = 98,
    R32_SINT = 99,
    R32_SFLOAT = 100,
    R32G32_UINT = 101,
    R32G32_SINT = 102,
    R32G32_SFLOAT = 103,
    R32G32B32_UINT = 104,
    R32G32B32_SINT = 105,
    R32G32B32_SFLOAT = 106,
    R32G32B32A32_UINT = 107,
    R32G32B32A32_SINT = 108,
    R32G32B32A32_SFLOAT = 109,
    R64_UINT = 110,
    R64_SINT = 111,
    R64_SFLOAT = 112,
    R64G64_UINT = 113,
    R64G64_SINT = 114,
    R64G64_SFLOAT = 115,
    R64G64B64_UINT = 116,
    R64G64B64_SINT = 117,
    R64G64B64_SFLOAT = 118,
    R64G64B64A64_UINT = 119,
    R64G64B64A64_SINT = 120,
    R64G64B64A64_SFLOAT = 121,
    B10G11R11_UFLOAT_PACK32 = 122,
    E5B9G9R9_UFLOAT_PACK32 = 123,
    D16_UNORM = 124,
    X8_D24_UNORM_PACK32 = 125,
    D32_SFLOAT = 126,
    S8_UINT = 127,
    D16_UNORM_S8_UINT = 128,
    D24_UNORM_S8_UINT = 129,
    D32_SFLOAT_S8_UINT = 130,
    BC1_RGB_UNORM_BLOCK = 131,
    BC1_RGB_SRGB_BLOCK = 132,
    BC1_RGBA_UNORM_BLOCK = 133,
    BC1_RGBA_SRGB_BLOCK = 134,
    BC2_UNORM_BLOCK = 135,
    BC2_SRGB_BLOCK = 136,
    BC3_UNORM_BLOCK = 137,
    BC3_SRGB_BLOCK = 138,
    BC4_UNORM_BLOCK = 139,
    BC4_SNORM_BLOCK = 140,
    BC5_UNORM_BLOCK = 141,
    BC5_SNORM_BLOCK = 142,
    BC6H_UFLOAT_BLOCK = 143,
    BC6H_SFLOAT_BLOCK = 144,
    BC7_UNORM_BLOCK = 145,
    BC7_SRGB_BLOCK = 146,
    ETC2_R8G8B8_UNORM_BLOCK = 147,
    ETC2_R8G8B8_SRGB_BLOCK = 148,
    ETC2_R8G8B8A1_UNORM_BLOCK = 149,
    ETC2_R8G8B8A1_SRGB_BLOCK = 150,
    ETC2_R8G8B8A8_UNORM_BLOCK = 151,
    ETC2_R8G8B8A8_SRGB_BLOCK = 152,
    EAC_R11_UNORM_BLOCK = 153,
    EAC_R11_SNORM_BLOCK = 154,
    EAC_R11G11_UNORM_BLOCK = 155,
    EAC_R11G11_SNORM_BLOCK = 156,
    ASTC_4x4_UNORM_BLOCK = 157,
    ASTC_4x4_SRGB_BLOCK = 158,
    ASTC_5x4_UNORM_BLOCK = 159,
    ASTC_5x4_SRGB_BLOCK = 160,
    ASTC_5x5_UNORM_BLOCK = 161,
    ASTC_5x5_SRGB_BLOCK = 162,
    ASTC_6x5_UNORM_BLOCK = 163,
    ASTC_6x5_SRGB_BLOCK = 164,
    ASTC_6x6_UNORM_BLOCK = 165,
    ASTC_6x6_SRGB_BLOCK = 166,
    ASTC_8x5_UNORM_BLOCK = 167,
    ASTC_8x5_SRGB_BLOCK = 168,
    ASTC_8x6_UNORM_BLOCK = 169,
    ASTC_8x6_SRGB_BLOCK = 170,
    ASTC_8x8_UNORM_BLOCK = 171,
    ASTC_8x8_SRGB_BLOCK = 172,
    ASTC_10x5_UNORM_BLOCK = 173,
    ASTC_10x5_SRGB_BLOCK = 174,
    ASTC_10x6_UNORM_BLOCK = 175,
    ASTC_10x6_SRGB_BLOCK = 176,
    ASTC_10x8_UNORM_BLOCK = 177,
    ASTC_10x8_SRGB_BLOCK = 178,
    ASTC_10x10_UNORM_BLOCK = 179,
    ASTC_10x10_SRGB_BLOCK = 180,
    ASTC_12x10_UNORM_BLOCK = 181,
    ASTC_12x10_SRGB_BLOCK = 182,
    ASTC_12x12_UNORM_BLOCK = 183,
    ASTC_12x12_SRGB_BLOCK = 184,
    G8B8G8R8_422_UNORM = 1000156000,
    B8G8R8G8_422_UNORM = 1000156001,
    G8_B8_R8_3PLANE_420_UNORM = 1000156002,
    G8_B8R8_2PLANE_420_UNORM = 1000156003,
    G8_B8_R8_3PLANE_422_UNORM = 1000156004,
    G8_B8R8_2PLANE_422_UNORM = 1000156005,
    G8_B8_R8_3PLANE_444_UNORM = 1000156006,
    R10X6_UNORM_PACK16 = 1000156007,
    R10X6G10X6_UNORM_2PACK16 = 1000156008,
    R10X6G10X6B10X6A10X6_UNORM_4PACK16 = 1000156009,
    G10X6B10X6G10X6R10X6_422_UNORM_4PACK16 = 1000156010,
    B10X6G10X6R10X6G10X6_422_UNORM_4PACK16 = 1000156011,
    G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16 = 1000156012,
    G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16 = 1000156013,
    G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16 = 1000156014,
    G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16 = 1000156015,
    G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16 = 1000156016,
    R12X4_UNORM_PACK16 = 1000156017,
    R12X4G12X4_UNORM_2PACK16 = 1000156018,
    R12X4G12X4B12X4A12X4_UNORM_4PACK16 = 1000156019,
    G12X4B12X4G12X4R12X4_422_UNORM_4PACK16 = 1000156020,
    B12X4G12X4R12X4G12X4_422_UNORM_4PACK16 = 1000156021,
    G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16 = 1000156022,
    G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16 = 1000156023,
    G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16 = 1000156024,
    G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16 = 1000156025,
    G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16 = 1000156026,
    G16B16G16R16_422_UNORM = 1000156027,
    B16G16R16G16_422_UNORM = 1000156028,
    G16_B16_R16_3PLANE_420_UNORM = 1000156029,
    G16_B16R16_2PLANE_420_UNORM = 1000156030,
    G16_B16_R16_3PLANE_422_UNORM = 1000156031,
    G16_B16R16_2PLANE_422_UNORM = 1000156032,
    G16_B16_R16_3PLANE_444_UNORM = 1000156033,
    PVRTC1_2BPP_UNORM_BLOCK_IMG = 1000054000,
    PVRTC1_4BPP_UNORM_BLOCK_IMG = 1000054001,
    PVRTC2_2BPP_UNORM_BLOCK_IMG = 1000054002,
    PVRTC2_4BPP_UNORM_BLOCK_IMG = 1000054003,
    PVRTC1_2BPP_SRGB_BLOCK_IMG = 1000054004,
    PVRTC1_4BPP_SRGB_BLOCK_IMG = 1000054005,
    PVRTC2_2BPP_SRGB_BLOCK_IMG = 1000054006,
    PVRTC2_4BPP_SRGB_BLOCK_IMG = 1000054007,
    ASTC_4x4_SFLOAT_BLOCK_EXT = 1000066000,
    ASTC_5x4_SFLOAT_BLOCK_EXT = 1000066001,
    ASTC_5x5_SFLOAT_BLOCK_EXT = 1000066002,
    ASTC_6x5_SFLOAT_BLOCK_EXT = 1000066003,
    ASTC_6x6_SFLOAT_BLOCK_EXT = 1000066004,
    ASTC_8x5_SFLOAT_BLOCK_EXT = 1000066005,
    ASTC_8x6_SFLOAT_BLOCK_EXT = 1000066006,
    ASTC_8x8_SFLOAT_BLOCK_EXT = 1000066007,
    ASTC_10x5_SFLOAT_BLOCK_EXT = 1000066008,
    ASTC_10x6_SFLOAT_BLOCK_EXT = 1000066009,
    ASTC_10x8_SFLOAT_BLOCK_EXT = 1000066010,
    ASTC_10x10_SFLOAT_BLOCK_EXT = 1000066011,
    ASTC_12x10_SFLOAT_BLOCK_EXT = 1000066012,
    ASTC_12x12_SFLOAT_BLOCK_EXT = 1000066013,
    G8_B8R8_2PLANE_444_UNORM_EXT = 1000330000,
    G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16_EXT = 1000330001,
    G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16_EXT = 1000330002,
    G16_B16R16_2PLANE_444_UNORM_EXT = 1000330003,
    A4R4G4B4_UNORM_PACK16_EXT = 1000340000,
    A4B4G4R4_UNORM_PACK16_EXT = 1000340001,
    MAX_ENUM = 0x7FFFFFFF
};

enum class SurfaceTransformFlagBits {
    IdentityBit = 0x00000001,
    Rotate90Bit = 0x00000002,
    Rotate180Bit = 0x00000004,
    Rotate270Bit = 0x00000008,
    HorizontalMirrorBit = 0x00000010,
    HorizontalMirrorRotate90Bit = 0x00000020,
    HorizontalMirrorRotate180Bit = 0x00000040,
    HorizontalMirrorRotate270Bit = 0x00000080,
    InheritBit = 0x00000100,
    MaxEnum = 0x7fffffff
};
using SurfaceTransformFlags = KDGpu::Flags<SurfaceTransformFlagBits>;

inline std::string surfaceTransformFlagBitsToString(SurfaceTransformFlagBits transform)
{
    switch (transform) {
    case SurfaceTransformFlagBits::IdentityBit:
        return "Identity";
    case SurfaceTransformFlagBits::Rotate90Bit:
        return "Rotate 90";
    case SurfaceTransformFlagBits::Rotate180Bit:
        return "Rotate 180";
    case SurfaceTransformFlagBits::Rotate270Bit:
        return "Rotate 270";
    case SurfaceTransformFlagBits::HorizontalMirrorBit:
        return "Horizontal Mirror";
    case SurfaceTransformFlagBits::HorizontalMirrorRotate90Bit:
        return "Horizontal Mirror Rotate 90";
    case SurfaceTransformFlagBits::HorizontalMirrorRotate180Bit:
        return "Horizontal Mirror Rotate 180";
    case SurfaceTransformFlagBits::HorizontalMirrorRotate270Bit:
        return "Horizontal Mirror Rotate 270";
    case SurfaceTransformFlagBits::InheritBit:
        return "Inherit";
    default:
        return "Unknown";
    }
}

enum class CompositeAlphaFlagBits {
    OpaqueBit = 0x00000001,
    PreMultipliedBit = 0x00000002,
    PostMultipliedBit = 0x00000004,
    InheritBit = 0x00000008,
    MaxEnum = 0x7FFFFFFF
};
using CompositeAlphaFlags = KDGpu::Flags<CompositeAlphaFlagBits>;

enum class TextureUsageFlagBits {
    TransferSrcBit = 0x00000001,
    TransferDstBit = 0x00000002,
    SampledBit = 0x00000004,
    StorageBit = 0x00000008,
    ColorAttachmentBit = 0x00000010,
    DepthStencilAttachmentBit = 0x00000020,
    TransientAttachmentBit = 0x00000040,
    InputAttachmentBit = 0x00000080,
    VideoDecodeDstBit = 0x00000400,
    VideoDecodeSrcBit = 0x00000800,
    VideoDecodeDpbBit = 0x00001000,
    FragmentDensityMapBit = 0x00000200,
    FragmentShadingRateAttachmentBit = 0x00000100,
    VideoEncodeDstBit = 0x00002000,
    VideoEncodeSrcBit = 0x00004000,
    VideoEncodeDpbBit = 0x00008000,
    InvocationMaskBit = 0x00040000,
    HostTransferBit = 0x00400000,
    MaxEnum = 0x7fffffff
};
using TextureUsageFlags = KDGpu::Flags<TextureUsageFlagBits>;

enum class TextureCreateFlagBits {
    SparseBindingBit = 0x00000001,
    SparseResidencyBit = 0x00000002,
    SparseAliasedBit = 0x00000004,
    MutableFormatBit = 0x00000008,
    CubeCompatibleBit = 0x00000010,
    AliasBit = 0x00000400,
    SplitInstanceBindRegionBit = 0x00000040,
    Array2DCompatibleBit = 0x00000020,
    BlockTexelVieewCompatibleBit = 0x00000080,
    ExtendedUsageBit = 0x00000100,
    ProtectedBit = 0x00000800,
    DisjointBit = 0x00000200,
    CornerSampledBit = 0x00002000,
    SampleLocationsCompatibleDepthBit = 0x00001000,
    SubsampledBit = 0x00004000,
    DescriptorBufferCaptureReplayBit = 0x00010000,
    MultisampledRenderToSingleSampledBit = 0x00040000,
    View2DCompatibleBit = 0x00020000,
    VideoProfileIndepenentBit = 0x00100000,
    FragmentDensityMapOffsetBit = 0x00008000,
};
using TextureCreateFlags = KDGpu::Flags<TextureCreateFlagBits>;

enum class TextureTiling {
    Optimal = 0,
    Linear = 1,
    DrmFormatModifier = 1000158000,
    MaxEnum = 0x7fffffff
};

enum class TextureLayout {
    Undefined = 0,
    General = 1,
    ColorAttachmentOptimal = 2,
    DepthStencilAttachmentOptimal = 3,
    DepthStencilReadOnlyOptimal = 4,
    ShaderReadOnlyOptimal = 5,
    TransferSrcOptimal = 6,
    TransferDstOptimal = 7,
    Preinitialized = 8,
    DepthReadOnlyStencilAttachmentOptimal = 1000117000,
    DepthAttachmentStencilReadOnlyOptimal = 1000117001,
    DepthAttachmentOptimal = 1000241000,
    DepthReadOnlyOptimal = 1000241001,
    StencilAttachmentOptimal = 1000241002,
    StencilReadOnlyOptimal = 1000241003,
    PresentSrc = 1000001002,
    VideoDecodeDst = 1000024000,
    VideoDecodeSrc = 1000024001,
    VideoDecodeDpb = 1000024002,
    SharedPresent = 1000111000,
    FragmentDensityMapOptimal = 1000218000,
    FragmentShadingRateAttachmentOptimal = 1000164003,
    VideoEncodeDst = 1000299000,
    VideoEncodeSrc = 1000299001,
    VideoEncodeDpb = 1000299002,
    ReadOnlyOptimal = 1000314000,
    AttachmentOptimal = 1000314001,
    DynamicLocalRead = 1000232000,
    MaxEnum = 0x7fffffff
};

enum class SharingMode {
    Exclusive = 0,
    Concurrent = 1,
    MaxEnum = 0x7FFFFFFF
};

enum class TextureType {
    TextureType1D = 0,
    TextureType2D = 1,
    TextureType3D = 2,
    TextureTypeCube = 3,
    MaxEnum = 0x7fffffff
};

enum class ViewType {
    ViewType1D = 0,
    ViewType2D = 1,
    ViewType3D = 2,
    ViewTypeCube = 3,
    ViewType1DArray = 4,
    ViewType2DArray = 5,
    ViewTypeCubeArray = 6,
    MaxEnum = 0x7fffffff
};

enum class TextureAspectFlagBits : uint32_t {
    None = 0,
    ColorBit = 0x00000001,
    DepthBit = 0x00000002,
    StencilBit = 0x00000004,
    MetadataBit = 0x00000008,
    Plane0Bit = 0x00000010,
    Plane1Bit = 0x00000020,
    Plane2Bit = 0x00000040,
    MemoryPlane0Bit = 0x00000080,
    MemoryPlane1Bit = 0x00000100,
    MemoryPlane2Bit = 0x00000200,
    MemoryPlane3Bit = 0x00000400,
    MaxEnum = 0x7fffffff
};
using TextureAspectFlags = KDGpu::Flags<TextureAspectFlagBits>;

struct TextureSubresourceRange {
    TextureAspectFlags aspectMask{ TextureAspectFlagBits::None };
    uint32_t baseMipLevel{ 0 };
    uint32_t levelCount{ remainingMipLevels };
    uint32_t baseArrayLayer{ 0 };
    uint32_t layerCount{ remainingArrayLayers };
};

struct TextureSubresourceLayers {
    TextureAspectFlags aspectMask{ TextureAspectFlagBits::None };
    uint32_t mipLevel{ 0 };
    uint32_t baseArrayLayer{ 0 };
    uint32_t layerCount{ 1 };
};

enum class BufferUsageFlagBits {
    TransferSrcBit = 0x00000001,
    TransferDstBit = 0x00000002,
    UniformTexelBufferBit = 0x00000004,
    StorageTexelBufferBit = 0x00000008,
    UniformBufferBit = 0x00000010,
    StorageBufferBit = 0x00000020,
    IndexBufferBit = 0x00000040,
    VertexBufferBit = 0x00000080,
    IndirectBufferBit = 0x00000100,
    ShaderDeviceAddressBit = 0x00020000,
    VideoDecodeSrcBit = 0x00002000,
    VideoDecodeDstBit = 0x00004000,
    TransformFeedbackBufferBit = 0x00000800,
    TransformFeedbackCounterBufferBit = 0x00001000,
    ConditionalRenderingBit = 0x00000200,
    AccelerationStructureBuildInputReadOnlyBit = 0x00080000,
    AccelerationStructureStorageBit = 0x00100000,
    ShaderBindingTableBit = 0x00000400,
    VideoEncodeDstBit = 0x00008000,
    VideoEncodeSrcBit = 0x00010000,
    SamplerDescriptorBufferBit = 0x00200000,
    ResourceDescriptorBufferBit = 0x00400000,
    PushDescriptorsDescriptorBufferBit = 0x04000000,
    MicromapBuildInputReadOnlyBit = 0x00800000,
    MicromapStorageBit = 0x01000000,
    MaxEnum = 0x7fffffff
};
using BufferUsageFlags = KDGpu::Flags<BufferUsageFlagBits>;

enum class VertexRate {
    Vertex = 0,
    Instance = 1,
    MaxEnum = 0x7fffffff
};

enum class IndexType {
    Uint16 = 0,
    Uint32 = 1,
    None = 1000165000,
    Uint8 = 1000265000,
    MaxEnum = 0x7fffffff
};

enum class MemoryUsage {
    Unknown = 0,
    GpuOnly = 1,
    CpuOnly = 2,
    CpuToGpu = 3,
    GpuToCpu = 4,
    CpuCopy = 5,
    GpuLazilyAllocated = 6,
    MaxEnum = 0x7fffffff
};

enum class ShaderStageFlagBits : uint32_t {
    VertexBit = 0x00000001,
    TessellationControlBit = 0x00000002,
    TessellationEvaluationBit = 0x00000004,
    GeometryBit = 0x00000008,
    FragmentBit = 0x00000010,
    ComputeBit = 0x00000020,
    AllGraphics = 0x0000001f,
    All = 0x7fffffff,
    RaygenBit = 0x00000100,
    AnyHitBit = 0x00000200,
    ClosestHitBit = 0x00000400,
    MissBit = 0x00000800,
    IntersectionBit = 0x00001000,
    CallableBit = 0x00002000,
    TaskBit = 0x00000040,
    MeshBit = 0x00000080,
    MaxEnum = 0x7fffffff
};
using ShaderStageFlags = KDGpu::Flags<ShaderStageFlagBits>;

enum class ResourceBindingType {
    Sampler = 0,
    CombinedImageSampler = 1,
    SampledImage = 2,
    StorageImage = 3,
    UniformTexelBuffer = 4,
    StorageTexelBuffer = 5,
    UniformBuffer = 6,
    StorageBuffer = 7,
    DynamicUniformBuffer = 8,
    DynamicStorageBuffer = 9,
    InputAttachment = 10,
    AccelerationStructure = 1000150000,
    MaxEnum = 0x7fffffff
};

enum class ResourceBindingFlagBits : uint32_t {
    None = 0,
    UpdateAfterBindBit = 0x00000001,
    UpdateUnusedWhilePendingBit = 0x00000002,
    PartiallyBoundBit = 0x00000004,
    VariableBindGroupEntriesCountBit = 0x00000008,
};
using ResourceBindingFlags = KDGpu::Flags<ResourceBindingFlagBits>;

enum class PrimitiveTopology {
    PointList = 0,
    LineList = 1,
    LineStrip = 2,
    TriangleList = 3,
    TriangleStrip = 4,
    TriangleFan = 5,
    LineListWithAdjacency = 6,
    LineStripWithAdjacency = 7,
    TriangleListWithAdjacency = 8,
    TriangleStripWithAdjacency = 9,
    PatchList = 10,
    MaxEnum = 0x7fffffff
};

enum class CullModeFlagBits {
    None = 0,
    FrontBit = 0x00000001,
    BackBit = 0x00000002,
    FrontAndBack = 0x00000003,
    MaxEnum = 0x7fffffff
};
using CullModeFlags = KDGpu::Flags<CullModeFlagBits>;

enum class FrontFace {
    CounterClockwise = 0,
    Clockwise = 1,
    MaxEnum = 0x7fffffff
};

enum class PolygonMode {
    Fill = 0,
    Line = 1,
    Point = 2,
    MaxEnum = 0x7fffffff
};

enum class CompareOperation {
    Never = 0,
    Less = 1,
    Equal = 2,
    LessOrEqual = 3,
    Greater = 4,
    NotEqual = 5,
    GreaterOrEqual = 6,
    Always = 7,
    MaxEnum = 0x7fffffff
};

enum class StencilOperation {
    Keep = 0,
    Zero = 1,
    Replace = 2,
    IncrementAndClamp = 3,
    DecrementAndClamp = 4,
    Invert = 5,
    IncrementAndWrap = 6,
    DecrementAndWrap = 7,
    MaxEnum = 0x7fffffff
};

enum ColorComponentFlagBits {
    RedBit = 0x00000001,
    GreenBit = 0x00000002,
    BlueBit = 0x00000004,
    AlphaBit = 0x00000008,
    AllComponents = RedBit | GreenBit | BlueBit | AlphaBit,
    MaxEnum = 0x7fffffff
};
using ColorComponentFlags = KDGpu::Flags<ColorComponentFlagBits>;

enum class BlendOperation {
    Add = 0,
    Subtract = 1,
    ReverseSubtract = 2,
    Min = 3,
    Max = 4,
    Zero = 1000148000,
    Src = 1000148001,
    Dst = 1000148002,
    SrcOver = 1000148003,
    DstOver = 1000148004,
    SrcIn = 1000148005,
    DstIn = 1000148006,
    SrcOut = 1000148007,
    DstOut = 1000148008,
    SrcAtop = 1000148009,
    DstAtop = 1000148010,
    Xor = 1000148011,
    Multiply = 1000148012,
    Screen = 1000148013,
    Overlay = 1000148014,
    Darken = 1000148015,
    Lighten = 1000148016,
    ColorDodge = 1000148017,
    ColorBurn = 1000148018,
    HardLight = 1000148019,
    SoftLight = 1000148020,
    Difference = 1000148021,
    Exclusion = 1000148022,
    Invert = 1000148023,
    InvertRgb = 1000148024,
    LinearDodge = 1000148025,
    LinearBurn = 1000148026,
    VividLight = 1000148027,
    LinearLight = 1000148028,
    PinLight = 1000148029,
    HardMix = 1000148030,
    HslHue = 1000148031,
    HslSaturation = 1000148032,
    HslColor = 1000148033,
    HslLuminosity = 1000148034,
    Plus = 1000148035,
    PlusClamped = 1000148036,
    PlusClampedAlpha = 1000148037,
    PlusDarker = 1000148038,
    Minus = 1000148039,
    MinusClamped = 1000148040,
    Contrast = 1000148041,
    InvertOvg = 1000148042,
    Red = 1000148043,
    Green = 1000148044,
    Blue = 1000148045,
    MaxEnum = 0x7fffffff
};

enum class BlendFactor {
    Zero = 0,
    One = 1,
    SrcColor = 2,
    OneMinusSrcColor = 3,
    DstColor = 4,
    OneMinusDstColor = 5,
    SrcAlpha = 6,
    OneMinusSrcAlpha = 7,
    DstAlpha = 8,
    OneMinusDstAlpha = 9,
    ConstantColor = 10,
    OneMinusConstantColor = 11,
    ConstantAlpha = 12,
    OneMinusConstantAlpha = 13,
    SrcAlphaSaturate = 14,
    Src1Color = 15,
    OneMinusSrc1Color = 16,
    Src1Alpha = 17,
    OneMinusSrc1Alpha = 18,
    MaxEnum = 0x7fffffff
};

enum class AttachmentLoadOperation {
    Load = 0,
    Clear = 1,
    DontCare = 2,
    MaxEnum = 0x7fffffff
};

enum class AttachmentStoreOperation {
    Store = 0,
    DontCare = 1,
    MaxEnum = 0x7fffffff
};

enum class AddressMode {
    Repeat = 0,
    MirroredRepeat = 1,
    ClampToEdge = 2,
    ClampToBorder = 3,
    MirrorClampToEdge = 4,
    MaxEnum = 0x7fffffff
};

enum class FilterMode {
    Nearest = 0,
    Linear = 1,
    MaxEnum = 0x7fffffff
};

enum class MipmapFilterMode {
    Nearest = 0,
    Linear = 1,
    MaxEnum = 0x7fffffff
};

namespace MipmapLodClamping {
constexpr float NoClamping = float(0x7fffffff);
}

enum class AccessFlagBit : uint64_t {
    None = 0,
    IndirectCommandReadBit = 0x00000001,
    IndexReadBit = 0x00000002,
    VertexAttributeReadBit = 0x00000004,
    UniformReadBit = 0x00000008,
    InputAttachmentReadBit = 0x00000010,
    ShaderReadBit = 0x00000020,
    ShaderWriteBit = 0x00000040,
    ColorAttachmentReadBit = 0x00000080,
    ColorAttachmentWriteBit = 0x00000100,
    DepthStencilAttachmentReadBit = 0x00000200,
    DepthStencilAttachmentWriteBit = 0x00000400,
    TransferReadBit = 0x00000800,
    TransferWriteBit = 0x00001000,
    HostReadBit = 0x00002000,
    HostWriteBit = 0x00004000,
    MemoryReadBit = 0x00008000,
    MemoryWriteBit = 0x00010000,
    ShaderSampledReadBit = 0x100000000ULL,
    ShaderStorageReadBit = 0x200000000ULL,
    ShaderStorageWriteBit = 0x400000000ULL,
    VideoDecodeReadBit = 0x800000000ULL,
    VideoDecodeWriteBit = 0x1000000000ULL,
    VideoEncodeReadBit = 0x2000000000ULL,
    VideoEncodeWriteBit = 0x4000000000ULL,
    TransformFeedbackWriteBit = 0x02000000ULL,
    TransformFeedbackCounterReadBit = 0x04000000ULL,
    TransformFeedbackCounterWriteBit = 0x08000000ULL,
    ConditionalRenderingReadBit = 0x00100000ULL,
    CommandPreprocessReadBit = 0x00020000ULL,
    CommandPreprocessWriteBit = 0x00040000ULL,
    FragmentShadingRateAttachmentReadBit = 0x00800000ULL,
    ShadingRateImageReadBit = 0x00800000ULL,
    AccelerationStructureReadBit = 0x00200000ULL,
    AccelerationStructureWriteBit = 0x00400000ULL,
    FragmentDensityMapReadBit = 0x01000000ULL,
    ColorAttachmentReadNoncoherentBit = 0x00080000ULL,
    DescriptorBufferReadBit = 0x20000000000ULL,
    ShaderBindingTableReadBit = 0x10000000000ULL,
    MicromapReadBit = 0x100000000000ULL,
    MicromapWriteBit = 0x200000000000ULL,
    OpticalFlowReadBit = 0x40000000000ULL,
    OpticalFlowWriteBit = 0x80000000000ULL
};
using AccessFlags = KDGpu::Flags<AccessFlagBit>;

enum class PipelineStageFlagBit : uint64_t {
    None = 0,
    TopOfPipeBit = 0x00000001ULL,
    DrawIndirectBit = 0x00000002ULL,
    VertexInputBit = 0x00000004ULL,
    VertexShaderBit = 0x00000008ULL,
    TessellationControlShaderBit = 0x00000010ULL,
    TessellationEvaluationShaderBit = 0x00000020ULL,
    GeometryShaderBit = 0x00000040ULL,
    FragmentShaderBit = 0x00000080ULL,
    EarlyFragmentTestBit = 0x00000100ULL,
    LateFragmentTestBit = 0x00000200ULL,
    ColorAttachmentOutputBit = 0x00000400ULL,
    ComputeShaderBit = 0x00000800ULL,
    TransferBit = 0x00001000ULL,
    BottomOfPipeBit = 0x00002000ULL,
    HostBit = 0x00004000ULL,
    AllGraphicsBit = 0x00008000ULL,
    AllCommandsBit = 0x00010000ULL,
    CopyBit = 0x100000000ULL,
    ResolveBit = 0x200000000ULL,
    BlitBit = 0x400000000ULL,
    ClearBit = 0x800000000ULL,
    IndexInputBit = 0x1000000000ULL,
    VertexAttributeInputBit = 0x2000000000ULL,
    PreRasterizationShadersBit = 0x4000000000ULL,
    VideoDecodeBit = 0x04000000ULL,
    VideoEncodeBit = 0x08000000ULL,
    TransformFeedbackBit = 0x01000000ULL,
    ConditionalRenderingBit = 0x00040000ULL,
    CommandPreprocessBit = 0x00020000ULL,
    FragmentShadingRateAttachmentBit = 0x00400000ULL,
    ShadingRateImageBit = 0x00400000ULL,
    AccelerationStructureBuildBit = 0x02000000ULL,
    RayTracingShaderBit = 0x00200000ULL,
    FragmentDensityProcessBit = 0x00800000ULL,
    TaskShaderBit = 0x00080000ULL,
    MeshShaderBit = 0x00100000ULL,
    AccelerationStructureCopyBit = 0x10000000ULL,
    MicromapBuildBit = 0x40000000ULL,
    OpticalFlowBit = 0x20000000ULL
};
using PipelineStageFlags = KDGpu::Flags<PipelineStageFlagBit>;

enum class CommandBufferLevel {
    Primary = 0,
    Secondary = 1,
    MaxEnum = 0x7FFFFFFF
};

enum class FormatFeatureFlagBit : uint32_t {
    SampledImageBit = 0x00000001,
    StorageImageBit = 0x00000002,
    StorageAtomicBit = 0x00000004,
    UniformTexelBufferBit = 0x00000008,
    StorageTexelBufferBit = 0x00000010,
    StorageTexelBufferAtomicBit = 0x00000020,
    VertexBufferBit = 0x00000040,
    ColorAttachmentBit = 0x00000080,
    ColorAttachmentBlendBit = 0x00000100,
    DepthStencilAttachmentBit = 0x00000200,
    BlitSrcBit = 0x00000400,
    BlitDstBit = 0x00000800,
    SampledImageFilterLinearBit = 0x00001000,
    TransferSrcBit = 0x00004000,
    TransferDstBit = 0x00008000,
    MidpointChromaSampleBit = 0x00020000,
    SampledImageYCBCRConversionLinearFilterBit = 0x00040000,
    SampledImageYCBCRConversionSeparateReconstructionFilterBit = 0x00080000,
    SampledImageYCBCRConversionChromaReconstructionExplicitBit = 0x00100000,
    SampledImageYCBCRConversionChromaReconstructionExplicitForceableBit = 0x00200000,
    DisjointBit = 0x00400000,
    CositedChromaSampledBit = 0x00800000,
    SampledImageFilterMinMaxBit = 0x00010000,
    SampledImageFilterCubicBit = 0x00002000,
    MaxEnum = 0x7FFFFFFF
};
using FormatFeatureFlags = KDGpu::Flags<FormatFeatureFlagBit>;

enum class PresentResult {
    // Error nothing submitted
    OutOfMemory,
    DeviceLost,
    // Error but commands submitted
    OutOfDate,
    SurfaceLost,
    // Success
    Success,
};

using AcquireImageResult = PresentResult;

enum class FenceStatus {
    Signalled = 0,
    Unsignalled = 1,
    Error = 2
};

enum class ExternalSemaphoreHandleTypeFlagBits : uint32_t {
    None = 0,
    OpaqueFD = 0x00000001,
    OpaqueWin32 = 0x00000002,
    OpaqueWin32Kmt = 0x00000004,
    D3D12Fence = 0x00000008,
    SyncFD = 0x00000010,
    ZirconEventFuchsia = 0x00000080,
};
using ExternalSemaphoreHandleTypeFlags = KDGpu::Flags<ExternalSemaphoreHandleTypeFlagBits>;

enum class ExternalFenceHandleTypeFlagBits : uint32_t {
    None = 0,
    OpaqueFD = 0x00000001,
    OpaqueWin32 = 0x00000002,
    OpaqueWin32Kmt = 0x00000004,
    SyncFD = 0x00000008,
};
using ExternalFenceHandleTypeFlags = KDGpu::Flags<ExternalFenceHandleTypeFlagBits>;

enum class ExternalMemoryHandleTypeFlagBits : uint32_t {
    None = 0,
    OpaqueFD = 0x00000001,
    OpaqueWin32 = 0x00000002,
    OpaqueWin32Kmt = 0x00000004,
    D3D11Texture = 0x00000008,
    D3D11TextureKmt = 0x00000010,
    D3D12Heap = 0x00000020,
    D3D12Resource = 0x00000040,
    DmaBuf = 0x00000200,
    AndroidHardwareBuffer = 0x00000400,
    HostAllocation = 0x00000080,
    HostMappedForeignMemor = 0x00000100,
    ZirconVmoFuschia = 0x00000800,
    RmdaAddressNV = 0x00001000,
    ScreenBufferQnx = 0x00004000,
};
using ExternalMemoryHandleTypeFlags = KDGpu::Flags<ExternalMemoryHandleTypeFlagBits>;

enum ResolveModeFlagBits : uint32_t {
    None = 0,
    SampleZero = 0x00000001,
    Average = 0x00000002,
    Min = 0x00000004,
    Max = 0x00000008,
};
using ResolveModeFlags = KDGpu::Flags<ResolveModeFlagBits>;

enum class StencilFaceFlagBits {
    FrontBit = 0x00000001,
    BackBit = 0x00000002,
    FrontAndBack = 0x00000003,
    MaxEnum = 0x7fffffff
};
using StencilFaceFlags = KDGpu::Flags<StencilFaceFlagBits>;

enum class DynamicState {
    StencilReference = 8,
};

enum class BuildAccelerationStructureMode {
    Build = 0,
    Update = 1
};

enum class AccelerationStructureType {
    TopLevel = 0,
    BottomLevel = 1,
    Generic = 2
};

enum class GeometryInstanceFlagBits {
    None = 0,
    TriangleFacingCullDisable = 0x00000001,
    TriangleFlipFacing = 0x00000002,
    ForceOpaque = 0x00000004,
    ForceNoOpaque = 0x00000008
};
using GeometryInstanceFlags = KDGpu::Flags<GeometryInstanceFlagBits>;

enum class RayTracingShaderGroupType {
    General = 0,
    TrianglesHit = 1,
    ProceduralHit = 2,
};

enum class AccelerationStructureFlagBits {
    None = 0,
    AllowUpdate = 0x00000001,
    AllowCompaction = 0x00000002,
    PreferFastTrace = 0x00000004,
    PreferFastBuild = 0x00000008,
    LowMemory = 0x00000010,
};
using AccelerationStructureFlags = KDGpu::Flags<AccelerationStructureFlagBits>;

enum class DependencyFlagBits {
    ByRegion = 0x00000001,
    ByDeviceGroup = 0x00000004,
    ByLocalView = 0x00000002,
    FeedbackLoop = 0x00000008,
};

using DependencyFlags = KDGpu::Flags<DependencyFlagBits>;

enum class HostImageCopyFlagBits {
    None = 0,
    HostImageMemcpy = 0x00000001,
};
using HostImageCopyFlags = KDGpu::Flags<HostImageCopyFlagBits>;

enum class SamplerYCbCrModelConversion {
    RgbIdentity = 0,
    YCbCrIdentity = 1,
    YCbCr709 = 2,
    YCbCr601 = 3,
    YCbCr2020 = 4,
};

enum class SamplerYCbCrRange {
    ItuFull = 0,
    ItuNarrow = 1,
};

enum class ChromaLocation {
    CositedEven = 0,
    MidPoint = 1,
};

enum class ComponentSwizzle {
    Identity = 0,
    Zero = 1,
    One = 2,
    R = 3,
    G = 4,
    B = 5,
    A = 6,
};

struct ComponentMapping {
    ComponentSwizzle r;
    ComponentSwizzle g;
    ComponentSwizzle b;
    ComponentSwizzle a;
};

enum class BindGroupPoolFlagBits {
    None = 0,
    CreateFreeBindGroups = 0x00000001,
    UpdateAfterBind = 0x00000002,
    CreateHostOnly = 0x00000004,
};
using BindGroupPoolFlags = KDGpu::Flags<BindGroupPoolFlagBits>;

enum class BindGroupLayoutFlagBits {
    None = 0,
    PushBindGroup = 0x00000001, // BindGroup to be used with RenderPassCommandRecorder::pushBindGroup and not allocated from a BindGroupPool
    UpdateAfterBind = 0x00000002, // BindGroups will have to be allocated with a BindGroupPool that was created with BindGroupPoolFlagBits::UpdateAfterBind
};
using BindGroupLayoutFlags = KDGpu::Flags<BindGroupLayoutFlagBits>;

/*! @} */

} // namespace KDGpu

OPERATORS_FOR_FLAGS(KDGpu::QueueFlags)
OPERATORS_FOR_FLAGS(KDGpu::TextureUsageFlags)
OPERATORS_FOR_FLAGS(KDGpu::TextureAspectFlags)
OPERATORS_FOR_FLAGS(KDGpu::BufferUsageFlags)
OPERATORS_FOR_FLAGS(KDGpu::ShaderStageFlags)
OPERATORS_FOR_FLAGS(KDGpu::CullModeFlags)
OPERATORS_FOR_FLAGS(KDGpu::ColorComponentFlags)
OPERATORS_FOR_FLAGS(KDGpu::AccessFlags)
OPERATORS_FOR_FLAGS(KDGpu::PipelineStageFlags)
OPERATORS_FOR_FLAGS(KDGpu::FormatFeatureFlags)
OPERATORS_FOR_FLAGS(KDGpu::ExternalSemaphoreHandleTypeFlags)
OPERATORS_FOR_FLAGS(KDGpu::ExternalMemoryHandleTypeFlags)
OPERATORS_FOR_FLAGS(KDGpu::ResolveModeFlags)
OPERATORS_FOR_FLAGS(KDGpu::StencilFaceFlags)
OPERATORS_FOR_FLAGS(KDGpu::GeometryInstanceFlags)
OPERATORS_FOR_FLAGS(KDGpu::AccelerationStructureFlags)
OPERATORS_FOR_FLAGS(KDGpu::DependencyFlags);
OPERATORS_FOR_FLAGS(KDGpu::HostImageCopyFlags);
OPERATORS_FOR_FLAGS(KDGpu::TextureCreateFlags);
OPERATORS_FOR_FLAGS(KDGpu::BindGroupPoolFlags);
OPERATORS_FOR_FLAGS(KDGpu::BindGroupLayoutFlags);
