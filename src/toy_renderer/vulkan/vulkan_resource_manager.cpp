#include "vulkan_resource_manager.h"

#include <toy_renderer/buffer_options.h>
#include <toy_renderer/graphics_pipeline_options.h>
#include <toy_renderer/instance.h>
#include <toy_renderer/swapchain_options.h>
#include <toy_renderer/texture_options.h>
#include <toy_renderer/vulkan/vulkan_config.h>
#include <toy_renderer/vulkan/vulkan_enums.h>

#include <assert.h>
#include <stdexcept>

namespace ToyRenderer {

VulkanResourceManager::VulkanResourceManager()
{
}

VulkanResourceManager::~VulkanResourceManager()
{
}

Handle<Instance_t> VulkanResourceManager::createInstance(const InstanceOptions &options)
{
    // Populate some basic application and engine info
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = options.applicationName.data();
    appInfo.applicationVersion = options.applicationVersion;
    appInfo.pEngineName = "Serenity Prototype";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    if (!requestedInstanceLayers.empty()) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(requestedInstanceLayers.size());
        assert(requestedInstanceLayers.size() <= std::numeric_limits<uint32_t>::max());
        createInfo.ppEnabledLayerNames = requestedInstanceLayers.data();
    }

    const auto requestedInstanceExtensions = getDefaultRequestedInstanceExtensions();
    if (!requestedInstanceExtensions.empty()) {
        createInfo.enabledExtensionCount = static_cast<uint32_t>(requestedInstanceExtensions.size());
        assert(requestedInstanceExtensions.size() <= std::numeric_limits<uint32_t>::max());
        createInfo.ppEnabledExtensionNames = requestedInstanceExtensions.data();
    }

    // Try to create the instance
    VkInstance instance = VK_NULL_HANDLE;
    const VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to create Vulkan instance!");
    }

    VulkanInstance vulkanInstance(this, instance);
    auto h = m_instances.emplace(vulkanInstance);
    return h;
}

void VulkanResourceManager::deleteInstance(Handle<Instance_t> handle)
{
    // TODO: Implement me!
}

Handle<Adapter_t> VulkanResourceManager::insertAdapter(const VulkanAdapter &physicalDevice)
{
    return m_adapters.emplace(physicalDevice);
}

void VulkanResourceManager::removeAdapter(Handle<Adapter_t> handle)
{
    m_adapters.remove(handle);
}

/*
 * Create a VkDevice (logical device) from the provided adapter (physical device) and requested options.
 * If no options are specified we request a single queue from the first family (usually graphics capable).
 */
