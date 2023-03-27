#pragma once

#include <kdgpu_kdgui/simple_example_engine_layer.h>

#include <kdgpu/bind_group.h>
#include <kdgpu/buffer.h>
#include <kdgpu/graphics_pipeline.h>
#include <kdgpu/render_pass_command_recorder_options.h>

#include <glm/glm.hpp>

using namespace KDGpuKDGui;

class RotatingTriangle : public SimpleExampleEngineLayer
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
    Buffer m_indexBuffer;
    PipelineLayout m_pipelineLayout;
    GraphicsPipeline m_pipeline;
    RenderPassCommandRecorderOptions m_opaquePassOptions;
    CommandBuffer m_commandBuffer;

    glm::mat4 m_transform;
    Buffer m_transformBuffer;
    BindGroup m_transformBindGroup;
};
