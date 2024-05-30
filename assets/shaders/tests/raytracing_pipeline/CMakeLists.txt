# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#
CompileRTShader(KDGpu_RayTracingPipelineGen raygen.rgen raygen.spv)
CompileRTShader(KDGpu_RayTracingPipelineGenSC raygensc.rgen raygensc.spv)
CompileRTShader(KDGpu_RayTracingPipelineMiss miss.rmiss miss.spv)
CompileRTShader(KDGpu_RayTracingPipelineClosest closest.rchit closest.spv)
