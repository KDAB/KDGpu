/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDGpu/raytracing_pipeline_options.h>
#include <KDGpu/raytracing_pipeline.h>
#include <KDGpu/raytracing_pass_command_recorder.h>
#include <KDGpu/command_recorder.h>
#include <KDGpu/device.h>
#include <KDGpu/queue.h>
#include <KDGpu/instance.h>
#include <KDGpu/vulkan/vulkan_graphics_api.h>

#include <KDUtils/file.h>
#include <KDUtils/dir.h>

#include <type_traits>

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
} // namespace

TEST_SUITE("RayTracingPassCommandRecorder")
{
    std::unique_ptr<GraphicsApi> api = std::make_unique<VulkanGraphicsApi>();
    Instance instance = api->createInstance(InstanceOptions{
            .applicationName = "RayTracingPipeline",
            .applicationVersion = KDGPU_MAKE_API_VERSION(0, 1, 0, 0) });
    Adapter *discreteGPUAdapter = instance.selectAdapter(AdapterDeviceType::Default);
    Device device = discreteGPUAdapter->createDevice(DeviceOptions{
            .requestedFeatures = discreteGPUAdapter->features(),
    });
    const bool supportsRayTracing = discreteGPUAdapter->features().rayTracingPipeline;

    TEST_CASE("Construction/Destruction" * doctest::skip(!supportsRayTracing))
    {

        SUBCASE("Can't be default constructed")
        {
            // EXPECT
            REQUIRE(!std::is_default_constructible<RayTracingPassCommandRecorder>::value);
            REQUIRE(!std::is_trivially_default_constructible<RayTracingPassCommandRecorder>::value);
        }

        SUBCASE("A constructed RayTracingPassCommandRecorder from a Vulkan API")
        {
            // GIVEN
            CommandRecorder commandRecorder = device.createCommandRecorder();

            // THEN
            CHECK(commandRecorder.isValid());

            // WHEN
            RayTracingPassCommandRecorder rayTracingCommandRecorder = commandRecorder.beginRayTracingPass();

            // THEN
            CHECK(rayTracingCommandRecorder.isValid());
        }

        SUBCASE("Destruction")
        {
            CommandRecorder commandRecorder = device.createCommandRecorder();
            Handle<RayTracingPassCommandRecorder_t> recorderHandle;

            {
                // WHEN
                RayTracingPassCommandRecorder rayTracingCommandRecorder = commandRecorder.beginRayTracingPass();
                recorderHandle = rayTracingCommandRecorder.handle();

                // THEN
                CHECK(commandRecorder.isValid());
                CHECK(rayTracingCommandRecorder.isValid());
                CHECK(recorderHandle.isValid());
                CHECK(api->resourceManager()->getRayTracingPassCommandRecorder(recorderHandle) != nullptr);
            }

            // THEN
            CHECK(api->resourceManager()->getRayTracingPassCommandRecorder(recorderHandle) == nullptr);
        }
    }
}