Handle<Device_t> VulkanResourceManager::createDevice(const Handle<Adapter_t> &adapterHandle, const DeviceOptions &options, std::vector<QueueRequest> &queueRequests)
{
    queueRequests = options.queues;
    if (queueRequests.empty()) {
        QueueRequest queueRequest = {
            .queueTypeIndex = 0,
            .count = 1,
            .priorities = { 1.0f }
        };
        queueRequests.emplace_back(queueRequest);
    }

    uint32_t queueRequestCount = queueRequests.size();
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    queueCreateInfos.reserve(queueRequestCount);
    for (uint32_t i = 0; i < queueRequestCount; ++i) {
        const auto &queueRequest = queueRequests[i];

        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueRequest.queueTypeIndex;
        queueCreateInfo.queueCount = queueRequest.count;
        queueCreateInfo.pQueuePriorities = queueRequest.priorities.data();

        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pNext = nullptr; // TODO: Use VkPhysicalDeviceFeatures2
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pEnabledFeatures = nullptr;
    createInfo.enabledLayerCount = 0;
    createInfo.ppEnabledLayerNames = nullptr;
    createInfo.enabledExtensionCount = 0;
    createInfo.ppEnabledExtensionNames = nullptr;

    // TODO: Obey requested adapter features (e.g. geometry shaders)
    // TODO: Merge requested device extensions and layers with our defaults
    const auto requestedDeviceExtensions = getDefaultRequestedDeviceExtensions();
    if (!requestedDeviceExtensions.empty()) {
        createInfo.enabledExtensionCount = static_cast<uint32_t>(requestedDeviceExtensions.size());
        assert(requestedDeviceExtensions.size() <= std::numeric_limits<uint32_t>::max());
        createInfo.ppEnabledExtensionNames = requestedDeviceExtensions.data();
    }

    VkDevice vkDevice{ VK_NULL_HANDLE };
    VulkanAdapter vulkanAdapter = *getAdapter(adapterHandle);
    if (vkCreateDevice(vulkanAdapter.physicalDevice, &createInfo, nullptr, &vkDevice) != VK_SUCCESS)
        throw std::runtime_error("Failed to create a logical device!");

    const auto deviceHandle = m_devices.emplace(vkDevice, this, adapterHandle);

    return deviceHandle;
}

void VulkanResourceManager::deleteDevice(Handle<Device_t> handle)
{
    // TODO: Implement me!
}

Handle<Queue_t> VulkanResourceManager::insertQueue(const VulkanQueue &vulkanQueue)
{
    return m_queues.emplace(vulkanQueue);
}

void VulkanResourceManager::removeQueue(Handle<Queue_t> handle)
{
    m_queues.remove(handle);
}

Handle<Swapchain_t> VulkanResourceManager::createSwapchain(const Handle<Device_t> &deviceHandle,
                                                           const SwapchainOptions &options)
{
    VulkanDevice vulkanDevice = *m_devices.get(deviceHandle);
    VulkanSurface vulkanSurface = *m_surfaces.get(options.surface);

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = vulkanSurface.surface;
    createInfo.minImageCount = options.minImageCount;
    createInfo.imageFormat = formatToVkFormat(options.format);
    createInfo.imageColorSpace = colorSpaceToVkColorSpaceKHR(options.colorSpace);
    createInfo.imageExtent = { .width = options.imageExtent.width, .height = options.imageExtent.height };
    createInfo.imageArrayLayers = options.imageLayers;
    createInfo.imageUsage = options.imageUsageFlags;
    createInfo.imageSharingMode = sharingModeToVkSharingMode(options.imageSharingMode);
    if (!options.queueTypeIndices.empty()) {
        createInfo.queueFamilyIndexCount = options.queueTypeIndices.size();
        createInfo.pQueueFamilyIndices = options.queueTypeIndices.data();
    }
    createInfo.preTransform = surfaceTransformFlagBitsToVkSurfaceTransformFlagBitsKHR(options.transform);
    createInfo.compositeAlpha = compositeAlphaFlagBitsToVkCompositeAlphaFlagBitsKHR(options.compositeAlpha);
    createInfo.presentMode = presentModeToVkPresentModeKHR(options.presentMode);
    createInfo.clipped = options.clipped;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    VkSwapchainKHR vkSwapchain{ VK_NULL_HANDLE };
    if (vkCreateSwapchainKHR(vulkanDevice.device, &createInfo, nullptr, &vkSwapchain) != VK_SUCCESS)
        return {};

    const auto swapchainHandle = m_swapchains.emplace(VulkanSwapchain{
            vkSwapchain,
            options.format,
            options.imageUsageFlags,
            this,
            deviceHandle });
    return swapchainHandle;
}

void VulkanResourceManager::deleteSwapchain(Handle<Swapchain_t> handle)
{
}

Handle<Surface_t> VulkanResourceManager::insertSurface(const VulkanSurface &vulkanSurface)
{
    return m_surfaces.emplace(vulkanSurface);
}

void VulkanResourceManager::deleteSurface(Handle<Surface_t> handle)
{
    VulkanSurface *vulkanSurface = m_surfaces.get(handle);
    if (vulkanSurface == nullptr)
        return;
    vkDestroySurfaceKHR(vulkanSurface->instance, vulkanSurface->surface, nullptr);
}

Handle<Texture_t> VulkanResourceManager::insertTexture(const VulkanTexture &vulkanTexture)
{
    return m_textures.emplace(vulkanTexture);
}

void VulkanResourceManager::removeTexture(Handle<Texture_t> handle)
{
    m_textures.remove(handle);
}

Handle<Texture_t> VulkanResourceManager::createTexture(const Handle<Device_t> deviceHandle, const TextureOptions &options)
{
    VulkanDevice vulkanDevice = *m_devices.get(deviceHandle);

    VkImageCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    createInfo.imageType = textureTypeToVkImageType(options.type);
    createInfo.format = formatToVkFormat(options.format);
    createInfo.extent = {
        .width = options.extent.width,
        .height = options.extent.height,
        .depth = options.extent.depth
    };
    createInfo.mipLevels = options.mipLevels;
    createInfo.arrayLayers = options.arrayLayers;
    createInfo.samples = sampleCountFlagBitsToVkSampleFlagBits(options.samples);
    createInfo.tiling = textureTilingToVkImageTiling(options.tiling);
    createInfo.usage = options.usage;
    createInfo.sharingMode = sharingModeToVkSharingMode(options.sharingMode);
    if (!options.queueTypeIndices.empty()) {
        createInfo.queueFamilyIndexCount = options.queueTypeIndices.size();
        createInfo.pQueueFamilyIndices = options.queueTypeIndices.data();
    }
    createInfo.initialLayout = textureLayoutToVkImageLayout(options.initialLayout);

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = memoryUsageToVmaMemoryUsage(options.memoryUsage);

    VkImage vkImage;
    VmaAllocation vmaAllocation;

    if (vmaCreateImage(vulkanDevice.allocator, &createInfo, &allocInfo, &vkImage, &vmaAllocation, nullptr) != VK_SUCCESS)
        return {};

    const auto vulkanTextureHandle = m_textures.emplace(VulkanTexture(
            vkImage,
            vmaAllocation,
            options.format,
            options.usage,
            this,
            deviceHandle));
    return vulkanTextureHandle;
}

void VulkanResourceManager::deleteTexture(Handle<Texture_t> handle)
{
}

Handle<TextureView_t> VulkanResourceManager::createTextureView(const Handle<Device_t> &deviceHandle,
                                                               const Handle<Texture_t> &textureHandle,
                                                               const TextureViewOptions &options)
{
    VulkanDevice vulkanDevice = *m_devices.get(deviceHandle);
    VulkanTexture vulkanTexture = *m_textures.get(textureHandle);

    VkImageViewCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = vulkanTexture.image;
    createInfo.viewType = viewTypeToVkImageViewType(options.viewType);

    // Specify the format. If none specified, default to the source texture's format
    if (options.format == Format::UNDEFINED) {
        createInfo.format = formatToVkFormat(vulkanTexture.format);
    } else {
        createInfo.format = formatToVkFormat(options.format);
    }

    // Specify which subset of the texture the view exposes
    createInfo.subresourceRange = {
        .aspectMask = options.range.aspectMask,
        .baseMipLevel = options.range.baseMipLevel,
        .levelCount = options.range.levelCount,
        .baseArrayLayer = options.range.baseArrayLayer,
        .layerCount = options.range.layerCount
    };

    // If no aspect is set, default to Color or Depth depending upon the texture usage
    if (options.range.aspectMask == static_cast<uint32_t>(TextureAspectFlagBits::None)) {
        if (vulkanTexture.usage & static_cast<uint32_t>(TextureUsageFlagBits::DepthStencilAttachmentBit))
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if (vulkanTexture.usage & static_cast<uint32_t>(TextureUsageFlagBits::ColorAttachmentBit))
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    VkImageView imageView;
    if (vkCreateImageView(vulkanDevice.device, &createInfo, nullptr, &imageView) != VK_SUCCESS)
        return {};

    const auto vulkanTextureViewHandle = m_textureViews.emplace(VulkanTextureView(imageView, textureHandle));
    return vulkanTextureViewHandle;
}

void VulkanResourceManager::deleteTextureView(Handle<TextureView_t> handle)
{
    // TODO: Implement me!
}

Handle<Buffer_t> VulkanResourceManager::createBuffer(const Handle<Device_t> deviceHandle, const BufferOptions &options, void *initialData)
{
    VulkanDevice vulkanDevice = *m_devices.get(deviceHandle);

    VkBufferCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.size = options.size;
    createInfo.usage = options.usage;
    createInfo.sharingMode = sharingModeToVkSharingMode(options.sharingMode);
    if (!options.queueTypeIndices.empty()) {
        createInfo.queueFamilyIndexCount = options.queueTypeIndices.size();
        createInfo.pQueueFamilyIndices = options.queueTypeIndices.data();
    }

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = memoryUsageToVmaMemoryUsage(options.memoryUsage);

    VkBuffer vkBuffer;
    VmaAllocation vmaAllocation;
    if (vmaCreateBuffer(vulkanDevice.allocator, &createInfo, &allocInfo, &vkBuffer, &vmaAllocation, nullptr) != VK_SUCCESS)
        return {};

    const auto vulkanBufferHandle = m_buffers.emplace(VulkanBuffer(vkBuffer, vmaAllocation, this, deviceHandle));
    return vulkanBufferHandle;
}

void VulkanResourceManager::deleteBuffer(Handle<Buffer_t> handle)
{
    // TODO: Implement me!
}

Handle<ShaderModule_t> VulkanResourceManager::createShaderModule(const Handle<Device_t> deviceHandle, const std::vector<uint32_t> &code)
{
    VulkanDevice vulkanDevice = *m_devices.get(deviceHandle);

    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size() * sizeof(uint32_t);
    createInfo.pCode = code.data();

    VkShaderModule vkShaderModule;
    if (vkCreateShaderModule(vulkanDevice.device, &createInfo, nullptr, &vkShaderModule) != VK_SUCCESS)
        return {};

    const auto vulkanShaderModuleHandle = m_shaderModules.emplace(vkShaderModule, this, deviceHandle);
    return vulkanShaderModuleHandle;
}

void VulkanResourceManager::deleteShaderModule(Handle<ShaderModule_t> handle)
{
    // TODO: Implement me!
}

Handle<PipelineLayout_t> VulkanResourceManager::createPipelineLayout(const Handle<Device_t> &deviceHandle, const PipelineLayoutOptions &options)
{
    VulkanDevice vulkanDevice = *m_devices.get(deviceHandle);

    assert(options.bindGroupLayouts.size() <= std::numeric_limits<uint32_t>::max());
    const uint32_t bindGroupLayoutCount = static_cast<uint32_t>(options.bindGroupLayouts.size());
    std::vector<VkDescriptorSetLayout> vkDescriptorSetLayouts;
    vkDescriptorSetLayouts.reserve(bindGroupLayoutCount);

    for (uint32_t i = 0; i < bindGroupLayoutCount; ++i) {
        const auto &bindGroupLayout = options.bindGroupLayouts.at(i);

        assert(bindGroupLayout.bindings.size() <= std::numeric_limits<uint32_t>::max());
        const uint32_t bindingLayoutCount = static_cast<uint32_t>(bindGroupLayout.bindings.size());
        std::vector<VkDescriptorSetLayoutBinding> vkBindingLayouts;
        vkBindingLayouts.reserve(bindingLayoutCount);

        for (uint32_t j = 0; j < bindingLayoutCount; ++j) {
            const auto &bindingLayout = bindGroupLayout.bindings.at(j);

            VkDescriptorSetLayoutBinding vkBindingLayout = {};
            vkBindingLayout.binding = bindingLayout.binding;
            vkBindingLayout.descriptorCount = bindingLayout.count;
            vkBindingLayout.descriptorType = resourceBindingTypeToVkDescriptorType(bindingLayout.resourceType);
            vkBindingLayout.stageFlags = bindingLayout.shaderStages;
            vkBindingLayout.pImmutableSamplers = nullptr; // TODO: Expose immutable samplers?

            vkBindingLayouts.emplace_back(std::move(vkBindingLayout));
        }

        // Associate the bindings into a descriptor set layout
        VkDescriptorSetLayoutCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        createInfo.bindingCount = static_cast<uint32_t>(vkBindingLayouts.size());
        createInfo.pBindings = vkBindingLayouts.data();

        VkDescriptorSetLayout vkDescriptorSetLayout{ VK_NULL_HANDLE };
        if (vkCreateDescriptorSetLayout(vulkanDevice.device, &createInfo, nullptr, &vkDescriptorSetLayout) != VK_SUCCESS) {
            // TODO: Log problem
            return {};
        }
        vkDescriptorSetLayouts.emplace_back(vkDescriptorSetLayout);
    }

    // Create the pipeline layout
    assert(options.pushConstantRanges.size() <= std::numeric_limits<uint32_t>::max());
    const uint32_t pushConstantRangeCount = options.pushConstantRanges.size();
    std::vector<VkPushConstantRange> vkPushConstantRanges;
    vkPushConstantRanges.reserve(pushConstantRangeCount);

    for (uint32_t i = 0; i < pushConstantRangeCount; ++i) {
        const auto &pushConstantRange = options.pushConstantRanges.at(i);

        VkPushConstantRange vkPushConstantRange = {
            .stageFlags = pushConstantRange.shaderStages,
            .offset = pushConstantRange.offset,
            .size = pushConstantRange.size
        };

        vkPushConstantRanges.emplace_back(std::move(vkPushConstantRange));
    }

    VkPipelineLayoutCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    createInfo.setLayoutCount = static_cast<uint32_t>(vkDescriptorSetLayouts.size());
    createInfo.pSetLayouts = vkDescriptorSetLayouts.data();
    createInfo.pushConstantRangeCount = static_cast<uint32_t>(vkPushConstantRanges.size());
    createInfo.pPushConstantRanges = vkPushConstantRanges.data();

    VkPipelineLayout vkPipelineLayout{ VK_NULL_HANDLE };
    if (vkCreatePipelineLayout(vulkanDevice.device, &createInfo, nullptr, &vkPipelineLayout) != VK_SUCCESS) {
        // TODO: Log problem
        return {};
    }

    // Store the results
    const auto vulkanPipelineLayoutHandle = m_pipelineLayouts.emplace(VulkanPipelineLayout(
            vkPipelineLayout,
            std::move(vkDescriptorSetLayouts),
            this,
            deviceHandle));

    return vulkanPipelineLayoutHandle;
}

void VulkanResourceManager::deletePipelineLayout(Handle<PipelineLayout_t> handle)
{
    // TODO: Implement me!
}

Handle<GraphicsPipeline_t> VulkanResourceManager::createGraphicsPipeline(const Handle<Device_t> &deviceHandle, const GraphicsPipelineOptions &options)
{
    VulkanDevice vulkanDevice = *m_devices.get(deviceHandle);

    // Shader stages
    std::vector<VkPipelineShaderStageCreateInfo> shaderInfos;
    const uint32_t shaderCount = static_cast<uint32_t>(options.shaderStages.size());
    shaderInfos.reserve(shaderCount);
    for (uint32_t i = 0; i < shaderCount; ++i) {
        const auto &shaderStage = options.shaderStages.at(i);

        VkPipelineShaderStageCreateInfo shaderInfo = {};
        shaderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderInfo.stage = shaderStageFlagBitsToVkShaderStageFlagBits(shaderStage.stage);

        // Lookup the shader module
        const auto vulkanShaderModule = getShaderModule(shaderStage.shaderModule);
        if (!vulkanShaderModule)
            return {};
        shaderInfo.module = vulkanShaderModule->shaderModule;
        shaderInfo.pName = shaderStage.entryPoint.data();

        shaderInfos.emplace_back(shaderInfo);
    }

    // Vertex input
    const uint32_t vertexBindingCount = static_cast<uint32_t>(options.vertex.buffers.size());
    std::vector<VkVertexInputBindingDescription> vertexBindings;
    vertexBindings.reserve(vertexBindingCount);
    for (uint32_t i = 0; i < vertexBindingCount; ++i) {
        const auto &binding = options.vertex.buffers.at(i);
        VkVertexInputBindingDescription vkBinding = {};
        vkBinding.binding = binding.binding;
        vkBinding.stride = binding.stride;
        vkBinding.inputRate = vertexRateToVkVertexInputRate(binding.inputRate);
        vertexBindings.emplace_back(vkBinding);
    }

    const uint32_t attributeCount = static_cast<uint32_t>(options.vertex.attributes.size());
    std::vector<VkVertexInputAttributeDescription> attributes;
    attributes.reserve(attributeCount);
    for (uint32_t i = 0; i < attributeCount; ++i) {
        const auto &attribute = options.vertex.attributes.at(i);
        VkVertexInputAttributeDescription vkAttribute = {};
        vkAttribute.location = attribute.location;
        vkAttribute.binding = attribute.binding;
        vkAttribute.format = formatToVkFormat(attribute.format);
        vkAttribute.offset = attribute.offset;
        attributes.emplace_back(vkAttribute);
    }

    VkPipelineVertexInputStateCreateInfo vertexInputState = {};
    vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexBindings.size());
    vertexInputState.pVertexBindingDescriptions = vertexBindings.data();
    vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributes.size());
    vertexInputState.pVertexAttributeDescriptions = attributes.data();

    // Input assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = primitiveTopologyToVkPrimitiveTopology(options.primitive.topology);
    inputAssembly.primitiveRestartEnable = options.primitive.primitiveRestart;

    // Tessellation
    VkPipelineTessellationStateCreateInfo tessellationStateInfo = {};
    tessellationStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
    tessellationStateInfo.flags = 0;
    tessellationStateInfo.patchControlPoints = options.primitive.patchControlPoints;

    // Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = polygonModeToVkPolygonMode(options.primitive.polygonMode);
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = options.primitive.cullMode;
    rasterizer.frontFace = frontFaceToVkFrontFace(options.primitive.frontFace);
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;

    // Multisampling
    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = options.multisample.samples > SampleCountFlagBits::Samples1Bit;
    multisampling.rasterizationSamples = sampleCountFlagBitsToVkSampleFlagBits(options.multisample.samples);
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = options.multisample.sampleMasks.data();
    multisampling.alphaToCoverageEnable = options.multisample.alphaToCoverageEnabled;
    multisampling.alphaToOneEnable = VK_FALSE;

    // Depth and stencil testing
    auto vkStencilOpStateFromStencilOperationOptions = [](const StencilOperationOptions &options) {
        VkStencilOpState stencilOp = {};
        stencilOp.failOp = stencilOperationToVkStencilOp(options.failOp);
        stencilOp.passOp = stencilOperationToVkStencilOp(options.passOp);
        stencilOp.depthFailOp = stencilOperationToVkStencilOp(options.depthFailOp);
        stencilOp.compareOp = compareOperationToVkCompareOp(options.compareOp);
        stencilOp.compareMask = options.compareMask;
        stencilOp.writeMask = options.writeMask;
        stencilOp.reference = options.reference;
        return stencilOp;
    };

    VkPipelineDepthStencilStateCreateInfo depthStencil = {};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = options.depthStencil.depthTestEnabled;
    depthStencil.depthWriteEnable = options.depthStencil.depthWritesEnabled;
    depthStencil.depthCompareOp = compareOperationToVkCompareOp(options.depthStencil.depthCompareOperation);
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f;
    depthStencil.maxDepthBounds = 1.0f;
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = vkStencilOpStateFromStencilOperationOptions(options.depthStencil.stencilFront);
    depthStencil.back = vkStencilOpStateFromStencilOperationOptions(options.depthStencil.stencilBack);

    // Blending
    const uint32_t attachmentCount = options.renderTargets.size();
    std::vector<VkPipelineColorBlendAttachmentState> attachmentBlends;
    attachmentBlends.reserve(attachmentCount);
    for (uint32_t i = 0; i < attachmentCount; ++i) {
        const auto &renderTarget = options.renderTargets.at(i);

        VkPipelineColorBlendAttachmentState vkAttachmentBlend = {};
        vkAttachmentBlend.colorWriteMask = renderTarget.writeMask;
        vkAttachmentBlend.blendEnable = renderTarget.blending.blendingEnabled;
        vkAttachmentBlend.srcColorBlendFactor = blendFactorToVkBlendFactor(renderTarget.blending.color.srcFactor);
        vkAttachmentBlend.dstColorBlendFactor = blendFactorToVkBlendFactor(renderTarget.blending.color.dstFactor);
        vkAttachmentBlend.colorBlendOp = blendOperationToVkBlendOp(renderTarget.blending.color.operation);
        vkAttachmentBlend.srcAlphaBlendFactor = blendFactorToVkBlendFactor(renderTarget.blending.alpha.srcFactor);
        vkAttachmentBlend.dstAlphaBlendFactor = blendFactorToVkBlendFactor(renderTarget.blending.alpha.dstFactor);
        vkAttachmentBlend.alphaBlendOp = blendOperationToVkBlendOp(renderTarget.blending.alpha.operation);

        attachmentBlends.emplace_back(vkAttachmentBlend);
    }

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = static_cast<uint32_t>(attachmentBlends.size());
    colorBlending.pAttachments = attachmentBlends.data();
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    // Dynamic pipeline state. This is state that can be overridden whilst recording
    // command buffers with commands such as vkCmdSetViewport or vkCmdSetScissor. We
    // always make the viewport and scissor states dynamic and require clients to
    // set these when recording.
    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
    dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicStateInfo.pDynamicStates = dynamicStates.data();
    dynamicStateInfo.flags = 0;

    // We do still need to specify the number of viewports (and scissor rects) though
    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = nullptr; // Provided by dynamic state
    viewportState.scissorCount = 1;
    viewportState.pScissors = nullptr; // Provided by dynamic state

    // Fetch the specified pipeline layout
    VulkanPipelineLayout *vulkanPipelineLayout = getPipelineLayout(options.layout);
    if (!vulkanPipelineLayout) {
        // TODO: Log invalid pipeline layout requested
        return {};
    }

    // TODO: Investigate using VK_KHR_dynamic_rendering (core in Vulkan 1.3).
    //       Do the other graphics APIs have an equivalent or perhaps they default
    //       to that sort of model? We also need this to be supported across all
    //       Vulkan target platforms (desktop, pi, android, imx8).
    //
    // Create a render pass that serves to specify the layout / compatibility of
    // concrete render passes and framebuffers used to perform rendering with this
    // pipeline at command record time. We only do this if the pipeline outputs to
    // render targets.
    VkRenderPass vkRenderPass = VK_NULL_HANDLE;
    if (!options.renderTargets.empty()) {
        // Specify attachment refs for all color and resolve render targets and any
        // depth-stencil target. Concrete render passes that want to use this pipeline
        // to render, must begin a render pass that is compatible with this render pass.
        // See the detailed description of render pass compatibility at:
        //
        // https://registry.khronos.org/vulkan/specs/1.3-extensions/html/vkspec.html#renderpass-compatibility
        //
        // But in short, the concrete render pass must match attachment counts of each
        // type and match the formats and sample counts in each case.

        // The render pass create info needs to know about all of the attachments that
        // could be used in all of the subpasses (just 1 here). We will populate this
        // with all of the color attachments, resolve attachments (if using MSAA), and
        // finally a depth-stencil attachment, if specified.
        //
        // We do not concern ourselves with subpass dependencies here as they do not
        // impact upon render pass compatibility (I think/hope).
        std::vector<VkAttachmentDescription> allAttachments;

        // The subpass description will then index into the above vector of attachment
        // descriptions to specify which subpasses use which of the available attachments.
        uint32_t attachmentIndex = 0;
        std::vector<VkAttachmentReference> colorAttachmentRefs;
        std::vector<VkAttachmentReference> resolveAttachmentRefs;
        VkAttachmentReference depthStencilAttachmentRef = {};

        const bool usingMultisampling = options.multisample.samples > SampleCountFlagBits::Samples1Bit;
        const VkSampleCountFlagBits sampleCount = sampleCountFlagBitsToVkSampleFlagBits(options.multisample.samples);

        // Color and resolve attachments
        {
            const uint32_t colorTargetsCount = options.renderTargets.size();
            colorAttachmentRefs.reserve(colorTargetsCount);
            resolveAttachmentRefs.reserve(colorTargetsCount);

            for (uint32_t i = 0; i < colorTargetsCount; ++i) {
                const auto &renderTarget = options.renderTargets.at(i);

                // NB: We don't care about load/store operations and initial/final layouts here
                // so we just set some random values;
                VkAttachmentDescription colorAttachment = {};
                colorAttachment.format = formatToVkFormat(renderTarget.format);
                colorAttachment.samples = sampleCount;
                colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                colorAttachment.finalLayout = usingMultisampling ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                allAttachments.emplace_back(colorAttachment);

                VkAttachmentReference colorAttachmentRef = {};
                colorAttachmentRef.attachment = attachmentIndex++;
                colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                colorAttachmentRefs.emplace_back(colorAttachmentRef);

                // If using multisampling, then for each color attachment we need a resolve attachment
                if (usingMultisampling) {
                    VkAttachmentDescription resolveAttachment = {};
                    resolveAttachment.format = formatToVkFormat(renderTarget.format);
                    resolveAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
                    resolveAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                    resolveAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                    resolveAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                    resolveAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                    resolveAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                    resolveAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                    allAttachments.emplace_back(resolveAttachment);

                    VkAttachmentReference resolveAttachmentRef = {};
                    resolveAttachmentRef.attachment = attachmentIndex++;
                    resolveAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                    colorAttachmentRefs.emplace_back(resolveAttachmentRef);
                }
            }
        }

        // Depth-stencil attachment
        if (options.depthStencil.format != Format::UNDEFINED) {
            VkAttachmentDescription depthStencilAttachment = {};
            depthStencilAttachment.format = formatToVkFormat(options.depthStencil.format);
            depthStencilAttachment.samples = sampleCount;
            depthStencilAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            depthStencilAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            depthStencilAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            depthStencilAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthStencilAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            depthStencilAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            allAttachments.emplace_back(depthStencilAttachment);

            depthStencilAttachmentRef.attachment = attachmentIndex++;
            depthStencilAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }

        // Just create a single subpass. We do not support multiple subpasses at this
        // stage as other graphics APIs do not have an equivalent to subpasses.
        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentRefs.size());
        subpass.pColorAttachments = colorAttachmentRefs.data();
        subpass.pResolveAttachments = usingMultisampling ? resolveAttachmentRefs.data() : nullptr;
        subpass.pDepthStencilAttachment = options.depthStencil.format != Format::UNDEFINED ? &depthStencilAttachmentRef : nullptr;

        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(allAttachments.size());
        renderPassInfo.pAttachments = allAttachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 0;
        renderPassInfo.pDependencies = nullptr;

        if (vkCreateRenderPass(vulkanDevice.device, &renderPassInfo, nullptr, &vkRenderPass) != VK_SUCCESS) {
            // TODO: Log failure to create a render pass
            return {};
        }
    }

    // Bring it all together in the all-knowing pipeline create info
    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;
    pipelineInfo.stageCount = static_cast<uint32_t>(shaderInfos.size());
    pipelineInfo.pStages = shaderInfos.data();
    pipelineInfo.pVertexInputState = &vertexInputState;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pTessellationState = &tessellationStateInfo;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicStateInfo;
    pipelineInfo.layout = vulkanPipelineLayout->pipelineLayout;
    pipelineInfo.renderPass = vkRenderPass;
    pipelineInfo.subpass = 0;

    VkPipeline vkPipeline{ VK_NULL_HANDLE };
    if (vkCreateGraphicsPipelines(vulkanDevice.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &vkPipeline) != VK_SUCCESS) {
        // TODO: Log failure to create a pipeline
        return {};
    }

    // Create VulkanPipeline object and return handle
    const auto vulkanGraphicsPipelineHandle = m_graphicsPipelines.emplace(VulkanGraphicsPipeline(
            vkPipeline,
            vkRenderPass,
            this,
            deviceHandle));

    return vulkanGraphicsPipelineHandle;
}

