#pragma once

namespace KDGpu {

struct AdapterFeatures {
    bool robustBufferAccess;
    bool fullDrawIndexUint32;
    bool imageCubeArray;
    bool independentBlend;
    bool geometryShader;
    bool tessellationShader;
    bool sampleRateShading;
    bool dualSrcBlend;
    bool logicOp;
    bool multiDrawIndirect;
    bool drawIndirectFirstInstance;
    bool depthClamp;
    bool depthBiasClamp;
    bool fillModeNonSolid;
    bool depthBounds;
    bool wideLines;
    bool largePoints;
    bool alphaToOne;
    bool multiViewport;
    bool samplerAnisotropy;
    bool textureCompressionETC2;
    bool textureCompressionASTC_LDR;
    bool textureCompressionBC;
    bool occlusionQueryPrecise;
    bool pipelineStatisticsQuery;
    bool vertexPipelineStoresAndAtomics;
    bool fragmentStoresAndAtomics;
    bool shaderTessellationAndGeometryPointSize;
    bool shaderImageGatherExtended;
    bool shaderStorageImageExtendedFormats;
    bool shaderStorageImageMultisample;
    bool shaderStorageImageReadWithoutFormat;
    bool shaderStorageImageWriteWithoutFormat;
    bool shaderUniformBufferArrayDynamicIndexing;
    bool shaderSampledImageArrayDynamicIndexing;
    bool shaderStorageBufferArrayDynamicIndexing;
    bool shaderStorageImageArrayDynamicIndexing;
    bool shaderClipDistance;
    bool shaderCullDistance;
    bool shaderFloat64;
    bool shaderInt64;
    bool shaderInt16;
    bool shaderResourceResidency;
    bool shaderResourceMinLod;
    bool sparseBinding;
    bool sparseResidencyBuffer;
    bool sparseResidencyImage2D;
    bool sparseResidencyImage3D;
    bool sparseResidency2Samples;
    bool sparseResidency4Samples;
    bool sparseResidency8Samples;
    bool sparseResidency16Samples;
    bool sparseResidencyAliased;
    bool variableMultisampleRate;
    bool inheritedQueries;
    bool uniformBufferStandardLayout;
    bool multiView;
    bool multiViewGeometryShader;
    bool multiViewTessellationShader;
};

} // namespace KDGpu
