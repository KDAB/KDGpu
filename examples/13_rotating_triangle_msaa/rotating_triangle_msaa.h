#pragma once

#include <toy_renderer_kdgui/simple_example_engine_layer.h>

#include <toy_renderer/bind_group.h>
#include <toy_renderer/buffer.h>
#include <toy_renderer/graphics_pipeline.h>
#include <toy_renderer/render_pass_command_recorder_options.h>
#include <toy_renderer/texture.h>
#include <toy_renderer/texture_view.h>

#include <glm/glm.hpp>

using namespace ToyRendererKDGui;

class RotatingTriangleMSAA : public SimpleExampleEngineLayer
{
public:
    RotatingTriangleMSAA();

protected:
    void initializeScene() override;
    void cleanupScene() override;
    void updateScene() override;
    void render() override;
    void resize() override;

private:
    void createRenderTarget();

    Buffer m_buffer;
    Buffer m_indexBuffer;
    Texture m_msaaTexture;
    TextureView m_msaaTextureView;
    PipelineLayout m_pipelineLayout;
    GraphicsPipeline m_pipeline;
    RenderPassCommandRecorderOptions m_opaquePassOptions;
    CommandBuffer m_commandBuffer;

    glm::mat4 m_transform;
    Buffer m_transformBuffer;
    BindGroup m_transformBindGroup;
};