void VulkanResourceManager::deleteGraphicsPipeline(Handle<GraphicsPipeline_t> handle)
{
    // TODO: Implement me!
}

Handle<GpuSemaphore_t> VulkanResourceManager::createGpuSemaphore(const Handle<Device_t> &deviceHandle, const GpuSemaphoreOptions &options)
{
    VulkanDevice vulkanDevice = *m_devices.get(deviceHandle);

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkSemaphore vkSemaphore{ VK_NULL_HANDLE };
    if (vkCreateSemaphore(vulkanDevice.device, &semaphoreInfo, nullptr, &vkSemaphore) != VK_SUCCESS) {
        // TODO: Log failure to create a pipeline
        return {};
    }

    const auto vulkanGpuSemaphoreHandle = m_gpuSemaphores.emplace(VulkanGpuSemaphore(
            vkSemaphore,
            this,
            deviceHandle));

    return vulkanGpuSemaphoreHandle;
}

void VulkanResourceManager::deleteGpuSemaphore(Handle<GpuSemaphore_t> handle)
{
    // TODO: Implement me!
}

Handle<CommandRecorder_t> VulkanResourceManager::createCommandRecorder(const Handle<Device_t> &deviceHandle, const CommandRecorderOptions &options)
{
    VulkanDevice *vulkanDevice = m_devices.get(deviceHandle);

    // Which queue is the command recorder requested for?
    Handle<Queue_t> queueHandle;
    uint32_t queueTypeIndex = std::numeric_limits<uint32_t>::max();
    if (!options.queue.isValid()) {
        if (vulkanDevice->queueDescriptions.empty()) {
            // TODO: Log that we have no queue descriptions on the device
            return {};
        }

        queueHandle = vulkanDevice->queueDescriptions[0].queue;
        queueTypeIndex = vulkanDevice->queueDescriptions[0].queueTypeIndex;
    } else {
        // Look for this queue on the device
        const auto it = std::find_if(
                vulkanDevice->queueDescriptions.begin(),
                vulkanDevice->queueDescriptions.end(),
                [options](const QueueDescription &queueDescription) { return queueDescription.queue == options.queue; });
        if (it == vulkanDevice->queueDescriptions.end()) {
            // TODO: Log that we can't find the requested queue on this device
            return {};
        }

        queueHandle = it->queue;
        queueTypeIndex = it->queueTypeIndex;
    }
    assert(queueHandle.isValid());
    assert(queueTypeIndex != std::numeric_limits<uint32_t>::max());

    // Find or create a command pool for this combination of thread and queue family
    if (vulkanDevice->commandPools[queueTypeIndex] == VK_NULL_HANDLE) {
        // No command pool exists yet for this queue family, let's create one why not!
        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = queueTypeIndex;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        VkCommandPool vkCommandPool = VK_NULL_HANDLE;
        if (vkCreateCommandPool(vulkanDevice->device, &poolInfo, nullptr, &vkCommandPool) != VK_SUCCESS) {
            // TODO: Log that we failed to create a command pool for this queue family
            return {};
        }
        vulkanDevice->commandPools[queueTypeIndex] = vkCommandPool;
    }
    VkCommandPool vkCommandPool = vulkanDevice->commandPools[queueTypeIndex];

    // Allocate a command buffer object from the pool
    // TODO: Support secondary command buffers? Is that a thing outside of Vulkan? Do we care?
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = vkCommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1U;

    VkCommandBuffer vkCommandBuffer{ VK_NULL_HANDLE };
    if (vkAllocateCommandBuffers(vulkanDevice->device, &allocInfo, &vkCommandBuffer) != VK_SUCCESS) {
        // TODO: Log failure to allocate a command buffer
        return {};
    }

    // Begin recording
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(vkCommandBuffer, &beginInfo) != VK_SUCCESS) {
        // TODO: Log failure to begin recording
        return {};
    }

    const auto vulkanCommandBufferHandle = m_commandBuffers.emplace(VulkanCommandBuffer(vkCommandBuffer));

    // Finally, we can create the command recorder object
    const auto vulkanCommandRecorderHandle = m_commandRecorders.emplace(VulkanCommandRecorder(
            vkCommandPool,
            vkCommandBuffer,
            vulkanCommandBufferHandle,
            this,
            deviceHandle));

    return vulkanCommandRecorderHandle;
}

