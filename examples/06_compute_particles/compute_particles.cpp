#include "compute_particles.h"

#include <toy_renderer_kdgui/engine.h>

#include <toy_renderer/bind_group_layout_options.h>
#include <toy_renderer/bind_group_options.h>
#include <toy_renderer/buffer_options.h>
#include <toy_renderer/graphics_pipeline_options.h>
#include <toy_renderer/compute_pipeline_options.h>

#include <glm/glm.hpp>

#include <cmath>
#include <random>
#include <cassert>

namespace ToyRenderer {

inline std::string assetPath()
{
#if defined(TOY_RENDERER_ASSET_PATH)
    return TOY_RENDERER_ASSET_PATH;
#else
    return "";
#endif
}

} // namespace ToyRenderer

namespace {

constexpr size_t ParticlesCount = 1024;

struct Vertex {
    glm::vec3 position;
};
static_assert(sizeof(Vertex) == 3 * sizeof(float));

struct ParticleData {
    glm::vec4 position;
    glm::vec4 velocity;
    glm::vec4 color;
};
static_assert(sizeof(ParticleData) == 12 * sizeof(float));

std::vector<ParticleData> initializeParticles(const size_t particlesCount)
{
    std::vector<ParticleData> particles(particlesCount);

    std::random_device rd; // Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()

    std::uniform_int_distribution<> posDistrib(0, 1024);
    std::uniform_int_distribution<> colorDistrib(0, 255);

    for (ParticleData &particle : particles) {
        for (size_t i = 0; i < 3; ++i) {
            particle.position[i] = (float(posDistrib(gen)) - 512.0f) / 512.0f * 0.01f;
            particle.velocity[i] = float(posDistrib(gen)) / 512.0f * 0.01f;
            particle.color[i] = float(colorDistrib(gen)) / 255.0f;
        }
        particle.position[3] = 1.0;
        particle.velocity[3] = 0.0;
        particle.color[3] = 1.0;
    }

    return particles;
}
} // namespace

void ComputeParticles::initializeScene()
{

    ///////////////////// BUFFERS /////////////////////////////

    auto initializeBuffers = [this]() {
        // Create a buffer to hold particles data (will be used as per Instance data)
        {
            const BufferOptions particlesBufferOptions = {
                .size = ParticlesCount * sizeof(ParticleData),
                .usage = BufferUsageFlags(BufferUsageFlagBits::VertexBufferBit) | BufferUsageFlags(BufferUsageFlagBits::StorageBufferBit), // TODO: Use a nice Flags template class
                .memoryUsage = MemoryUsage::CpuToGpu // So we can map it to CPU address space
            };
            const std::vector<ParticleData> particles = initializeParticles(ParticlesCount);
            m_particleDataBuffer = m_device.createBuffer(particlesBufferOptions, particles.data());
        }

        // Create a buffer to hold the triangle vertex data
        {
            const BufferOptions triangleBufferOptions = {
                .size = 3 * sizeof(Vertex),
                .usage = BufferUsageFlags(BufferUsageFlagBits::VertexBufferBit),
                .memoryUsage = MemoryUsage::CpuToGpu
            };

            const float r = 0.08f;
            std::array<Vertex, 3> vertexData;
            vertexData[0] = { { r * std::cos(7.0f * M_PI / 6.0f), -r * std::sin(7.0f * M_PI / 6.0f), 0.0f } }; // Bottom-left
            vertexData[1] = { { r * std::cos(11.0f * M_PI / 6.0f), -r * std::sin(11.0f * M_PI / 6.0f), 0.0f } }; // Bottom-right
            vertexData[2] = { { 0.0f, -r, 0.0f } }; // Top
            m_triangleVertexBuffer = m_device.createBuffer(triangleBufferOptions, vertexData.data());
        }
    };

    initializeBuffers();

    //////////////////// PIPELINES ////////////////////////////

    auto initializeComputePipeline = [this]() {
        // Create a compute shader (spir-v only for now)
        const auto computeShaderPath = ToyRenderer::assetPath() + "/shaders/examples/06_compute_particles/particles.comp.spv";
        auto computeShader = m_device.createShaderModule(ToyRenderer::readShaderFile(computeShaderPath));

        // Create bind group layout consisting of a single binding holding a SSBO
        // clang-format off
        const BindGroupLayoutOptions bindGroupLayoutOptions = {
            .bindings = {{
                .binding = 0,
                .resourceType = ResourceBindingType::StorageBuffer,
                .shaderStages = ShaderStageFlags(ShaderStageFlagBits::ComputeBit)
            }}
        };
        // clang-format on
        const BindGroupLayout bindGroupLayout = m_device.createBindGroupLayout(bindGroupLayoutOptions);

        // Create a pipeline layout (array of bind group layouts)
        const PipelineLayoutOptions pipelineLayoutOptions = {
            .bindGroupLayouts = { bindGroupLayout }
        };
        auto pipelineLayout = m_device.createPipelineLayout(pipelineLayoutOptions);

        // Create a bindGroup to hold the UBO with the transform
        // clang-format off
        const BindGroupOptions bindGroupOptions {
            .layout = bindGroupLayout,
            .resources = {{
                .binding = 0,
                .resource = BindingResource(StorageBufferBinding{ .buffer = m_particleDataBuffer })
            }}
        };
        // clang-format on
        m_particleBindGroup = m_device.createBindGroup(bindGroupOptions);

        const ComputePipelineOptions pipelineOptions{
            .layout = pipelineLayout,
            .shaderStage = { .shaderModule = computeShader }
        };

        m_computePipeline = m_device.createComputePipeline(pipelineOptions);
    };

    auto initializeGraphicsPipeline = [this]() {
        // Create a vertex shader and fragment shader (spir-v only for now)
        const auto vertexShaderPath = ToyRenderer::assetPath() + "/shaders/examples/06_compute_particles/triangle.vert.spv";
        auto vertexShader = m_device.createShaderModule(ToyRenderer::readShaderFile(vertexShaderPath));

        const auto fragmentShaderPath = ToyRenderer::assetPath() + "/shaders/examples/06_compute_particles/triangle.frag.spv";
        auto fragmentShader = m_device.createShaderModule(ToyRenderer::readShaderFile(fragmentShaderPath));

        // Create a pipeline layout (array of bind group layouts)
        const PipelineLayoutOptions pipelineLayoutOptions = {};
        auto pipelineLayout = m_device.createPipelineLayout(pipelineLayoutOptions);

        // Create a pipeline
        // clang-format off
        const GraphicsPipelineOptions pipelineOptions = {
            .shaderStages = {
                { .shaderModule = vertexShader, .stage = ShaderStageFlagBits::VertexBit },
                { .shaderModule = fragmentShader, .stage = ShaderStageFlagBits::FragmentBit }
            },
            .layout = pipelineLayout,
            .vertex = {
                .buffers = {
                    { .binding = 0, .stride = sizeof(Vertex) },
                    { .binding = 1, .stride = sizeof(ParticleData), .inputRate = VertexRate::Instance }
                },
                .attributes = {
                    { .location = 0, .binding = 0, .format = Format::R32G32B32_SFLOAT }, // Vertex Position
                    { .location = 1, .binding = 1, .format = Format::R32G32B32A32_SFLOAT }, // Particle Position
                    { .location = 2, .binding = 1, .format = Format::R32G32B32A32_SFLOAT, .offset = 2 * sizeof(glm::vec4) } // Particle Color
                }
            },
            .renderTargets = {
                { .format = m_swapchainFormat }
            },
            .depthStencil = {
                .format = m_depthFormat,
                .depthWritesEnabled = true,
                .depthCompareOperation = CompareOperation::Less
            }
        };
        // clang-format on
        m_graphicsPipeline = m_device.createGraphicsPipeline(pipelineOptions);

        // Most of the render pass is the same between frames. The only thing that changes, is which image
        // of the swapchain we wish to render to. So set up what we can here, and in the render loop we will
        // just update the color texture view.
        // clang-format off
        m_opaquePassOptions = {
            .colorAttachments = {
                {
                    .view = {}, // Not setting the swapchain texture view just yet
                    .clearValue = { 0.3f, 0.3f, 0.3f, 1.0f },
                    .finalLayout = TextureLayout::PresentSrc
                }
            },
            .depthStencilAttachment = {
                .view = m_depthTextureView,
            }
        };
        // clang-format on
    };

    initializeComputePipeline();
    initializeGraphicsPipeline();

    // Initialize a Semaphore to sync Compute & Render
    m_computeSemaphoreComplete = m_device.createGpuSemaphore();
}

