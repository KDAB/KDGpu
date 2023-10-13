/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "vulkan_adapter.h"

#include <KDGpu/vulkan/vulkan_enums.h>
#include <KDGpu/vulkan/vulkan_resource_manager.h>
#include <KDGpu/vulkan/vulkan_surface.h>

namespace KDGpu {

VulkanAdapter::VulkanAdapter(VkPhysicalDevice _physicalDevice,
                             VulkanResourceManager *_vulkanResourceManager,
                             const Handle<Instance_t> &_instanceHandle)
    : ApiAdapter()
    , physicalDevice(_physicalDevice)
    , vulkanResourceManager(_vulkanResourceManager)
    , instanceHandle(_instanceHandle)
{
}

std::vector<Extension> VulkanAdapter::extensions() const
{
    uint32_t extensionCount{ 0 };
    if (vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr) != VK_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Unable to enumerate instance extensions");
        return {};
    }

    std::vector<VkExtensionProperties> vkExtensions(extensionCount);
    if (vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, vkExtensions.data()) != VK_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Unable to query instance extensions");
        return {};
    }

    std::vector<Extension> extensions;
    extensions.reserve(extensionCount);
    for (const auto &vkExtension : vkExtensions) {
        extensions.emplace_back(Extension{
                .name = vkExtension.extensionName,
                .version = vkExtension.specVersion });
    }

    return extensions;
}

