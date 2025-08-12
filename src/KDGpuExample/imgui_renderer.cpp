/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "imgui_renderer.h"

#include <KDGpuExample/kdgpuexample.h>

#include <KDGpu/bind_group_options.h>
#include <KDGpu/bind_group_layout_options.h>
#include <KDGpu/buffer_options.h>
#include <KDGpu/device.h>
#include <KDGpu/render_pass.h>
#include <KDGpu/texture_options.h>
#include <KDUtils/color.h>

#include <cmrc/cmrc.hpp>

#include <imgui.h>

#include <vector>

CMRC_DECLARE(KDGpuExample::ShaderResources);
CMRC_DECLARE(KDGpuExample::Resources);

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
            .format = KDGpu::Format::R8G8B8A8_UNORM,
            .offset = offsetof(ImDrawVert, col),
        }};
        // clang-format on
        return attributes;
    }
};

std::vector<uint32_t> readShaderFileFromCmrc(cmrc::embedded_filesystem &fs, const std::string &filename)
{
    auto file = fs.open(filename);
    const std::size_t byteSize = file.size();
    std::vector<uint32_t> buffer(byteSize / 4);
    std::memcpy(buffer.data(), file.cbegin(), byteSize);
    return buffer;
}

} // namespace

namespace KDGpuExample {

ImGuiRenderer::ImGuiRenderer(KDGpu::Device *device, KDGpu::Queue *queue, ImGuiContext *imGuiContext)
    : m_device(device)
    , m_queue(queue)
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

    style.Colors[ImGuiCol_Text] = KDUtils::hexToRgba<ImVec4>("#e2e8f0", 1.0f);
    style.Colors[ImGuiCol_WindowBg] = KDUtils::hexToRgba<ImVec4>("#2a2726", 0.85f);
    style.Colors[ImGuiCol_TitleBg] = KDUtils::hexToRgba<ImVec4>("#1e293b", 1.0f);
    style.Colors[ImGuiCol_TitleBgActive] = KDUtils::hexToRgba<ImVec4>("#334155", 1.0f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = KDUtils::hexToRgba<ImVec4>("#1e293b", 1.0f);
}

ImGuiRenderer::~ImGuiRenderer()
{
}

void ImGuiRenderer::initialize(float scaleFactor, KDGpu::SampleCountFlagBits samples, KDGpu::Format colorFormat, KDGpu::Format depthFormat)
{
    {
        auto fs = cmrc::KDGpuExample::ShaderResources::get_filesystem();
        const auto vertShaderCode = readShaderFileFromCmrc(fs, "imgui.vert.spv");
        m_vertexShader = m_device->createShaderModule(vertShaderCode);
        const auto fragShaderCode = readShaderFileFromCmrc(fs, "imgui.frag.spv");
        m_fragmentShader = m_device->createShaderModule(fragShaderCode);
    }

    m_bindGroupLayout = m_device->createBindGroupLayout(
            BindGroupLayoutOptions{
                    .bindings = {
                            { .binding = 0,
                              .count = 1,
                              .resourceType = ResourceBindingType::CombinedImageSampler,
                              .shaderStages = ShaderStageFlagBits::FragmentBit },
                    },
            });

    m_pipelineLayout = m_device->createPipelineLayout(
            PipelineLayoutOptions{
                    .bindGroupLayouts = { m_bindGroupLayout },
                    .pushConstantRanges = {
                            PushConstantRange{
                                    .offset = 0,
                                    .size = sizeof(PushConstantBlock),
                                    .shaderStages = ShaderStageFlagBits::VertexBit,
                            },
                    },
            });

    const auto samplerOptions = SamplerOptions{
        .magFilter = FilterMode::Linear,
        .minFilter = FilterMode::Linear
    };
    m_sampler = m_device->createSampler(samplerOptions);

    m_pipelineInfo = GraphicsPipelineOptions{
        .shaderStages = {
                { .shaderModule = m_vertexShader, .stage = ShaderStageFlagBits::VertexBit },
                { .shaderModule = m_fragmentShader, .stage = ShaderStageFlagBits::FragmentBit },
        },
        .layout = m_pipelineLayout,
        .vertex = {
                .buffers = { VertexImGui::vertexBufferLayout() },
                .attributes = VertexImGui::vertexAttributes(),
        },
        .renderTargets = {
                {
                        .format = colorFormat,
                        .blending = {
                                .blendingEnabled = true,
                                .color = {
                                        .srcFactor = BlendFactor::SrcAlpha,
                                        .dstFactor = BlendFactor::OneMinusSrcAlpha,
                                },
                                .alpha = {
                                        .srcFactor = BlendFactor::One,
                                        .dstFactor = BlendFactor::OneMinusSrcAlpha,
                                },
                        },
                },
        },
        .depthStencil = {
                .format = depthFormat,
                .depthTestEnabled = false,
                .depthWritesEnabled = false,
        },
        .primitive = {
                .cullMode = CullModeFlagBits::None,
        },
        .multisample = {
                .samples = static_cast<KDGpu::SampleCountFlagBits>(samples),
        },
    };

    updateScale(scaleFactor);
}

void ImGuiRenderer::updateScale(const float scaleFactor)
{
    ImGuiStyle &style = ImGui::GetStyle();
    style.ScaleAllSizes(scaleFactor / m_oldScaleFactor);

    initializeFontData(scaleFactor);

    m_oldScaleFactor = scaleFactor;
}

void ImGuiRenderer::cleanup()
{
    m_meshes.clear();
    m_pipeline = {};
    m_pipelineLayout = {};
    m_bindGroupLayout = {};
    m_bindGroup = {};
    m_sampler = {};
    m_textureView = {};
    m_texture = {};
    m_vertexShader = {};
    m_fragmentShader = {};
}

bool ImGuiRenderer::updateGeometryBuffers(uint32_t inFlightIndex)
{
    ImDrawData *imDrawData = ImGui::GetDrawData();

    if (!imDrawData)
        return false;

    // Note: Alignment is done inside buffer creation
    const size_t vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
    const size_t indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);

