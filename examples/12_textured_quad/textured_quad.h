#pragma once

#include <kdgpu_kdgui/simple_example_engine_layer.h>

#include <kdgpu/bind_group.h>
#include <kdgpu/buffer.h>
#include <kdgpu/graphics_pipeline.h>
#include <kdgpu/render_pass_command_recorder_options.h>
#include <kdgpu/texture.h>

#include <glm/glm.hpp>

using namespace KDGpuKDGui;

class TexturedQuad : public SimpleExampleEngineLayer
{
public:
protected:
    void initializeScene() override;
    void cleanupScene() override;
    void updateScene() override;
    void render() override;
    void resize() override;

private:
    Buffer m_buffer;
    Texture m_texture;
    TextureView m_textureView;
    Sampler m_sampler;
    BindGroup m_textureBindGroup;
    PipelineLayout m_pipelineLayout;
    GraphicsPipeline m_pipeline;
    RenderPassCommandRecorderOptions m_opaquePassOptions;
    CommandBuffer m_commandBuffer;
};
