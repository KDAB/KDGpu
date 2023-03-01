#pragma once

#include <toy_renderer_kdgui/example_engine_layer.h>

#include <toy_renderer/bind_group.h>
#include <toy_renderer/buffer.h>
#include <toy_renderer/graphics_pipeline.h>
#include <toy_renderer/render_pass_command_recorder_options.h>

#include <glm/glm.hpp>

using namespace ToyRendererKDGui;

class ComputeParticles : public ExampleEngineLayer
{
public:
protected:
    void initializeScene() override;
    void cleanupScene() override;
    void updateScene() override;
    void render() override;

private:
    Buffer m_particleDataBuffer;
    Buffer m_triangleVertexBuffer;
    ComputePipeline m_computePipeline;
    GraphicsPipeline m_graphicsPipeline;
    RenderPassCommandRecorderOptions m_opaquePassOptions;

    BindGroup m_particleBindGroup;
    GpuSemaphore m_computeSemaphoreComplete;
};
