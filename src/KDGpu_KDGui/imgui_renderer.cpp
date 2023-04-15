#include "imgui_renderer.h"

#include <KDGpu/graphics_pipeline_options.h>

#include <KDGpu/device.h>

#include <imgui.h>

#include <vector>

using namespace KDGpu;

namespace {
struct VertexImGui {
    static VertexBufferLayout vertexBufferLayout()
    {
        return VertexBufferLayout{
            .binding = 0,
            .stride = sizeof(ImDrawVert),
            .inputRate = KDGpu::VertexRate::Vertex,
        };
    }

    static std::vector<VertexAttribute> vertexAttributes()
    {
        // clang-format off
        static std::vector<VertexAttribute> attributes = {{
            .location = 0,
            .binding = 0,
            .format = KDGpu::Format::R32G32_SFLOAT,
            .offset = offsetof(ImDrawVert, pos),
        }, {
            .location = 1,
            .binding = 0,
            .format = KDGpu::Format::R32G32_SFLOAT,
            .offset = offsetof(ImDrawVert, uv),
        }, {
            .location = 2,
            .binding = 0,
            .format = KDGpu::Format::R8G8B8_UNORM,
            .offset = offsetof(ImDrawVert, col),
        }};
        // clang-format on
        return attributes;
    }
};

} // namespace

namespace KDGpuKDGui {

inline std::string assetPath()
{
#if defined(KDGPU_KDGUI_ASSET_PATH)
    return KDGPU_KDGUI_ASSET_PATH;
#else
    return "";
#endif
}

ImGuiRenderer::ImGuiRenderer(KDGpu::Device *device, ImGuiContext *imGuiContext)
    : m_device(device)
    , m_imGuiContext(imGuiContext)
{
    ImGui::SetCurrentContext(m_imGuiContext);

    // Color scheme
    ImGuiStyle &style = ImGui::GetStyle();
    style.ChildRounding = 5.0f;
    style.FrameRounding = 2.0f;
    style.PopupRounding = 5.0f;
    style.WindowRounding = 5.0f;
    style.AntiAliasedFill = true;
    style.AntiAliasedLines = true;
    style.ItemSpacing = ImVec2(8.0f, 8.0f);
    style.ItemInnerSpacing = ImVec2(6.0f, 6.0f);
    // TODO: Create a constexpr hex -> rgb/rgba helper that works with templated vector types e.g. imgui and glm.
    style.Colors[ImGuiCol_Text] = ImVec4(226.0f / 255.0f, 232.0f / 255.0f, 240.0f / 255.0f, 1.0f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(15.0f / 255.0f, 23.0f / 255.0f, 42.0f / 255.0f, 1.0f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(30.0f / 255.0f, 41.0f / 255.0f, 59.0f / 255.0f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(51.0f / 255.0f, 65.0f / 255.0f, 85.0f / 255.0f, 1.0f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(30.0f / 255.0f, 41.0f / 255.0f, 59.0f / 255.0f, 1.0f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(14.0f / 255.0f, 165.0f / 255.0f, 233.0f / 255.0f, 1.0f);
    style.Colors[ImGuiCol_Button] = ImVec4(14.0f / 255.0f, 165.0f / 255.0f, 233.0f / 255.0f, 1.0f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(14.0f / 255.0f, 165.0f / 255.0f, 233.0f / 255.0f, 1.0f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(14.0f / 255.0f, 165.0f / 255.0f, 233.0f / 255.0f, 1.0f);

    const auto vertShaderCode = KDGpu::readShaderFile(KDGpuKDGui::assetPath() + "/shaders/kdgpu_kdgui/imgui.vert.spv");
    m_vertexShader = m_device->createShaderModule(vertShaderCode);
    const auto fragShaderCode = KDGpu::readShaderFile(KDGpuKDGui::assetPath() + "/shaders/kdgpu_kdgui/imgui.frag.spv");
    m_fragmentShader = m_device->createShaderModule(fragShaderCode);
}

ImGuiRenderer::~ImGuiRenderer()
{
}

} // namespace KDGpuKDGui
