#pragma once

#include <KDGpuExample/simple_example_engine_layer.h>

#include <KDGpu/bind_group.h>
#include <KDGpu/buffer.h>
#include <KDGpu/graphics_pipeline.h>
#include <KDGpu/render_pass_command_recorder_options.h>

using namespace KDGpuExample;

class MultiView : public SimpleExampleEngineLayer
{
public:
protected:
    void initializeScene() override;
    void cleanupScene() override;
    void updateScene() override;
    void render() override;
    void resize() override;

private:
    void initializeMultiViewPass();
    void initializeFullScreenPass();
    void createMultiViewOffscreenTextures();
    void updateFinalPassBindGroup();

    // MultiView Scene
    Buffer m_vertexBuffer;
    PipelineLayout m_mvPipelineLayout;
    GraphicsPipeline m_mvPipeline;
    const PushConstantRange m_mvPushConstantRange{
        .offset = 0,
        .size = sizeof(float),
        .shaderStages = ShaderStageFlags(ShaderStageFlagBits::VertexBit)
    };

    // Full Screen Quad Scene
    PipelineLayout m_fsqPipelineLayout;
    GraphicsPipeline m_fsqPipeline;
    BindGroupLayout m_fsqTextureBindGroupLayout;
    BindGroup m_fsqTextureBindGroup;
    const PushConstantRange m_fsqLayerIdxPushConstantRange{
        .offset = 0,
        .size = sizeof(int),
        .shaderStages = ShaderStageFlags(ShaderStageFlagBits::FragmentBit)
    };

    Texture m_multiViewColorOutput;
    Texture m_multiViewDepth;
    TextureView m_multiViewColorOutputView;
    TextureView m_multiViewDepthView;

    Sampler m_multiViewColorOutputSampler;

    RenderPassCommandRecorderOptions m_mvPassOptions;
    RenderPassCommandRecorderOptions m_fsqPassOptions;
    CommandBuffer m_commandBuffer;

    const Format m_mvColorFormat{ Format::R8G8B8A8_UNORM };
    const Format m_mvDepthFormat{ Format::D24_UNORM_S8_UINT };
};