    // Update buffers only if vertex or index count has been changed compared to current buffer size
    if ((vertexBufferSize == 0) || (indexBufferSize == 0))
        return false;

    if (m_meshes.size() <= inFlightIndex) {
        m_meshes.resize(inFlightIndex + 1);
    }
    m_mesh = &m_meshes[inFlightIndex];

    // Vertex buffer
    if ((!m_mesh->vertices.isValid()) || (m_mesh->vertexCount != imDrawData->TotalVtxCount)) {
        m_mesh->vertices = m_device->createBuffer(BufferOptions{
                .size = vertexBufferSize,
                .usage = BufferUsageFlagBits::VertexBufferBit,
                .memoryUsage = MemoryUsage::CpuToGpu,
        });
        m_mesh->vertexCount = imDrawData->TotalVtxCount;
    }

    // Index buffer
    if ((!m_mesh->indexBuffer.isValid()) ||
        (m_mesh->indexCount < static_cast<uint32_t>(imDrawData->TotalIdxCount))) {
        m_mesh->indexBuffer = m_device->createBuffer(BufferOptions{
                .size = indexBufferSize,
                .usage = BufferUsageFlagBits::IndexBufferBit,
                .memoryUsage = MemoryUsage::CpuToGpu,
        });
        m_mesh->indexCount = imDrawData->TotalIdxCount;
    }

    // Upload data
    ImDrawVert *vtxDst = static_cast<ImDrawVert *>(m_mesh->vertices.map());
    ImDrawIdx *idxDst = static_cast<ImDrawIdx *>(m_mesh->indexBuffer.map());

