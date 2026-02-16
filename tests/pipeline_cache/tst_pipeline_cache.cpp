/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2026 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDGpu/graphics_pipeline.h>
#include <KDGpu/graphics_pipeline_options.h>
#include <KDGpu/pipeline_cache.h>
#include <KDGpu/pipeline_cache_options.h>
#include <KDGpu/device.h>
#include <KDGpu/instance.h>
#include <KDGpu/vulkan/vulkan_graphics_api.h>

#include <KDUtils/file.h>
#include <KDUtils/dir.h>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

using namespace KDGpu;

namespace {

inline std::string assetPath()
{
#if defined(KDGPU_ASSET_PATH)
    return KDGPU_ASSET_PATH;
#else
    return "";
#endif
}

std::vector<uint32_t> readShaderFile(const std::string &filename)
{
    using namespace KDUtils;

    File file(File::exists(filename) ? filename : Dir::applicationDir().absoluteFilePath(filename));

    if (!file.open(std::ios::in | std::ios::binary)) {
        SPDLOG_CRITICAL("Failed to open file {}", filename);
        throw std::runtime_error("Failed to open file");
    }

    const ByteArray fileContent = file.readAll();
    std::vector<uint32_t> buffer(fileContent.size() / 4);
    std::memcpy(buffer.data(), fileContent.data(), fileContent.size());

    return buffer;
}

Format selectDepthFormat(Adapter *adapter)
{
    // Choose a depth format from the ones supported
    constexpr std::array<Format, 5> preferredDepthFormat = {
        Format::D24_UNORM_S8_UINT,
        Format::D16_UNORM_S8_UINT,
        Format::D32_SFLOAT_S8_UINT,
        Format::D16_UNORM,
        Format::D32_SFLOAT
    };

    for (const auto &depthFormat : preferredDepthFormat) {
        const FormatProperties formatProperties = adapter->formatProperties(depthFormat);
        if (formatProperties.optimalTilingFeatures & FormatFeatureFlagBit::DepthStencilAttachmentBit) {
            return depthFormat;
        }
    }

    return Format::UNDEFINED;
}

void populateCacheWithPipeline(Handle<PipelineCache_t> cacheHandle, Adapter *adapter, Device &device)
{
    const auto vertexShaderPath = assetPath() + "/shaders/tests/graphics_pipeline/triangle.vert.spv";
    auto vertexShader = device.createShaderModule(readShaderFile(vertexShaderPath));

    const auto fragmentShaderPath = assetPath() + "/shaders/tests/graphics_pipeline/triangle.frag.spv";
    auto fragmentShader = device.createShaderModule(readShaderFile(fragmentShaderPath));

    PipelineLayoutOptions pipelineLayoutOptions{};
    PipelineLayout pipelineLayout = device.createPipelineLayout(pipelineLayoutOptions);
    const Format depthFormat = selectDepthFormat(adapter);
    REQUIRE(depthFormat != Format::UNDEFINED);
    REQUIRE(vertexShader.isValid());
    REQUIRE(fragmentShader.isValid());
    REQUIRE(pipelineLayout.isValid());

    const uint32_t stride = static_cast<uint32_t>(2u * 4u * sizeof(float));
    const uint32_t colorOffset = static_cast<uint32_t>(4u * sizeof(float));

    GraphicsPipelineOptions pipelineOptions = {
        .shaderStages = {
                { .shaderModule = vertexShader.handle(), .stage = ShaderStageFlagBits::VertexBit },
                { .shaderModule = fragmentShader.handle(), .stage = ShaderStageFlagBits::FragmentBit },
        },
        .layout = pipelineLayout.handle(),
        .vertex = {
                .buffers = { { .binding = 0, .stride = stride } },
                .attributes = {
                        {
                                .location = 0,
                                .binding = 0,
                                .format = Format::R32G32B32A32_SFLOAT,
                        }, // Position
                        {
                                .location = 1,
                                .binding = 0,
                                .format = Format::R32G32B32A32_SFLOAT,
                                .offset = colorOffset,
                        }, // Color
                },
        },
        .renderTargets = { { .format = Format::R8G8B8A8_UNORM } },
        .depthStencil = {
                .format = depthFormat,
                .depthWritesEnabled = true,
                .depthCompareOperation = CompareOperation::Less,
        },
        .pipelineCache = cacheHandle,
    };

    GraphicsPipeline g1 = device.createGraphicsPipeline(pipelineOptions);
    CHECK(g1.isValid());
};

} // namespace

