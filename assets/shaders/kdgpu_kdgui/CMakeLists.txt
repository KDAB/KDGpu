# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#
CompileShaderSet(KDGpuKDGui_ImGui imgui)

# TODO: Can/should we fold this into the compile shader set helper as an option?
install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/imgui.vert.spv ${CMAKE_CURRENT_BINARY_DIR}/imgui.frag.spv
    DESTINATION ${CMAKE_INSTALL_DATADIR}/assets/shaders/kdgpu_kdgui
)
