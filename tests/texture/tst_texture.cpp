#include <kdgpu/texture.h>
#include <kdgpu/texture_options.h>
#include <kdgpu/device.h>
#include <kdgpu/instance.h>
#include <kdgpu/vulkan/vulkan_graphics_api.h>

#include <set>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

using namespace KDGpu;

TEST_SUITE("Texture")
{
    std::unique_ptr<GraphicsApi> api = std::make_unique<VulkanGraphicsApi>();
    Instance instance = api->createInstance(InstanceOptions{
            .applicationName = "buffer",
            .applicationVersion = SERENITY_MAKE_API_VERSION(0, 1, 0, 0) });
    Adapter *discreteGPUAdapter = instance.selectAdapter(AdapterDeviceType::Default);
    Device device = discreteGPUAdapter->createDevice();

    TEST_CASE("Construction")
    {
        SUBCASE("A default constructed Texture is invalid")
        {
            // GIVEN
            Texture t;
            // THEN
            REQUIRE(!t.isValid());
        }

        SUBCASE("A constructed Texture from a Vulkan API")
        {
            // GIVEN
            const TextureOptions textureOptions = {
                .type = TextureType::TextureType2D,
                .format = Format::R8G8B8A8_SNORM,
                .extent = { 512, 512, 1 },
                .mipLevels = 1,
                .usage = TextureUsageFlagBits::SampledBit,
                .memoryUsage = MemoryUsage::GpuOnly
            };

            // WHEN
            Texture t = device.createTexture(textureOptions);

            // THEN
            CHECK(t.isValid());
        }
    }

    TEST_CASE("Destruction")
    {
        // GIVEN
        const TextureOptions textureOptions = {
            .type = TextureType::TextureType2D,
            .format = Format::R8G8B8A8_SNORM,
            .extent = { 512, 512, 1 },
            .mipLevels = 1,
            .usage = TextureUsageFlagBits::SampledBit,
            .memoryUsage = MemoryUsage::GpuOnly
        };

        Handle<Texture_t> textureHandle;

        SUBCASE("Going Out Of Scope")
        {
            {
                // WHEN
                Texture t = device.createTexture(textureOptions);
                textureHandle = t.handle();

                // THEN
                CHECK(t.isValid());
                CHECK(textureHandle.isValid());
                CHECK(api->resourceManager()->getTexture(textureHandle) != nullptr);
            }

            // THEN
            CHECK(api->resourceManager()->getTexture(textureHandle) == nullptr);
        }

        SUBCASE("Move assigment")
        {
            // WHEN
            Texture t = device.createTexture(textureOptions);
            textureHandle = t.handle();

            // THEN
            CHECK(t.isValid());
            CHECK(textureHandle.isValid());
            CHECK(api->resourceManager()->getTexture(textureHandle) != nullptr);

            // WHEN
            t = {};

            // THEN
            CHECK(api->resourceManager()->getTexture(textureHandle) == nullptr);
        }
    }

    TEST_CASE("Comparison")
    {
        SUBCASE("Compare default contructed Textures")
        {
            // GIVEN
            Texture a;
            Texture b;

            // THEN
            CHECK(a == b);
        }

        SUBCASE("Compare device created Textures")
        {
            // GIVEN
            const TextureOptions textureOptions = {
                .type = TextureType::TextureType2D,
                .format = Format::R8G8B8A8_SNORM,
                .extent = { 512, 512, 1 },
                .mipLevels = 1,
                .usage = TextureUsageFlagBits::SampledBit,
                .memoryUsage = MemoryUsage::GpuOnly
            };

            // WHEN
            Texture a = device.createTexture(textureOptions);
            Texture b = device.createTexture(textureOptions);

            // THEN
            CHECK(a != b);
        }
    }
}
