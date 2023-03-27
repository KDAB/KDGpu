#pragma once

#include <kdgpu_kdgui/simple_example_engine_layer.h>

#include <kdgpu/bind_group.h>
#include <kdgpu/buffer.h>
#include <kdgpu/graphics_pipeline.h>
#include <kdgpu/render_pass_command_recorder_options.h>

#include <glm/glm.hpp>

using namespace KDGpuKDGui;

class ComputeParticles : public SimpleExampleEngineLayer
{
public:
protected:
    void initializeScene() override;
    void cleanupScene() override;
    void updateScene() override;
    void render() override;
    void resize() override;

    void renderSingleCommandBuffer();
    void renderMultipleCommandBuffers();

private:
    Buffer m_particleDataBuffer;
    Buffer m_triangleVertexBuffer;
    ComputePipeline m_computePipeline;
    GraphicsPipeline m_graphicsPipeline;
    PipelineLayout m_graphicsPipelineLayout;
    PipelineLayout m_computePipelineLayout;
    RenderPassCommandRecorderOptions m_opaquePassOptions;
    CommandBuffer m_graphicsCommands;
    CommandBuffer m_computeCommands;
    CommandBuffer m_graphicsAndComputeCommands;

    BindGroup m_particleBindGroup;
    GpuSemaphore m_computeSemaphoreComplete;
};