void VulkanResourceManager::deleteCommandRecorder(Handle<CommandRecorder_t> handle)
{
    // TODO: Implement me!
}

Handle<RenderPassCommandRecorder_t> VulkanResourceManager::createRenderPassCommandRecorder(const Handle<Device_t> &deviceHandle,
                                                                                           const Handle<CommandRecorder_t> commandRecorderHandle,
                                                                                           const RenderPassCommandRecorderOptions &options)
{
    VulkanDevice *vulkanDevice = m_devices.get(deviceHandle);

    // TODO: Should we make RenderPass and Framebuffer objects explicitly available to the API?
    // Doing so would make our API more Vulkan-like and perhaps give a tiny performance boost. On the downside
    // it's more API surface area (like Vulkan) and other backends would need to store the render pass and
    // framebuffer requirements to look them up later when using whatever wrapper they have around render passes.
    // E.g in a WebGPU backend, the render pass backend would just store the options, ready to pass to beginRenderPass().
    // For now we take a similar approach to WebGPU or the Vulkan dynamic rendering extension.

    // Find or create a render pass object that matches the request
    const VulkanRenderPassKey renderPassKey{ options };
    auto it = vulkanDevice->renderPasses.find(renderPassKey);
    Handle<RenderPass_t> vulkanRenderPassHandle;
    if (it == vulkanDevice->renderPasses.end()) {
        // TODO: Create the render pass and cache the handle for it
        vulkanRenderPassHandle = createRenderPass(deviceHandle, options);
        vulkanDevice->renderPasses.insert({ renderPassKey, vulkanRenderPassHandle });
    } else {
        vulkanRenderPassHandle = it->second;
    }

    VulkanRenderPass *vulkanRenderPass = m_renderPasses.get(vulkanRenderPassHandle);
    if (!vulkanRenderPass) {
        // TODO: Log about not finding/creating a render pass
        return {};
    }
    VkRenderPass vkRenderPass = vulkanRenderPass->renderPass;

    // TODO: Find or create a framebuffer as per the render pass above

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = vkRenderPass;

    // TODO:
    // renderPassInfo.framebuffer = vkFramebuffer;
    // render area
    // clear values

    VulkanCommandRecorder *vulkanCommandRecorder = m_commandRecorders.get(commandRecorderHandle);
    if (!vulkanCommandRecorder) {
        // TODO: Log about not having a valid command recorder
        return {};
    }
    VkCommandBuffer vkCommandBuffer = vulkanCommandRecorder->commandBuffer;

    vkCmdBeginRenderPass(vkCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    return {};
}

void VulkanResourceManager::deleteRenderPassCommandRecorder(Handle<RenderPassCommandRecorder_t> handle)
{
    // TODO: Implement me!
}

Handle<RenderPass_t> VulkanResourceManager::createRenderPass(const Handle<Device_t> &deviceHandle,
                                                             const RenderPassCommandRecorderOptions &options)
{
    VulkanDevice *vulkanDevice = m_devices.get(deviceHandle);
    std::vector<VkAttachmentDescription> allAttachments;

    // The subpass description will then index into the above vector of attachment
    // descriptions to specify which subpasses use which of the available attachments.
    uint32_t attachmentIndex = 0;
    std::vector<VkAttachmentReference> colorAttachmentRefs;
    std::vector<VkAttachmentReference> resolveAttachmentRefs;
    VkAttachmentReference depthStencilAttachmentRef = {};

    // TODO: Handle multisampling
    const bool usingMultisampling = false; // options.multisample.samples > SampleCountFlagBits::Samples1Bit;
    const VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT; // sampleCountFlagBitsToVkSampleFlagBits(options.multisample.samples);

    // Color and resolve attachments
    {
        const uint32_t colorTargetsCount = options.colorAttachments.size();
        colorAttachmentRefs.reserve(colorTargetsCount);
        resolveAttachmentRefs.reserve(colorTargetsCount);

        for (uint32_t i = 0; i < colorTargetsCount; ++i) {
            const auto &renderTarget = options.colorAttachments.at(i);

            VulkanTextureView *view = getTextureView(renderTarget.view);
            if (!view) {
                // Log invalid view requested
                return {};
            }
            VulkanTexture *texture = getTexture(view->textureHandle);
            if (!texture) {
                // Log invalid texture requested
                return {};
            }

            // NB: We don't care about load/store operations and initial/final layouts here
            // so we just set some random values;
            VkAttachmentDescription colorAttachment = {};
            colorAttachment.format = formatToVkFormat(texture->format);
            colorAttachment.samples = sampleCount;
            colorAttachment.loadOp = attachmentLoadOperationToVkAttachmentLoadOp(renderTarget.loadOperation);
            colorAttachment.storeOp = attachmentStoreOperationToVkAttachmentStoreOp(renderTarget.storeOperation);
            colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            colorAttachment.initialLayout = textureLayoutToVkImageLayout(renderTarget.initialLayout);
            colorAttachment.finalLayout = textureLayoutToVkImageLayout(renderTarget.finalLayout);
            // colorAttachment.finalLayout = usingMultisampling ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            allAttachments.emplace_back(colorAttachment);

            VkAttachmentReference colorAttachmentRef = {};
            colorAttachmentRef.attachment = attachmentIndex++;
            colorAttachmentRef.layout = textureLayoutToVkImageLayout(renderTarget.initialLayout);
            colorAttachmentRefs.emplace_back(colorAttachmentRef);

            // If using multisampling, then for each color attachment we need a resolve attachment
            // if (usingMultisampling) {
            //     VkAttachmentDescription resolveAttachment = {};
            //     resolveAttachment.format = formatToVkFormat(renderTarget.format);
            //     resolveAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            //     resolveAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            //     resolveAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            //     resolveAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            //     resolveAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            //     resolveAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            //     resolveAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            //     allAttachments.emplace_back(resolveAttachment);

            //     VkAttachmentReference resolveAttachmentRef = {};
            //     resolveAttachmentRef.attachment = attachmentIndex++;
            //     resolveAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            //     colorAttachmentRefs.emplace_back(resolveAttachmentRef);
            // }
        }
    }

    // Depth-stencil attachment
    if (options.depthStencilAttachment.view.isValid()) {
        const auto &renderTarget = options.depthStencilAttachment;

        VulkanTextureView *view = getTextureView(renderTarget.view);
        if (!view) {
            // Log invalid view requested
            return {};
        }
        VulkanTexture *texture = getTexture(view->textureHandle);
        if (!texture) {
            // Log invalid texture requested
            return {};
        }

        VkAttachmentDescription depthStencilAttachment = {};
        depthStencilAttachment.format = formatToVkFormat(texture->format);
        depthStencilAttachment.samples = sampleCount;
        depthStencilAttachment.loadOp = attachmentLoadOperationToVkAttachmentLoadOp(renderTarget.depthLoadOperation);
        depthStencilAttachment.storeOp = attachmentStoreOperationToVkAttachmentStoreOp(renderTarget.depthStoreOperation);
        depthStencilAttachment.stencilLoadOp = attachmentLoadOperationToVkAttachmentLoadOp(renderTarget.stencilLoadOperation);
        depthStencilAttachment.stencilStoreOp = attachmentStoreOperationToVkAttachmentStoreOp(renderTarget.stencilStoreOperation);
        depthStencilAttachment.initialLayout = textureLayoutToVkImageLayout(renderTarget.initialLayout);
        depthStencilAttachment.finalLayout = textureLayoutToVkImageLayout(renderTarget.finalLayout);
        allAttachments.emplace_back(depthStencilAttachment);

        depthStencilAttachmentRef.attachment = attachmentIndex++;
        depthStencilAttachmentRef.layout = textureLayoutToVkImageLayout(renderTarget.initialLayout);
    }

    // Just create a single subpass. We do not support multiple subpasses at this
    // stage as other graphics APIs do not have an equivalent to subpasses.
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentRefs.size());
    subpass.pColorAttachments = colorAttachmentRefs.data();
    subpass.pResolveAttachments = usingMultisampling ? resolveAttachmentRefs.data() : nullptr;
    subpass.pDepthStencilAttachment = options.depthStencilAttachment.view.isValid() ? &depthStencilAttachmentRef : nullptr;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(allAttachments.size());
    renderPassInfo.pAttachments = allAttachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 0;
    renderPassInfo.pDependencies = nullptr;

    VkRenderPass vkRenderPass{ VK_NULL_HANDLE };
    if (vkCreateRenderPass(vulkanDevice->device, &renderPassInfo, nullptr, &vkRenderPass) != VK_SUCCESS) {
        // TODO: Log failure to create a render pass
        return {};
    }

    const auto vulkanRenderPassHandle = m_renderPasses.emplace(VulkanRenderPass(vkRenderPass, this, deviceHandle));
    return vulkanRenderPassHandle;
}

Handle<BindGroup> VulkanResourceManager::createBindGroup(BindGroupDescription desc)
{
    // TODO: This is where we will call vkAllocateDescriptorSets
    return {};
}

void VulkanResourceManager::deleteBindGroup(Handle<BindGroup> handle)
{
    // TODO: Implement me!
}

} // namespace ToyRenderer