AdapterProperties VulkanAdapter::queryAdapterProperties()
{
    VkPhysicalDeviceProperties2 deviceProperties2{};
    deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;

    VkPhysicalDeviceMultiviewProperties multiViewProperties{};
    multiViewProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PROPERTIES;

    VkPhysicalDeviceDepthStencilResolveProperties depthResolveProps{};
    depthResolveProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_STENCIL_RESOLVE_PROPERTIES;
    multiViewProperties.pNext = &depthResolveProps;

    VkPhysicalDeviceDescriptorIndexingProperties descriptorIndexingProperties{};
    descriptorIndexingProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES;
    depthResolveProps.pNext = &descriptorIndexingProperties;

    deviceProperties2.pNext = &multiViewProperties;

    vkGetPhysicalDeviceProperties2(physicalDevice, &deviceProperties2);

    const VkPhysicalDeviceProperties &deviceProperties = deviceProperties2.properties;

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
            .framebufferColorSampleCounts = SampleCountFlags::fromInt(limits.framebufferColorSampleCounts),
            .framebufferDepthSampleCounts = SampleCountFlags::fromInt(limits.framebufferDepthSampleCounts),
            .framebufferStencilSampleCounts = SampleCountFlags::fromInt(limits.framebufferStencilSampleCounts),
            .framebufferNoAttachmentsSampleCounts = SampleCountFlags::fromInt(limits.framebufferNoAttachmentsSampleCounts),
            .maxColorAttachments = limits.maxColorAttachments,
            .sampledImageColorSampleCounts = SampleCountFlags::fromInt(limits.sampledImageColorSampleCounts),
            .sampledImageIntegerSampleCounts = SampleCountFlags::fromInt(limits.sampledImageIntegerSampleCounts),
            .sampledImageDepthSampleCounts = SampleCountFlags::fromInt(limits.sampledImageDepthSampleCounts),
            .sampledImageStencilSampleCounts = SampleCountFlags::fromInt(limits.sampledImageStencilSampleCounts),
            .storageImageSampleCounts = SampleCountFlags::fromInt(limits.storageImageSampleCounts),
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
        },
        .multiViewProperties = {
            .maxMultiViewCount = multiViewProperties.maxMultiviewViewCount,
            .maxMultiviewInstanceIndex = multiViewProperties.maxMultiviewInstanceIndex,
        },
        .depthResolveProperties = {
            .supportedDepthResolveModes = vkResolveModesToResolveModes(depthResolveProps.supportedDepthResolveModes),
            .supportedStencilResolveModes = vkResolveModesToResolveModes(depthResolveProps.supportedStencilResolveModes),
            .independentResolveNone = static_cast<bool>(depthResolveProps.independentResolveNone),
            .independentResolve = static_cast<bool>(depthResolveProps.independentResolve),
        },
        .bindGroupIndexingProperties = {
            .maxUpdateAfterBindBindGroups = descriptorIndexingProperties.maxUpdateAfterBindDescriptorsInAllPools,
            .shaderUniformBufferArrayNonUniformIndexingNative  = static_cast<bool>(descriptorIndexingProperties.shaderUniformBufferArrayNonUniformIndexingNative),
            .shaderSampledImageArrayNonUniformIndexingNative = static_cast<bool>(descriptorIndexingProperties.shaderSampledImageArrayNonUniformIndexingNative),
            .shaderStorageBufferArrayNonUniformIndexingNative = static_cast<bool>(descriptorIndexingProperties.shaderStorageBufferArrayNonUniformIndexingNative),
            .shaderStorageImageArrayNonUniformIndexingNative = static_cast<bool>(descriptorIndexingProperties.shaderStorageImageArrayNonUniformIndexingNative),
            .shaderInputAttachmentArrayNonUniformIndexingNative = static_cast<bool>(descriptorIndexingProperties.shaderInputAttachmentArrayNonUniformIndexingNative),
            .robustBufferAccessUpdateAfterBind = static_cast<bool>(descriptorIndexingProperties.robustBufferAccessUpdateAfterBind),
            .quadDivergentImplicitLod = static_cast<bool>(descriptorIndexingProperties.quadDivergentImplicitLod),
            .maxPerStageBindGroupEntriesUpdateAfterBindSamplers  = descriptorIndexingProperties.maxPerStageDescriptorUpdateAfterBindSamplers,
            .maxPerStageBindGroupEntriesUpdateAfterBindUniformBuffers  = descriptorIndexingProperties.maxPerStageDescriptorUpdateAfterBindUniformBuffers,
            .maxPerStageBindGroupEntriesUpdateAfterBindStorageBuffers  = descriptorIndexingProperties.maxPerStageDescriptorUpdateAfterBindStorageBuffers,
            .maxPerStageBindGroupEntriesUpdateAfterBindSampledImages  = descriptorIndexingProperties.maxPerStageDescriptorUpdateAfterBindSampledImages,
            .maxPerStageBindGroupEntriesUpdateAfterBindStorageImages = descriptorIndexingProperties.maxPerStageDescriptorUpdateAfterBindStorageImages,
            .maxPerStageBindGroupEntriesUpdateAfterBindInputAttachments = descriptorIndexingProperties.maxPerStageDescriptorUpdateAfterBindInputAttachments,
            .maxPerStageUpdateAfterBindResources = descriptorIndexingProperties.maxPerStageUpdateAfterBindResources,
            .maxBindGroupUpdateAfterBindSamplers = descriptorIndexingProperties.maxDescriptorSetUpdateAfterBindSamplers,
            .maxBindGroupUpdateAfterBindUniformBuffers = descriptorIndexingProperties.maxDescriptorSetUpdateAfterBindUniformBuffers,
            .maxBindGroupUpdateAfterBindUniformBuffersDynamic = descriptorIndexingProperties.maxDescriptorSetUpdateAfterBindUniformBuffersDynamic,
            .maxBindGroupUpdateAfterBindStorageBuffers = descriptorIndexingProperties.maxDescriptorSetUpdateAfterBindStorageBuffers,
            .maxBindGroupUpdateAfterBindStorageBuffersDynamic = descriptorIndexingProperties.maxDescriptorSetUpdateAfterBindStorageBuffersDynamic,
            .maxBindGroupUpdateAfterBindSampledImages = descriptorIndexingProperties.maxDescriptorSetUpdateAfterBindSampledImages,
            .maxBindGroupUpdateAfterBindStorageImages = descriptorIndexingProperties.maxDescriptorSetUpdateAfterBindStorageImages,
            .maxBindGroupUpdateAfterBindInputAttachments = descriptorIndexingProperties.maxDescriptorSetUpdateAfterBindInputAttachments,
        }
    };
    // clang-format-on
    return properties;
}

