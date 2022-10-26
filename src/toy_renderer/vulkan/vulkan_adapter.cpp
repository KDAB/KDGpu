#include "vulkan_adapter.h"

#include <toy_renderer/vulkan/vulkan_enums.h>

namespace ToyRenderer {

VulkanAdapter::VulkanAdapter(VkPhysicalDevice _physicalDevice)
    : ApiAdapter()
    , physicalDevice(_physicalDevice)
{
}

AdapterProperties VulkanAdapter::queryAdapterProperties()
{
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

    const auto &limits = deviceProperties.limits;
    const auto &sparseProperties = deviceProperties.sparseProperties;

    // clang-format off
    AdapterProperties properties = {
        .apiVersion = deviceProperties.apiVersion,
        .driverVersion = deviceProperties.driverVersion,
        .vendorID = deviceProperties.vendorID,
        .deviceID = deviceProperties.deviceID,
        .deviceType = vkPhysicalDeviceTypeToAdapterDeviceType(deviceProperties.deviceType),
        .deviceName = deviceProperties.deviceName,
        .pipelineCacheUUID = {
            deviceProperties.pipelineCacheUUID[0],
            deviceProperties.pipelineCacheUUID[1],
            deviceProperties.pipelineCacheUUID[2],
            deviceProperties.pipelineCacheUUID[3],
            deviceProperties.pipelineCacheUUID[4],
            deviceProperties.pipelineCacheUUID[5],
            deviceProperties.pipelineCacheUUID[6],
            deviceProperties.pipelineCacheUUID[7],
            deviceProperties.pipelineCacheUUID[8],
            deviceProperties.pipelineCacheUUID[9],
            deviceProperties.pipelineCacheUUID[10],
            deviceProperties.pipelineCacheUUID[11],
            deviceProperties.pipelineCacheUUID[12],
            deviceProperties.pipelineCacheUUID[13],
            deviceProperties.pipelineCacheUUID[14],
            deviceProperties.pipelineCacheUUID[15]
        },
        .limits = {
            .maxImageDimension1D = limits.maxImageDimension1D,
            .maxImageDimension2D = limits.maxImageDimension2D,
            .maxImageDimension3D = limits.maxImageDimension3D,
            .maxImageDimensionCube = limits.maxImageDimensionCube,
            .maxImageArrayLayers = limits.maxImageArrayLayers,
            .maxTexelBufferElements = limits.maxTexelBufferElements,
            .maxUniformBufferRange = limits.maxUniformBufferRange,
            .maxStorageBufferRange = limits.maxStorageBufferRange,
            .maxPushConstantsSize = limits.maxPushConstantsSize,
            .maxMemoryAllocationCount = limits.maxMemoryAllocationCount,
            .maxSamplerAllocationCount = limits.maxSamplerAllocationCount,
            .bufferImageGranularity = limits.bufferImageGranularity,
            .sparseAddressSpaceSize = limits.sparseAddressSpaceSize,
            .maxBoundDescriptorSets = limits.maxBoundDescriptorSets,
            .maxPerStageDescriptorSamplers = limits.maxPerStageDescriptorSamplers,
            .maxPerStageDescriptorUniformBuffers = limits.maxPerStageDescriptorUniformBuffers,
            .maxPerStageDescriptorStorageBuffers = limits.maxPerStageDescriptorStorageBuffers,
            .maxPerStageDescriptorSampledImages = limits.maxPerStageDescriptorSampledImages,
            .maxPerStageDescriptorStorageImages = limits.maxPerStageDescriptorStorageImages,
            .maxPerStageDescriptorInputAttachments = limits.maxPerStageDescriptorInputAttachments,
            .maxPerStageResources = limits.maxPerStageResources,
            .maxDescriptorSetSamplers = limits.maxDescriptorSetSamplers,
            .maxDescriptorSetUniformBuffers = limits.maxDescriptorSetUniformBuffers,
            .maxDescriptorSetUniformBuffersDynamic = limits.maxDescriptorSetUniformBuffersDynamic,
            .maxDescriptorSetStorageBuffers = limits.maxDescriptorSetStorageBuffers,
            .maxDescriptorSetStorageBuffersDynamic = limits.maxDescriptorSetStorageBuffersDynamic,
            .maxDescriptorSetSampledImages = limits.maxDescriptorSetSampledImages,
            .maxDescriptorSetStorageImages = limits.maxDescriptorSetStorageImages,
            .maxDescriptorSetInputAttachments = limits.maxDescriptorSetInputAttachments,
            .maxVertexInputAttributes = limits.maxVertexInputAttributes,
            .maxVertexInputBindings = limits.maxVertexInputBindings,
            .maxVertexInputAttributeOffset = limits.maxVertexInputAttributeOffset,
            .maxVertexInputBindingStride = limits.maxVertexInputBindingStride,
            .maxVertexOutputComponents = limits.maxVertexOutputComponents,
            .maxTessellationGenerationLevel = limits.maxTessellationGenerationLevel,
            .maxTessellationPatchSize = limits.maxTessellationPatchSize,
            .maxTessellationControlPerVertexInputComponents = limits.maxTessellationControlPerVertexInputComponents,
            .maxTessellationControlPerVertexOutputComponents = limits.maxTessellationControlPerVertexOutputComponents,
            .maxTessellationControlPerPatchOutputComponents = limits.maxTessellationControlPerPatchOutputComponents,
            .maxTessellationControlTotalOutputComponents = limits.maxTessellationControlTotalOutputComponents,
            .maxTessellationEvaluationInputComponents = limits.maxTessellationEvaluationInputComponents,
            .maxTessellationEvaluationOutputComponents = limits.maxTessellationEvaluationOutputComponents,
            .maxGeometryShaderInvocations = limits.maxGeometryShaderInvocations,
            .maxGeometryInputComponents = limits.maxGeometryInputComponents,
            .maxGeometryOutputComponents = limits.maxGeometryOutputComponents,
            .maxGeometryOutputVertices = limits.maxGeometryOutputVertices,
            .maxGeometryTotalOutputComponents = limits.maxGeometryTotalOutputComponents,
            .maxFragmentInputComponents = limits.maxFragmentInputComponents,
            .maxFragmentOutputAttachments = limits.maxFragmentOutputAttachments,
            .maxFragmentDualSrcAttachments = limits.maxFragmentDualSrcAttachments,
            .maxFragmentCombinedOutputResources = limits.maxFragmentCombinedOutputResources,
            .maxComputeSharedMemorySize = limits.maxComputeSharedMemorySize,
            .maxComputeWorkGroupCount = {
                limits.maxComputeWorkGroupCount[0],
                limits.maxComputeWorkGroupCount[1],
                limits.maxComputeWorkGroupCount[2]
            },
            .maxComputeWorkGroupInvocations = limits.maxComputeWorkGroupInvocations,
            .maxComputeWorkGroupSize = {
                limits.maxComputeWorkGroupSize[0],
                limits.maxComputeWorkGroupSize[1],
                limits.maxComputeWorkGroupSize[2]
            },
            .subPixelPrecisionBits = limits.subPixelPrecisionBits,
            .subTexelPrecisionBits = limits.subTexelPrecisionBits,
            .mipmapPrecisionBits = limits.mipmapPrecisionBits,
            .maxDrawIndexedIndexValue = limits.maxDrawIndexedIndexValue,
            .maxDrawIndirectCount = limits.maxDrawIndirectCount,
            .maxSamplerLodBias = limits.maxSamplerLodBias,
            .maxSamplerAnisotropy = limits.maxSamplerAnisotropy,
            .maxViewports = limits.maxViewports,
            .maxViewportDimensions = {
                limits.maxViewportDimensions[0],
                limits.maxViewportDimensions[1]
            },
            .viewportBoundsRange = {
                limits.viewportBoundsRange[0],
                limits.viewportBoundsRange[1]
            },
            .viewportSubPixelBits = limits.viewportSubPixelBits,
            .minMemoryMapAlignment = limits.minMemoryMapAlignment,
            .minTexelBufferOffsetAlignment = limits.minTexelBufferOffsetAlignment,
            .minUniformBufferOffsetAlignment = limits.minUniformBufferOffsetAlignment,
            .minStorageBufferOffsetAlignment = limits.minStorageBufferOffsetAlignment,
            .minTexelOffset = limits.minTexelOffset,
            .maxTexelOffset = limits.maxTexelOffset,
            .minTexelGatherOffset = limits.minTexelGatherOffset,
            .maxTexelGatherOffset = limits.maxTexelGatherOffset,
            .minInterpolationOffset = limits.minInterpolationOffset,
            .maxInterpolationOffset = limits.maxInterpolationOffset,
            .subPixelInterpolationOffsetBits = limits.subPixelInterpolationOffsetBits,
            .maxFramebufferWidth = limits.maxFramebufferWidth,
            .maxFramebufferHeight = limits.maxFramebufferHeight,
            .maxFramebufferLayers = limits.maxFramebufferLayers,
            .framebufferColorSampleCounts = limits.framebufferColorSampleCounts,
            .framebufferDepthSampleCounts = limits.framebufferDepthSampleCounts,
            .framebufferStencilSampleCounts = limits.framebufferStencilSampleCounts,
            .framebufferNoAttachmentsSampleCounts = limits.framebufferNoAttachmentsSampleCounts,
            .maxColorAttachments = limits.maxColorAttachments,
            .sampledImageColorSampleCounts = limits.sampledImageColorSampleCounts,
            .sampledImageIntegerSampleCounts = limits.sampledImageIntegerSampleCounts,
            .sampledImageDepthSampleCounts = limits.sampledImageDepthSampleCounts,
            .sampledImageStencilSampleCounts = limits.sampledImageStencilSampleCounts,
            .storageImageSampleCounts = limits.storageImageSampleCounts,
            .maxSampleMaskWords = limits.maxSampleMaskWords,
            .timestampComputeAndGraphics = static_cast<bool>(limits.timestampComputeAndGraphics),
            .timestampPeriod = limits.timestampPeriod,
            .maxClipDistances = limits.maxClipDistances,
            .maxCullDistances = limits.maxCullDistances,
            .maxCombinedClipAndCullDistances = limits.maxCombinedClipAndCullDistances,
            .discreteQueuePriorities = limits.discreteQueuePriorities,
            .pointSizeRange = {
                limits.pointSizeRange[0],
                limits.pointSizeRange[1]
            },
            .lineWidthRange = {
                limits.lineWidthRange[0],
                limits.lineWidthRange[1]
            },
            .pointSizeGranularity = limits.pointSizeGranularity,
            .lineWidthGranularity = limits.lineWidthGranularity,
            .strictLines = static_cast<bool>(limits.strictLines),
            .standardSampleLocations = static_cast<bool>(limits.standardSampleLocations),
            .optimalBufferCopyOffsetAlignment = limits.optimalBufferCopyOffsetAlignment,
            .optimalBufferCopyRowPitchAlignment = limits.optimalBufferCopyRowPitchAlignment,
            .nonCoherentAtomSize = limits.nonCoherentAtomSize
        },
        .sparseProperties = {
            .residencyStandard2DBlockShape = static_cast<bool>(sparseProperties.residencyStandard2DBlockShape),
            .residencyStandard2DMultisampleBlockShape = static_cast<bool>(sparseProperties.residencyStandard2DMultisampleBlockShape),
            .residencyStandard3DBlockShape = static_cast<bool>(sparseProperties.residencyStandard3DBlockShape),
            .residencyAlignedMipSize = static_cast<bool>(sparseProperties.residencyAlignedMipSize),
            .residencyNonResidentStrict = static_cast<bool>(sparseProperties.residencyNonResidentStrict)
        }
    };
    // clang-format-on
    return properties;
}

AdapterFeatures VulkanAdapter::queryAdapterFeatures()
{
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

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

void VulkanAdapter::createDevice(const DeviceOptions &options)
{
    // return VK_NULL_HANDLE;
}

} // namespace ToyRenderer
