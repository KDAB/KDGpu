# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#
kdgpu_compilertshader(KDGpu_RayTracingPipelineGen raygen.rgen raygen.spv)
kdgpu_compilertshader(KDGpu_RayTracingPipelineGenSC raygensc.rgen raygensc.spv)
kdgpu_compilertshader(KDGpu_RayTracingPipelineMiss miss.rmiss miss.spv)
kdgpu_compilertshader(KDGpu_RayTracingPipelineClosest closest.rchit closest.spv)
