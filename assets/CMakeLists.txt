# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

if(KDGPU_BUILD_EXAMPLES)
    add_subdirectory(fonts)
    add_subdirectory(textures)
endif()

add_subdirectory(shaders)
