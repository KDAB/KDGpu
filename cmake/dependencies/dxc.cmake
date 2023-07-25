# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2022-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#
message("-- Locating the DXC compiler...")

find_program(DXC_EXECUTABLE dxc HINTS "$ENV{VULKAN_SDK}/bin")
if(NOT DXC_EXECUTABLE)
    message(
        FATAL_ERROR
            "\ndxc (DirectXShaderCompiler) could not be found. "
            "This tool is required to compile hlsl shaders. Consider adding the VulkanSDK/bin folder to your PATH.\n"
    )
else()
    message(NOTICE "Found dxc at ${DXC_EXECUTABLE}")
endif()
