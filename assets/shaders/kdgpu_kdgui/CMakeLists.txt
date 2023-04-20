CompileShaderSet(KDGpuKDGui_ImGui imgui)

# TODO: Can/should we fold this into the compile shader set helper as an option?
install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/imgui.vert.spv
    ${CMAKE_CURRENT_BINARY_DIR}/imgui.frag.spv
    DESTINATION
    ${CMAKE_INSTALL_DATADIR}/assets/shaders/kdgpu_kdgui
)
