#include <toy_renderer/vulkan/vulkan_resource_manager.h>

#include <toy_renderer/bind_group_description.h>

#include <iostream>
#include <map>
#include <span>
#include <vector>

using namespace ToyRenderer;

class Pipeline;

enum class IndexFormat {
    UINT16 = 0,
    UINT32 = 1
};

class RenderPass
{
public:
    // void begin();
    // void end();

    void setBindGroup(uint32_t groupSlot, const Handle<BindGroup> &bindGroup) { }
    void setPipeline(const Handle<Pipeline> &pipeline) { }
    void setVertexBuffer(uint32_t index, const Handle<Buffer> &buffer, uint64_t offset, uint64_t size) { }
    void setIndexBuffer(const Handle<Buffer> &buffer, IndexFormat format, uint64_t offset, uint64_t size) { }
    void draw(uint32_t drawCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) { }

    // TODO: Add drawIndexed()
};

// Data maintained by the renderer
Handle<BindGroup> frameBindGroup;
Handle<BindGroup> instancesBindGroup;

struct Material {
    Handle<BindGroup> bindGroup;
};

struct GpuPrimitive {
    std::vector<BufferBinding> vertexBuffers;

    uint32_t drawCount;
    struct Instances {
        uint32_t count;
        uint32_t first;
    } instances;
};

using GpuPrimitives = std::vector<GpuPrimitive>;

struct GpuPipeline {
    Handle<Pipeline> pipeline;
    std::map<Material, GpuPrimitives> materialPrimitives;
};

std::vector<GpuPipeline> gpuPipelineData;

// Pseudo-code for a render loop
void renderGltf(RenderPass &renderPass)
{
    renderPass.setBindGroup(0, frameBindGroup);
    renderPass.setBindGroup(1, instancesBindGroup);

    for (const auto &gpuPipeline : gpuPipelineData) {
        renderPass.setPipeline(gpuPipeline.pipeline);

        // Loop through every material that uses this pipeline and get an array of primitives
        // that uses that material.
        for (const auto [material, primitives] : gpuPipeline.materialPrimitives) {
            // Set the material bind group.
            renderPass.setBindGroup(2, material.bindGroup);

            // Loop through the primitives that use the current material/pipeline combo and draw
            // them as usual.
            for (const auto &gpuPrimitive : primitives) {
                for (const auto &vertexBuffer : gpuPrimitive.vertexBuffers) {
                    renderPass.setVertexBuffer(vertexBuffer.slot, vertexBuffer.buffer, vertexBuffer.offset, vertexBuffer.size);
                }

                renderPass.draw(gpuPrimitive.drawCount, gpuPrimitive.instances.count, 0, gpuPrimitive.instances.first);
            }
        }
    }
}

int main()
{
    // Assume we have a working Vulkan resource manager;
    ResourceManager *resourceManager = new VulkanResourceManager;

    // Assume we have some textures and a buffer for a material and we want to bind these...
    Handle<Texture> textureBaseColor;
    Handle<Texture> textureMetalRough;
    Handle<Texture> textureNormal;
    Handle<Buffer> materialUniforms;

    TextureBinding textureBindings[3];
    textureBindings[0].texture = textureBaseColor;
    textureBindings[0].slot = 0;
    textureBindings[1].texture = textureMetalRough;
    textureBindings[1].slot = 1;
    textureBindings[2].texture = textureNormal;
    textureBindings[2].slot = 2;

    BufferBinding bufferBindings[1];
    bufferBindings[0].buffer = materialUniforms;
    bufferBindings[0].slot = 0;

    BindGroupDescription bindGroupDescription;
    bindGroupDescription.textures = textureBindings;
    bindGroupDescription.buffers = bufferBindings;

    // Create the bind group
    Handle<BindGroup> bindGroup = resourceManager->createBindGroup(bindGroupDescription);

    // And release it again
    resourceManager->deleteBindGroup(bindGroup);

    return 0;
}