AdapterFeatures VulkanAdapter::queryAdapterFeatures()
{
    VkPhysicalDeviceFeatures2 deviceFeatures2 {};
    deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

    VkPhysicalDeviceMultiviewFeatures multiViewFeatures {};
    multiViewFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES;
    deviceFeatures2.pNext = &multiViewFeatures; // So that it gets filled by the vkGetPhysicalDeviceFeatures2 call

    VkPhysicalDeviceUniformBufferStandardLayoutFeatures stdLayoutFeatures {};
    stdLayoutFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFORM_BUFFER_STANDARD_LAYOUT_FEATURES;
    multiViewFeatures.pNext = &stdLayoutFeatures; // So that it gets filled by the vkGetPhysicalDeviceFeatures2 call

    VkPhysicalDeviceDescriptorIndexingFeatures deviceDescriptorIndexingFeatures {};
    deviceDescriptorIndexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
    stdLayoutFeatures.pNext = &deviceDescriptorIndexingFeatures; // So that it gets filled by the vkGetPhysicalDeviceFeatures2 call

#if defined(VK_KHR_synchronization2)
    VkPhysicalDeviceSynchronization2Features synchronization2Features{};
    synchronization2Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
    deviceDescriptorIndexingFeatures.pNext = &synchronization2Features;
#endif

    vkGetPhysicalDeviceFeatures2(physicalDevice, &deviceFeatures2);
    const VkPhysicalDeviceFeatures &deviceFeatures = deviceFeatures2.features;

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
        .inheritedQueries = static_cast<bool>(deviceFeatures.inheritedQueries),
        .uniformBufferStandardLayout = static_cast<bool>(stdLayoutFeatures.uniformBufferStandardLayout),
        .multiView = static_cast<bool>(multiViewFeatures.multiview),
        .multiViewGeometryShader = static_cast<bool>(multiViewFeatures.multiviewGeometryShader),
        .multiViewTessellationShader = static_cast<bool>(multiViewFeatures.multiviewTessellationShader),
        .shaderInputAttachmentArrayDynamicIndexing = static_cast<bool>(deviceDescriptorIndexingFeatures.shaderInputAttachmentArrayDynamicIndexing),
        .shaderUniformTexelBufferArrayDynamicIndexing  = static_cast<bool>(deviceDescriptorIndexingFeatures.shaderUniformTexelBufferArrayDynamicIndexing),
        .shaderStorageTexelBufferArrayDynamicIndexing = static_cast<bool>(deviceDescriptorIndexingFeatures.shaderStorageTexelBufferArrayDynamicIndexing),
        .shaderUniformBufferArrayNonUniformIndexing = static_cast<bool>(deviceDescriptorIndexingFeatures.shaderUniformBufferArrayNonUniformIndexing),
        .shaderSampledImageArrayNonUniformIndexing = static_cast<bool>(deviceDescriptorIndexingFeatures.shaderSampledImageArrayNonUniformIndexing),
        .shaderStorageBufferArrayNonUniformIndexing = static_cast<bool>(deviceDescriptorIndexingFeatures.shaderStorageBufferArrayNonUniformIndexing),
        .shaderStorageImageArrayNonUniformIndexing = static_cast<bool>(deviceDescriptorIndexingFeatures.shaderStorageImageArrayNonUniformIndexing),
        .shaderInputAttachmentArrayNonUniformIndexing = static_cast<bool>(deviceDescriptorIndexingFeatures.shaderInputAttachmentArrayNonUniformIndexing),
        .shaderUniformTexelBufferArrayNonUniformIndexing = static_cast<bool>(deviceDescriptorIndexingFeatures.shaderUniformTexelBufferArrayNonUniformIndexing),
        .shaderStorageTexelBufferArrayNonUniformIndexing = static_cast<bool>(deviceDescriptorIndexingFeatures.shaderStorageTexelBufferArrayNonUniformIndexing),
        .bindGroupBindingUniformBufferUpdateAfterBind = static_cast<bool>(deviceDescriptorIndexingFeatures.descriptorBindingUniformBufferUpdateAfterBind),
        .bindGroupBindingSampledImageUpdateAfterBind = static_cast<bool>(deviceDescriptorIndexingFeatures.descriptorBindingSampledImageUpdateAfterBind),
        .bindGroupBindingStorageImageUpdateAfterBind = static_cast<bool>(deviceDescriptorIndexingFeatures.descriptorBindingStorageImageUpdateAfterBind),
        .bindGroupBindingStorageBufferUpdateAfterBind = static_cast<bool>(deviceDescriptorIndexingFeatures.descriptorBindingStorageBufferUpdateAfterBind),
        .bindGroupBindingUniformTexelBufferUpdateAfterBind = static_cast<bool>(deviceDescriptorIndexingFeatures.descriptorBindingUniformTexelBufferUpdateAfterBind),
        .bindGroupBindingStorageTexelBufferUpdateAfterBind = static_cast<bool>(deviceDescriptorIndexingFeatures.descriptorBindingStorageTexelBufferUpdateAfterBind),
        .bindGroupBindingUpdateUnusedWhilePending = static_cast<bool>(deviceDescriptorIndexingFeatures.descriptorBindingUpdateUnusedWhilePending),
        .bindGroupBindingPartiallyBound = static_cast<bool>(deviceDescriptorIndexingFeatures.descriptorBindingPartiallyBound),
        .bindGroupBindingVariableDescriptorCount = static_cast<bool>(deviceDescriptorIndexingFeatures.descriptorBindingVariableDescriptorCount),
        .runtimeBindGroupArray = static_cast<bool>(deviceDescriptorIndexingFeatures.runtimeDescriptorArray),
    };