    for (int n = 0; n < imDrawData->CmdListsCount; n++) {
        const ImDrawList *cmd_list = imDrawData->CmdLists[n];
        memcpy(vtxDst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
        memcpy(idxDst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
        vtxDst += cmd_list->VtxBuffer.Size;
        idxDst += cmd_list->IdxBuffer.Size;
    }

    // Flush
    m_mesh->vertices.unmap();
    m_mesh->indexBuffer.unmap();

    return m_mesh->vertexCount != 0;
}

void ImGuiRenderer::recordCommands(KDGpu::RenderPassCommandRecorder *recorder,
                                   KDGpu::Extent2D extent,
                                   uint32_t inFlightIndex,
                                   KDGpu::RenderPass *currentRenderPass,
                                   int lastSubpassIndex,
                                   bool dynamicRendering)
{
    ImDrawData *imDrawData = ImGui::GetDrawData();

    assert((dynamicRendering && !currentRenderPass) ||
           (currentRenderPass && !dynamicRendering) ||
           (!currentRenderPass && !dynamicRendering)); // We can't have both dynamic rendering and a render pass

    if ((!imDrawData) || (imDrawData->CmdListsCount == 0))
        return;

    ImGuiIO &io = ImGui::GetIO();
    int32_t vertexOffset = 0;
    uint32_t indexOffset = 0;

    // Bind the pipeline
    if (currentRenderPass) {
        m_pipelineInfo.renderPass = currentRenderPass->handle();
        m_pipelineInfo.subpassIndex = lastSubpassIndex;
    }
    m_pipelineInfo.dynamicRendering.enabled = dynamicRendering;

    if (!m_pipeline.isValid())
        m_pipeline = m_device->createGraphicsPipeline(m_pipelineInfo);
    recorder->setPipeline(m_pipeline);

    // Bind the descriptor set
    recorder->setBindGroup(0, m_bindGroup);

    // Set the push constants
    const float displaySize[2] = { imDrawData->DisplaySize.x, imDrawData->DisplaySize.y };
    const float displayPos[2] = { imDrawData->DisplayPos.x, imDrawData->DisplayPos.y };
    m_pushConstantBlock.scale[0] = 2.0f / displaySize[0];
    m_pushConstantBlock.scale[1] = 2.0f / displaySize[1];
    m_pushConstantBlock.translate[0] = -1.0f - displayPos[0] * m_pushConstantBlock.scale[0];
    m_pushConstantBlock.translate[1] = -1.0f - displayPos[1] * m_pushConstantBlock.scale[1];

    recorder->pushConstant(
            PushConstantRange{
                    .offset = 0,
                    .size = sizeof(PushConstantBlock),
                    .shaderStages = ShaderStageFlagBits::VertexBit,
            },
            &m_pushConstantBlock);

    // Set Viewport and scissor rect
    recorder->setViewport(KDGpu::Viewport{
            .x = 0.0f,
            .y = 0.0f,
            .width = static_cast<float>(extent.width),
            .height = static_cast<float>(extent.height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
    });

    recorder->setScissor(KDGpu::Rect2D{
            .offset = { 0, 0 },
            .extent = extent,
    });

    // Bind the vertex and index buffers
    recorder->setVertexBuffer(0, m_mesh->vertices);
    recorder->setIndexBuffer(m_mesh->indexBuffer, 0, IndexType::Uint16);

    for (int32_t cmdIdx = 0; cmdIdx < imDrawData->CmdListsCount; cmdIdx++) {
        const ImDrawList *cmd_list = imDrawData->CmdLists[cmdIdx];

        for (int32_t j = 0; j < cmd_list->CmdBuffer.Size; j++) {
            const ImDrawCmd *pcmd = &cmd_list->CmdBuffer[j];

            // Set the scissor rect
            recorder->setScissor(KDGpu::Rect2D{
                    .offset = {
                            .x = std::max(static_cast<int32_t>(pcmd->ClipRect.x), 0),
                            .y = std::max(static_cast<int32_t>(pcmd->ClipRect.y), 0),
                    },
                    .extent = {
                            .width = static_cast<uint32_t>(pcmd->ClipRect.z - pcmd->ClipRect.x),
                            .height = static_cast<uint32_t>(pcmd->ClipRect.w - pcmd->ClipRect.y),
                    },
            });

            // And finally, draw a part of the UI
            recorder->drawIndexed(DrawIndexedCommand{
                    .indexCount = pcmd->ElemCount,
                    .firstIndex = indexOffset,
                    .vertexOffset = vertexOffset,
            });

            indexOffset += pcmd->ElemCount;
        }
        vertexOffset += cmd_list->VtxBuffer.Size;
    }
}

void ImGuiRenderer::initializeFontData(const float scaleFactor)
{
    ImGuiIO &io = ImGui::GetIO();
    io.Fonts->Clear();

    // Clear previous font texture, view
    m_texture = {};
    m_textureView = {};

    // Create font texture, view
    unsigned char *fontData;
    int texWidth, texHeight;
    auto fs = cmrc::KDGpuExample::Resources::get_filesystem();
    auto ttfFile = fs.open("fonts/Roboto-Medium.ttf");
    auto ttfData = const_cast<void *>(static_cast<const void *>(ttfFile.begin()));
    ImFontConfig fontConfig{};
    fontConfig.FontDataOwnedByAtlas = false;
    const float fontPixelSize = 18.0f * scaleFactor;
    io.Fonts->AddFontFromMemoryTTF(ttfData, ttfFile.size(), fontPixelSize, &fontConfig);
    io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);
    DeviceSize uploadSize = texWidth * texHeight * 4 * sizeof(char);

    const auto textureOptions = TextureOptions{
        .type = TextureType::TextureType2D,
        .format = Format::R8G8B8A8_UNORM,
        .extent = { .width = static_cast<uint32_t>(texWidth), .height = static_cast<uint32_t>(texHeight), .depth = 1 },
        .mipLevels = 1,
        .usage = TextureUsageFlagBits::SampledBit | TextureUsageFlagBits::TransferDstBit
    };
    m_texture = m_device->createTexture(textureOptions);

    // Upload the font texture data
    // clang-format off
    const std::vector<BufferTextureCopyRegion> regions = {{
        .textureSubResource = { .aspectMask = TextureAspectFlagBits::ColorBit },
        .textureExtent = { .width = static_cast<uint32_t>(texWidth), .height = static_cast<uint32_t>(texHeight), .depth = 1 }
    }};
    // clang-format on
    const WaitForTextureUploadOptions uploadOptions = {
        .destinationTexture = m_texture,
        .dstStages = PipelineStageFlagBit::FragmentShaderBit,
        .data = fontData,
        .byteSize = uploadSize,
        .oldLayout = TextureLayout::Undefined,
        .newLayout = TextureLayout::ShaderReadOnlyOptimal,
        .regions = regions
    };
    m_queue->waitForUploadTextureData(uploadOptions);

    m_textureView = m_texture.createView();

    // Update previous bind group if it exists
    if (m_bindGroup.isValid()) {
        const BindGroupEntry entry{
            .binding = 0,
            .resource = TextureViewSamplerBinding{ .textureView = m_textureView, .sampler = m_sampler }
        };

        m_bindGroup.update(entry);
    } else {
        // Create a bind group for the font texture
        // clang-format off
        const BindGroupOptions bindGroupOptions = {
            .layout = m_bindGroupLayout,
            .resources = {{
                .binding = 0,
                .resource = TextureViewSamplerBinding{ .textureView = m_textureView, .sampler = m_sampler }
            }}
        };
        // clang-format on
        m_bindGroup = m_device->createBindGroup(bindGroupOptions);
    }
}

} // namespace KDGpuExample
