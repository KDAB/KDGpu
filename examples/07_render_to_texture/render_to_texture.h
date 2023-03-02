#pragma once

#include <toy_renderer_kdgui/example_engine_layer.h>

#include <toy_renderer/bind_group.h>
#include <toy_renderer/buffer.h>
#include <toy_renderer/graphics_pipeline.h>
#include <toy_renderer/render_pass_command_recorder_options.h>

#include <glm/glm.hpp>

using namespace ToyRendererKDGui;

class RenderToTexture : public ExampleEngineLayer
{
public:
protected:
    void initializeScene() override;
    void cleanupScene() override;
    void updateScene() override;
    void render() override;

private:
    void initializeMainScene();
    void initializePostProcess();

    // Main scene resources
    Buffer m_buffer;
    Buffer m_indexBuffer;
    PipelineLayout m_pipelineLayout;
    GraphicsPipeline m_pipeline;

    glm::mat4 m_transform;
    Buffer m_transformBuffer;
    BindGroup m_transformBindGroup;

    // Post process resources
    Buffer m_fullScreenQuad;
    PipelineLayout m_postProcessPipelineLayout;
    GraphicsPipeline m_postProcessPipeline;
    BindGroup m_colorBindGroup;
    const PushConstantRange m_filterPosPushConstantRange{
        .offset = 0,
        .size = sizeof(float),
        .shaderStages = ShaderStageFlags(ShaderStageFlagBits::FragmentBit)
    };
    std::vector<uint8_t> m_filterPosData = { sizeof(float) };
    float m_filterPos{ 0.0f };

    // Rendering resources
    const Format m_colorFormat{ Format::R8G8B8A8_UNORM };
    Texture m_colorOutput;
    TextureView m_colorOutputView;
    Sampler m_colorOutputSampler;
    RenderPassCommandRecorderOptions m_opaquePassOptions;
    RenderPassCommandRecorderOptions m_finalPassOptions;
};