TEST_SUITE("PipelineCache")
{
    std::unique_ptr<GraphicsApi> api = std::make_unique<VulkanGraphicsApi>();
    Instance instance = api->createInstance(InstanceOptions{
            .applicationName = "PipelineCache",
            .applicationVersion = KDGPU_MAKE_API_VERSION(0, 1, 0, 0) });
    Adapter *adapter = instance.selectAdapter(AdapterDeviceType::Default);

    TEST_CASE("Construction")
    {
        Device device = adapter->createDevice();

        SUBCASE("A default constructed PipelineCache is invalid")
        {
            // GIVEN
            PipelineCache cache;
            // THEN
            REQUIRE(!cache.isValid());
        }

        SUBCASE("A constructed PipelineCache from a Vulkan API")
        {
            // GIVEN
            const PipelineCacheOptions cacheOptions{};

            // WHEN
            PipelineCache cache = device.createPipelineCache(cacheOptions);

            // THEN
            CHECK(cache.isValid());
        }

        SUBCASE("A constructed PipelineCache with initial data")
        {
            // GIVEN
            // First create a cache, get its data
            PipelineCache cache1 = device.createPipelineCache();
            CHECK(cache1.isValid());

            // WHEN
            populateCacheWithPipeline(cache1, adapter, device);
            std::vector<uint8_t> cacheData = cache1.getData();

            // THEN
            CHECK(cacheData.size() > 0);

            // WHEN - Create new cache with initial data
            const PipelineCacheOptions cacheOptions{
                .initialData = cacheData
            };
            PipelineCache cache2 = device.createPipelineCache(cacheOptions);

            // THEN
            CHECK(cache2.isValid());
        }

        SUBCASE("Move constructor & move assignment")
        {
            // GIVEN
            const PipelineCacheOptions cacheOptions{};

            PipelineCache cache1 = device.createPipelineCache(cacheOptions);

            // WHEN
            populateCacheWithPipeline(cache1, adapter, device);
            const std::vector<uint8_t> cache1Data = cache1.getData();

            // THEN
            CHECK(cache1Data.size() > 0);

            // WHEN
            PipelineCache cache2(std::move(cache1));

            // THEN
            CHECK(cache2.isValid());
            CHECK(!cache1.isValid());

            // WHEN
            const std::vector<uint8_t> cache2Data = cache2.getData();

            // THEN
            CHECK(cache2Data.size() > 0);
            CHECK(cache2Data == cache1Data);

            // WHEN
            PipelineCache cache3 = device.createPipelineCache(cacheOptions);
            const auto cache2Handle = cache2.handle();
            cache3 = std::move(cache2);

            // THEN
            CHECK(cache3.isValid());
            CHECK(!cache2.isValid());
            CHECK(cache3.handle() == cache2Handle);
        }
    }

    TEST_CASE("Destruction")
    {
        Device device = adapter->createDevice();

        // GIVEN
        const PipelineCacheOptions cacheOptions{};

        Handle<PipelineCache_t> cacheHandle;

        SUBCASE("Going Out Of Scope")
        {
            {
                // WHEN
                PipelineCache cache = device.createPipelineCache(cacheOptions);
                cacheHandle = cache.handle();

                // THEN
                CHECK(cache.isValid());
                CHECK(cacheHandle.isValid());
                CHECK(api->resourceManager()->getPipelineCache(cacheHandle) != nullptr);
            }

            // THEN
            CHECK(api->resourceManager()->getPipelineCache(cacheHandle) == nullptr);
        }
    }

    TEST_CASE("getData")
    {
        Device device = adapter->createDevice();

        // GIVEN
        PipelineCache cache = device.createPipelineCache();
        CHECK(cache.isValid());

        // WHEN
        std::vector<uint8_t> data = cache.getData();

        // THEN
        // If cache is empty, expect data to return a possibly empty vector (though it should still contain a header, so size should be > 0)
        CHECK(data.size() >= 0);

        // WHEN
        populateCacheWithPipeline(cache, adapter, device);
        data = cache.getData();

        // THEN
        CHECK(data.size() > 0);
    }

    TEST_CASE("merge")
    {
        Device device = adapter->createDevice();

        SUBCASE("Merge single source cache")
        {
            // GIVEN
            PipelineCache dstCache = device.createPipelineCache();
            PipelineCache srcCache = device.createPipelineCache();
            CHECK(dstCache.isValid());
            CHECK(srcCache.isValid());

            // WHEN
            populateCacheWithPipeline(srcCache, adapter, device);

            // THEN
            CHECK(srcCache.getData().size() > 0);

            // WHEN
            std::vector<Handle<PipelineCache_t>> sources = { srcCache.handle() };
            dstCache.merge(sources);

            // THEN
            CHECK(dstCache.isValid());
            CHECK(srcCache.isValid()); // Source should remain valid
        }

        SUBCASE("Merge multiple source caches")
        {
            // GIVEN
            PipelineCache dstCache = device.createPipelineCache();
            PipelineCache srcCache1 = device.createPipelineCache();
            PipelineCache srcCache2 = device.createPipelineCache();
            PipelineCache srcCache3 = device.createPipelineCache();
            CHECK(dstCache.isValid());
            CHECK(srcCache1.isValid());
            CHECK(srcCache2.isValid());
            CHECK(srcCache3.isValid());

            // WHEN
            populateCacheWithPipeline(srcCache1, adapter, device);
            populateCacheWithPipeline(srcCache2, adapter, device);
            populateCacheWithPipeline(srcCache3, adapter, device);

            std::vector<Handle<PipelineCache_t>> sources = {
                srcCache1.handle(),
                srcCache2.handle(),
                srcCache3.handle()
            };
            dstCache.merge(sources);

            // THEN
            CHECK(dstCache.isValid());
            CHECK(srcCache1.isValid());
            CHECK(srcCache2.isValid());
            CHECK(srcCache3.isValid());
        }

        SUBCASE("Merge with empty sources")
        {
            // GIVEN
            PipelineCache dstCache = device.createPipelineCache();
            CHECK(dstCache.isValid());

            // WHEN
            std::vector<Handle<PipelineCache_t>> sources;
            dstCache.merge(sources);

            // THEN
            CHECK(dstCache.isValid());
        }
    }

    TEST_CASE("Comparison")
    {
        Device device = adapter->createDevice();

        SUBCASE("Equality")
        {
            // GIVEN
            PipelineCache cache1 = device.createPipelineCache();
            PipelineCache cache2 = device.createPipelineCache();

            // THEN
            CHECK(cache1 != cache2);
            CHECK(!(cache1 == cache2));
        }

        SUBCASE("Self equality")
        {
            // GIVEN
            PipelineCache cache1 = device.createPipelineCache();

            // THEN
            CHECK(cache1 == cache1);
            CHECK(!(cache1 != cache1));
        }
    }
}
