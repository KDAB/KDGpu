#include "vulkan_graphics_api.h"

#include <toy_renderer/vulkan/vulkan_enums.h>

namespace ToyRenderer {

VulkanGraphicsApi::VulkanGraphicsApi()
    : GraphicsApi()
    , m_vulkanResourceManager{ std::make_unique<VulkanResourceManager>() }
{
    m_resourceManager = m_vulkanResourceManager.get();
}

VulkanGraphicsApi::~VulkanGraphicsApi()
{
}

std::vector<Handle<Adapter_t>> VulkanGraphicsApi::queryAdapters(const Handle<Instance_t> &instanceHandle)
{
    // Query the physical devices from the instance
    VkInstance vkInstance = *m_vulkanResourceManager->getInstance(instanceHandle);

    uint32_t adapterCount = 0;
    vkEnumeratePhysicalDevices(vkInstance, &adapterCount, nullptr);
    std::vector<VkPhysicalDevice> physicalDevices(adapterCount);
    vkEnumeratePhysicalDevices(vkInstance, &adapterCount, physicalDevices.data());

    // Store the resulting physical devices in the resource manager so that
    // the Adapters can access them later, and create the Adapters.
    std::vector<Handle<Adapter_t>> adapterHandles;
    adapterHandles.reserve(adapterCount);
    for (uint32_t adapterIndex = 0; adapterIndex < adapterCount; ++adapterIndex) {
        VulkanAdapter vulkanAdapter{ physicalDevices[adapterIndex] };
        adapterHandles.emplace_back(m_vulkanResourceManager->insertAdapter(vulkanAdapter));
    }

    return adapterHandles;
}

AdapterFeatures VulkanGraphicsApi::queryAdapterFeatures(const Handle<Adapter_t> &adapterHandle)
{
    VulkanAdapter vulkanAdapter = *dynamic_cast<VulkanAdapter *>(m_vulkanResourceManager->getAdapter(adapterHandle));
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(vulkanAdapter.physicalDevice, &deviceFeatures);

    AdapterFeatures features = {
        .robustBufferAccess = static_cast<bool>(deviceFeatures.robustBufferAccess),
        .fullDrawIndexUint32 = static_cast<bool>(deviceFeatures.fullDrawIndexUint32),
        .imageCubeArray = static_cast<bool>(deviceFeatures.imageCubeArray),
        .independentBlend = static_cast<bool>(deviceFeatures.independentBlend),
        .geometryShader = static_cast<bool>(deviceFeatures.geometryShader),
        .tessellationShader = static_cast<bool>(deviceFeatures.tessellationShader),
        .sampleRateShading = static_cast<bool>(deviceFeatures.sampleRateShading),
        .dualSrcBlend = static_cast<bool>(deviceFeatures.dualSrcBlend),
        .logicOp = static_cast<bool>(deviceFeatures.logicOp),
        .multiDrawIndirect = static_cast<bool>(deviceFeatures.multiDrawIndirect),
        .drawIndirectFirstInstance = static_cast<bool>(deviceFeatures.drawIndirectFirstInstance),
        .depthClamp = static_cast<bool>(deviceFeatures.depthClamp),
        .depthBiasClamp = static_cast<bool>(deviceFeatures.depthBiasClamp),
        .fillModeNonSolid = static_cast<bool>(deviceFeatures.fillModeNonSolid),
        .depthBounds = static_cast<bool>(deviceFeatures.depthBounds),
        .wideLines = static_cast<bool>(deviceFeatures.wideLines),
        .largePoints = static_cast<bool>(deviceFeatures.largePoints),
        .alphaToOne = static_cast<bool>(deviceFeatures.alphaToOne),
        .multiViewport = static_cast<bool>(deviceFeatures.multiViewport),
        .samplerAnisotropy = static_cast<bool>(deviceFeatures.samplerAnisotropy),
        .textureCompressionETC2 = static_cast<bool>(deviceFeatures.textureCompressionETC2),
        .textureCompressionASTC_LDR = static_cast<bool>(deviceFeatures.textureCompressionASTC_LDR),
        .textureCompressionBC = static_cast<bool>(deviceFeatures.textureCompressionBC),
        .occlusionQueryPrecise = static_cast<bool>(deviceFeatures.occlusionQueryPrecise),
        .pipelineStatisticsQuery = static_cast<bool>(deviceFeatures.pipelineStatisticsQuery),
        .vertexPipelineStoresAndAtomics = static_cast<bool>(deviceFeatures.vertexPipelineStoresAndAtomics),
        .fragmentStoresAndAtomics = static_cast<bool>(deviceFeatures.fragmentStoresAndAtomics),
        .shaderTessellationAndGeometryPointSize = static_cast<bool>(deviceFeatures.shaderTessellationAndGeometryPointSize),
        .shaderImageGatherExtended = static_cast<bool>(deviceFeatures.shaderImageGatherExtended),
        .shaderStorageImageExtendedFormats = static_cast<bool>(deviceFeatures.shaderStorageImageExtendedFormats),
        .shaderStorageImageMultisample = static_cast<bool>(deviceFeatures.shaderStorageImageMultisample),
        .shaderStorageImageReadWithoutFormat = static_cast<bool>(deviceFeatures.shaderStorageImageReadWithoutFormat),
        .shaderStorageImageWriteWithoutFormat = static_cast<bool>(deviceFeatures.shaderStorageImageWriteWithoutFormat),
        .shaderUniformBufferArrayDynamicIndexing = static_cast<bool>(deviceFeatures.shaderUniformBufferArrayDynamicIndexing),
        .shaderSampledImageArrayDynamicIndexing = static_cast<bool>(deviceFeatures.shaderSampledImageArrayDynamicIndexing),
        .shaderStorageBufferArrayDynamicIndexing = static_cast<bool>(deviceFeatures.shaderStorageBufferArrayDynamicIndexing),
        .shaderStorageImageArrayDynamicIndexing = static_cast<bool>(deviceFeatures.shaderStorageImageArrayDynamicIndexing),
        .shaderClipDistance = static_cast<bool>(deviceFeatures.shaderClipDistance),
        .shaderCullDistance = static_cast<bool>(deviceFeatures.shaderCullDistance),
        .shaderFloat64 = static_cast<bool>(deviceFeatures.shaderFloat64),
        .shaderInt64 = static_cast<bool>(deviceFeatures.shaderInt64),
        .shaderInt16 = static_cast<bool>(deviceFeatures.shaderInt16),
        .shaderResourceResidency = static_cast<bool>(deviceFeatures.shaderResourceResidency),
        .shaderResourceMinLod = static_cast<bool>(deviceFeatures.shaderResourceMinLod),
        .sparseBinding = static_cast<bool>(deviceFeatures.sparseBinding),
        .sparseResidencyBuffer = static_cast<bool>(deviceFeatures.sparseResidencyBuffer),
        .sparseResidencyImage2D = static_cast<bool>(deviceFeatures.sparseResidencyImage2D),
        .sparseResidencyImage3D = static_cast<bool>(deviceFeatures.sparseResidencyImage3D),
        .sparseResidency2Samples = static_cast<bool>(deviceFeatures.sparseResidency2Samples),
        .sparseResidency4Samples = static_cast<bool>(deviceFeatures.sparseResidency4Samples),
        .sparseResidency8Samples = static_cast<bool>(deviceFeatures.sparseResidency8Samples),
        .sparseResidency16Samples = static_cast<bool>(deviceFeatures.sparseResidency16Samples),
        .sparseResidencyAliased = static_cast<bool>(deviceFeatures.sparseResidencyAliased),
        .variableMultisampleRate = static_cast<bool>(deviceFeatures.variableMultisampleRate),
        .inheritedQueries = static_cast<bool>(deviceFeatures.inheritedQueries)
    };
    return features;
}

} // namespace ToyRenderer
