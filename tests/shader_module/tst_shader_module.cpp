/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDGpu/shader_module.h>
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

} // namespace

TEST_SUITE("Shader_Module")
{
    std::unique_ptr<GraphicsApi> api = std::make_unique<VulkanGraphicsApi>();
    Instance instance = api->createInstance(InstanceOptions{
            .applicationName = "Shader_Module",
            .applicationVersion = KDGPU_MAKE_API_VERSION(0, 1, 0, 0) });
    Adapter *discreteGPUAdapter = instance.selectAdapter(AdapterDeviceType::Default);
    Device device = discreteGPUAdapter->createDevice();

    TEST_CASE("Construction")
    {
        SUBCASE("A default constructed ShaderModule is invalid")
        {
            // GIVEN
            ShaderModule s;
            // THEN
            REQUIRE(!s.isValid());
        }
        SUBCASE("A constructed ShaderModule from a Vulkan API")
        {
            // GIVEN
            const auto shaderPath = assetPath() + "/shaders/tests/compute_pipeline/empty_compute.comp.spv";
            const auto shaderCode = readShaderFile(shaderPath);

            // WHEN
            ShaderModule s = device.createShaderModule(shaderCode);

            // THEN
            CHECK(s.isValid());
        }

        SUBCASE("Move constructor & move assigment")
        {
            // GIVEN
            const auto shaderPath = assetPath() + "/shaders/tests/compute_pipeline/empty_compute.comp.spv";
            const auto shaderCode = readShaderFile(shaderPath);

            ShaderModule shaderModule = device.createShaderModule(shaderCode);

            // WHEN
            ShaderModule shaderModule2(std::move(shaderModule));

            // THEN
            CHECK(!shaderModule.isValid());
            CHECK(shaderModule2.isValid());

            // WHEN
            ShaderModule shaderModule3 = device.createShaderModule(shaderCode);
            const auto shaderModule2Handle = shaderModule2.handle();
            shaderModule3 = std::move(shaderModule2);

            // THEN
            CHECK(shaderModule3.isValid());
            CHECK(!shaderModule2.isValid());
            CHECK(shaderModule3.handle() == shaderModule2Handle);
        }
    }
}
