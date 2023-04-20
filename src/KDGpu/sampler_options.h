#pragma once

#include <KDGpu/gpu_core.h>

namespace KDGpu {

struct SamplerOptions {
    FilterMode magFilter{ FilterMode::Nearest };
    FilterMode minFilter{ FilterMode::Nearest };

    MipmapFilterMode mipmapFilter{ MipmapFilterMode::Nearest };

    AddressMode u{ AddressMode::Repeat };
    AddressMode v{ AddressMode::Repeat };
    AddressMode w{ AddressMode::Repeat };

    float lodMinClamp{ 0.0f };
    float lodMaxClamp{ MipmapLodClamping::NoClamping };

    bool anisotropyEnabled{ false };
    float maxAnisotropy{ 1.0f };

    bool compareEnabled{ false };
    CompareOperation compare{ CompareOperation::Never };

    bool normalizedCoordinates{ true };
};

} // namespace KDGpu
