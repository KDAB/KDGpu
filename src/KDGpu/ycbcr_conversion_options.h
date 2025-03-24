/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/gpu_core.h>

namespace KDGpu {

struct YCbCrConversionOptions {
    std::string_view label;
    Format format;
    SamplerYCbCrModelConversion model{ SamplerYCbCrModelConversion::RgbIdentity };
    SamplerYCbCrRange range{ SamplerYCbCrRange::ItuFull };
    ComponentMapping components{
        .r = ComponentSwizzle::Identity,
        .g = ComponentSwizzle::Identity,
        .b = ComponentSwizzle::Identity,
        .a = ComponentSwizzle::Identity,
    };
    ChromaLocation xChromaOffset{ ChromaLocation::CositedEven };
    ChromaLocation yChromaOffset{ ChromaLocation::CositedEven };
    FilterMode chromaFilter{ FilterMode::Nearest };
    bool forceExplicitReconstruction{ false };
};

} // namespace KDGpu