void ComputeParticles::cleanupScene()
{
    // TODO: Properly handle destroying the underlying resources
    m_computePipeline = {};
    m_graphicsPipeline = {};
    m_particleDataBuffer = {};
    m_triangleVertexBuffer = {};
    m_opaquePassOptions = {};
    m_particleBindGroup = {};
    m_computeSemaphoreComplete = {};
}

void ComputeParticles::updateScene()
{
}

void ComputeParticles::render()
{
    // Prepare Command Buffers

    // Compute
    auto computeCommandRecorder = m_device.createCommandRecorder();
    {
        auto computePass = computeCommandRecorder.beginComputePass();
        computePass.setPipeline(m_computePipeline);
        computePass.setBindGroup(0, m_particleBindGroup);
        constexpr size_t LocalWorkGroupXSize = 256;
        computePass.dispatchCompute(ComputeCommand{ .workGroupX = ParticlesCount / LocalWorkGroupXSize });
        computePass.end();
    }
    const auto computeCommands = computeCommandRecorder.finish();

    // Render
    auto graphicsCommandRecorder = m_device.createCommandRecorder();
    {
        m_opaquePassOptions.colorAttachments[0].view = m_swapchainViews.at(m_currentSwapchainImageIndex);
        auto opaquePass = graphicsCommandRecorder.beginRenderPass(m_opaquePassOptions);
        opaquePass.setPipeline(m_graphicsPipeline);
        opaquePass.setVertexBuffer(0, m_triangleVertexBuffer);
        opaquePass.setVertexBuffer(1, m_particleDataBuffer); // Per instance Data
        opaquePass.draw(DrawCommand{ .vertexCount = 3, .instanceCount = ParticlesCount });
        opaquePass.end();
    }
    const auto graphicsCommands = graphicsCommandRecorder.finish();

    // Submit Commands

    // We first submit compute commands
    SubmitOptions computeSubmitOptions = {
        .commandBuffers = { computeCommands },
        .waitSemaphores = { m_presentCompleteSemaphores[m_inFlightIndex] },
        .signalSemaphores = { m_computeSemaphoreComplete }
    };
    m_queue.submit(computeSubmitOptions);

    // Then we submit the graphics command, we rely on a semaphore to ensure
    // graphics commands don't start prior to the compute commands being completed
    SubmitOptions graphicsSubmitOptions = {
        .commandBuffers = { graphicsCommands },
        .waitSemaphores = { m_computeSemaphoreComplete },
        .signalSemaphores = { m_renderCompleteSemaphores[m_inFlightIndex] }
    };
    m_queue.submit(graphicsSubmitOptions);
}
