#pragma once

#include <toy_renderer_kdgui/simple_example_engine_layer.h>

#include <toy_renderer/bind_group.h>
#include <toy_renderer/buffer.h>
#include <toy_renderer/graphics_pipeline.h>
#include <toy_renderer/render_pass_command_recorder_options.h>

#include <glm/glm.hpp>

#include <cmath>

using namespace ToyRendererKDGui;

struct Vec2DAnimation {
    glm::vec2 start{ 0.0f, 0.0f }; // Start
    glm::vec2 end{ 1.0f, 1.0f }; // End
    float period; // Duration in seconds

    inline glm::vec2 evaluate(float t) const
    {
        float u = 0.5f * (std::sin(2.0f * M_PI * t / period) + 1.0f);
        return start + u * (end - start);
    }
};

class StyledRectangles : public SimpleExampleEngineLayer
{
public:
protected:
    void initializeScene() override;
    void cleanupScene() override;
    void updateScene() override;
    void render() override;

private:
    void initializeRectangles();
    void initializeBackground();

    // Rectangle resources
    Buffer m_normalizedQuad;
    Buffer m_globalBuffer;
    Buffer m_rectBuffer;
    BindGroup m_rectBindGroup;
    PipelineLayout m_rectPipelineLayout;
    GraphicsPipeline m_rectPipeline;

    // Background resources
    Buffer m_fullScreenQuad;
    Buffer m_colorStopsBuffer;
    BindGroup m_colorStopsBindGroup;
    PipelineLayout m_bgPipelineLayout;
    GraphicsPipeline m_bgPipeline;

    RenderPassCommandRecorderOptions m_renderPassOptions;
    CommandBuffer m_commandBuffer;

    // Background data
    glm::vec4 m_color0{ 190.0f / 255.0f, 186.0f / 255.0f, 255.0f / 255.0f, 1.0 }; // Top-left
    glm::vec4 m_color1{ 230.0f / 255.0f, 161.0f / 255.0f, 243.0f / 255.0f, 1.0 }; // Top-right
    glm::vec4 m_color2{ 143.0f / 255.0f, 143.0f / 255.0f, 245.0f / 255.0f, 1.0 }; // Bottom-left
    glm::vec4 m_color3{ 189.0f / 255.0f, 153.0f / 255.0f, 246.0f / 255.0f, 1.0 }; // Bottom-right
    glm::vec2 m_p0{ 0.35f, 0.20f }; // Top-left
    glm::vec2 m_p1{ 0.95f, 0.05f }; // Top-right
    glm::vec2 m_p2{ 0.05f, 0.90f }; // Bottom-left
    glm::vec2 m_p3{ 0.80f, 0.85f }; // Bottom-right

    // Animation data
    Vec2DAnimation m_p0Anim{ { 0.35f, 0.20f }, { 0.05f, 0.10f }, 10.0f };
    Vec2DAnimation m_p1Anim{ { 0.95f, 0.05f }, { 0.65f, 0.10f }, 13.0f };
    Vec2DAnimation m_p2Anim{ { 0.05f, 0.90f }, { 0.15f, 0.95f }, 23.0f };
    Vec2DAnimation m_p3Anim{ { 0.80f, 0.85f }, { 0.65f, 0.90f }, 8.0f };
};
