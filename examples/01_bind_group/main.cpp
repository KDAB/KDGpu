#include <toy_renderer/vulkan/vulkan_resource_manager.h>

#include <toy_renderer/bind_group_description.h>

#include <iostream>
#include <span>

using namespace ToyRenderer;

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