#if defined(VK_KHR_synchronization2)
    supportsSynchronization2 = synchronization2Features.synchronization2;
#endif

    return features;
}

AdapterSwapchainProperties VulkanAdapter::querySwapchainProperties(const Handle<Surface_t> &surfaceHandle)
{
    AdapterSwapchainProperties properties = {};

    // Get the capabilities
    VulkanSurface surface = *vulkanResourceManager->getSurface(surfaceHandle);
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface.surface, &capabilities);

    properties.capabilities = {
        .minImageCount = capabilities.minImageCount,
        .maxImageCount = capabilities.maxImageCount,
        .currentExtent = { capabilities.currentExtent.width, capabilities.currentExtent.height },
        .minImageExtent = { capabilities.minImageExtent.width, capabilities.minImageExtent.height },
        .maxImageExtent = { capabilities.maxImageExtent.width, capabilities.maxImageExtent.height },
        .maxImageArrayLayers = capabilities.maxImageArrayLayers,
        .supportedTransforms = SurfaceTransformFlags::fromInt(capabilities.supportedTransforms),
        .currentTransform = vkSurfaceTransformFlagBitsKHRToSurfaceTransformFlagBits(capabilities.currentTransform),
        .supportedCompositeAlpha = CompositeAlphaFlags::fromInt(capabilities.supportedCompositeAlpha),
        .supportedUsageFlags = TextureUsageFlags::fromInt(capabilities.supportedUsageFlags)
    };

    // Get the supported formats and colorspaces
    uint32_t formatCount;
    std::vector<VkSurfaceFormatKHR> vkFormats;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface.surface, &formatCount, nullptr);
    if (formatCount != 0) {
        vkFormats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface.surface, &formatCount, vkFormats.data());
    }

    std::vector<SurfaceFormat> formats;
    formats.reserve(formatCount);
    for (uint32_t i = 0; i < formatCount; ++i) {
        formats.emplace_back(SurfaceFormat {
            vkFormatToFormat(vkFormats[i].format),
            vkColorSpaceKHRToColorSpace(vkFormats[i].colorSpace)
        });
    }
    properties.formats = std::move(formats);

    // Get the supported present modes
    uint32_t presentModeCount;
    std::vector<VkPresentModeKHR> vkPresentModes;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface.surface, &presentModeCount, nullptr);
    if (presentModeCount != 0) {
        vkPresentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface.surface, &presentModeCount, vkPresentModes.data());
    }

    std::vector<PresentMode> presentModes;
    presentModes.reserve(presentModeCount);
    for (uint32_t i = 0; i < presentModeCount; ++i)
        presentModes.emplace_back(vkPresentModeKHRToPresentMode(vkPresentModes[i]));
    properties.presentModes = std::move(presentModes);

    return properties;
}

std::vector<AdapterQueueType> VulkanAdapter::queryQueueTypes()
{
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

    queueTypes.clear();
    queueTypes.reserve(queueFamilyCount);
    for (uint32_t i = 0; i < queueFamilyCount; ++i) {
        const auto &queueFamily = queueFamilies[i];
        queueTypes.emplace_back(
            AdapterQueueType {
                .flags = QueueFlags::fromInt(queueFamily.queueFlags),
                .availableQueues = queueFamily.queueCount,
                .timestampValidBits = queueFamily.timestampValidBits,
                .minImageTransferGranularity = {
                    .width = queueFamily.minImageTransferGranularity.width,
                    .height = queueFamily.minImageTransferGranularity.height,
                    .depth = queueFamily.minImageTransferGranularity.depth
                }
            }
        );
    }

    return queueTypes;
}

bool VulkanAdapter::supportsPresentation(const Handle<Surface_t> surfaceHandle, uint32_t queueTypeIndex)
{
    VulkanSurface vulkanSurface = *vulkanResourceManager->getSurface(surfaceHandle);
    VkBool32 canPresent = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueTypeIndex, vulkanSurface.surface, &canPresent);
    return canPresent;
}

FormatProperties VulkanAdapter::formatProperties(Format format) const
{
    VkFormatProperties2 props{};
    props.sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2;
    vkGetPhysicalDeviceFormatProperties2(physicalDevice, static_cast<VkFormat>(format), &props);

    return FormatProperties {
        .linearTilingFeatures = FormatFeatureFlags::fromInt(props.formatProperties.linearTilingFeatures),
        .optimalTilingFeatures = FormatFeatureFlags::fromInt(props.formatProperties.optimalTilingFeatures),
        .bufferFeatures = FormatFeatureFlags::fromInt(props.formatProperties.bufferFeatures)
    };
}

} // namespace KDGpu
