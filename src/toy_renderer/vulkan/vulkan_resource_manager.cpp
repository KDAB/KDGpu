#include "vulkan_resource_manager.h"

#include <toy_renderer/buffer_options.h>
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

    const auto swapchainHandle = m_swapchains.emplace(VulkanSwapchain{ vkSwapchain, vulkanDevice.device, this });
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

    VkImage vkImage;
    if (vkCreateImage(vulkanDevice.device, &createInfo, nullptr, &vkImage) != VK_SUCCESS)
        return {};

    const auto vulkanTextureHandle = m_textures.emplace(VulkanTexture(vkImage, vulkanDevice.device, this));
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
    createInfo.format = formatToVkFormat(options.format); // TODO: Should we default this to match the texture format?
    createInfo.subresourceRange = {
        .aspectMask = options.range.aspectMask,
        .baseMipLevel = options.range.baseMipLevel,
        .levelCount = options.range.levelCount,
        .baseArrayLayer = options.range.baseArrayLayer,
        .layerCount = options.range.layerCount
    };

    VkImageView imageView;
    if (vkCreateImageView(vulkanDevice.device, &createInfo, nullptr, &imageView) != VK_SUCCESS)
        return {};

    // TODO: Allocate memory and bind to image

    const auto vulkanTextureViewHandle = m_textureViews.emplace(VulkanTextureView(imageView));
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
    return {};
}

void VulkanResourceManager::deletePipelineLayout(Handle<PipelineLayout_t> handle)
{
    // TODO: Implement me!
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
