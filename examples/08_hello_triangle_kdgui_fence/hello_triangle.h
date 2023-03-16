#pragma once

#include <toy_renderer_kdgui/advanced_example_engine_layer.h>

#include <toy_renderer/buffer.h>
#include <toy_renderer/graphics_pipeline.h>
#include <toy_renderer/render_pass_command_recorder_options.h>

#include <glm/glm.hpp>

using namespace ToyRendererKDGui;

class HelloTriangle : public AdvancedExampleEngineLayer
{
public:
protected:
    void initializeScene() override;
    void cleanupScene() override;
    void updateScene() override;
    void render() override;
    void resize() override;

private:
    void updateClearColor();
    void updateTransform();

    glm::mat4 m_transform = glm::mat4(1.0f);
    Buffer m_buffer;
    GraphicsPipeline m_pipeline;
    PipelineLayout m_pipelineLayout;
    RenderPassCommandRecorderOptions m_opaquePassOptions;
    const PushConstantRange m_transformPushConstantRange{ .offset = 0,
                                                          .size = 16 * sizeof(float),
                                                          .shaderStages = ShaderStageFlags(ShaderStageFlagBits::VertexBit) };
    std::array<CommandBuffer, MAX_FRAMES_IN_FLIGHT> m_commandBuffers;
};
