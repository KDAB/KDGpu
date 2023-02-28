#pragma once

#include <toy_renderer/gpu_core.h>

namespace ToyRenderer {

struct SamplerOptions {
    FilterMode magFilter{ FilterMode::Nearest };
    FilterMode minFilter{ FilterMode::Nearest };

    MipmapFilterMode mipmapFilter{ MipmapFilterMode::Nearest };

    AddressMode u{ AddressMode::Repeat };
    AddressMode v{ AddressMode::Repeat };
    AddressMode w{ AddressMode::Repeat };

    float lodMinClamp{ 0.0f };
    float lodMaxClamp{ 32.0f };

    bool anisotropyEnabled{ false };
    float maxAnisotropy{ 1.0f };

    bool compareEnabled{ false };
    CompareOperation compare{ CompareOperation::Never };
};

} // namespace ToyRenderer
