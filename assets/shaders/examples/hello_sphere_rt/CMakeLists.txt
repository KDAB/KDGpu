# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#
kdgpu_compilertshader(KDGpu_HelloSphereRtGen raygen.rgen raygen.spv)
kdgpu_compilertshader(KDGpu_HelloSphereRtMiss miss.rmiss miss.spv)
kdgpu_compilertshader(KDGpu_HelloSphereRtClosest closest.rchit closest.spv)
kdgpu_compilertshader(KDGpu_HelloSphereRtIntersection intersection.rint intersection.spv)
