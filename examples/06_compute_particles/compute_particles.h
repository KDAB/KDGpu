#pragma once

#include <KDGpuExample/simple_example_engine_layer.h>

#include <KDGpu/bind_group.h>
#include <KDGpu/buffer.h>
#include <KDGpu/graphics_pipeline.h>
#include <KDGpu/render_pass_command_recorder_options.h>

#include <glm/glm.hpp>

using namespace KDGpuExample;

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
