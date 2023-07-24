# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2022-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#
add_subdirectory(KDGpu)
add_subdirectory(KDGpuKDGui)

if(KDGPU_BUILD_EXAMPLES)
    add_subdirectory(KDGpuExample)
endif()
