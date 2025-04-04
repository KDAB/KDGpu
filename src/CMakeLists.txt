# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#
add_subdirectory(KDGpu)
add_subdirectory(KDGpuUtils)

if(KDGPU_BUILD_KDGPUKDGUI)
    add_subdirectory(KDGpuKDGui)
endif()

if(KDGPU_BUILD_KDXR)
    add_subdirectory(KDXr)
endif()

if(KDGPU_BUILD_KDGPUEXAMPLE)
    add_subdirectory(KDGpuExample)
endif()
