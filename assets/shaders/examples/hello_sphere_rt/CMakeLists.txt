# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#
CompileRTShader(KDGpu_HelloSphereRtGen raygen.rgen raygen.spv)
CompileRTShader(KDGpu_HelloSphereRtMiss miss.rmiss miss.spv)
CompileRTShader(KDGpu_HelloSphereRtClosest closest.rchit closest.spv)
CompileRTShader(KDGpu_HelloSphereRtIntersection intersection.rint intersection.spv)
