#include <toy_renderer/texture.h>
#include <toy_renderer/texture_options.h>
#include <toy_renderer/device.h>
#include <toy_renderer/instance.h>
#include <toy_renderer/vulkan/vulkan_graphics_api.h>

#include <set>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

using namespace ToyRenderer;

TEST_SUITE("TextureView")
{
    std::unique_ptr<GraphicsApi> api = std::make_unique<VulkanGraphicsApi>();
    Instance instance = api->createInstance(InstanceOptions{
            .applicationName = "TextureView",
            .applicationVersion = SERENITY_MAKE_API_VERSION(0, 1, 0, 0) });
    Adapter discreteGPUAdapter = instance.selectAdapter(AdapterDeviceType::DiscreteGpu).value_or(Adapter());
    Device device = discreteGPUAdapter.createDevice();

    TEST_CASE("Construction")
    {
        SUBCASE("A default constructed TextureView is invalid")
        {
            // GIVEN
            TextureView tv;
            // THEN
            REQUIRE(!tv.isValid());
        }

        SUBCASE("A constructed Texture from a Vulkan API")
        {
            // GIVEN
            const TextureOptions textureOptions = {
                .type = TextureType::TextureType2D,
                .format = Format::R8G8B8A8_SNORM,
                .extent = { 512, 512, 1 },
                .mipLevels = 1,
                .usage = TextureUsageFlags(static_cast<uint32_t>(TextureUsageFlagBits::SampledBit) | static_cast<uint32_t>(TextureUsageFlagBits::ColorAttachmentBit)),
                .memoryUsage = MemoryUsage::GpuOnly
            };

            // WHEN
            Texture t = device.createTexture(textureOptions);

            // THEN
            CHECK(t.isValid());

            // WHEN
            TextureView tv = t.createView();

            // THEN
            CHECK(tv.isValid());
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
            .usage = TextureUsageFlags(TextureUsageFlagBits::SampledBit),
            .memoryUsage = MemoryUsage::GpuOnly
        };

        Texture t = device.createTexture(textureOptions);

        Handle<TextureView_t> textureViewHandle;

        {
            // WHEN
            TextureView tv = t.createView();
            textureViewHandle = tv.handle();

            // THEN
            CHECK(t.isValid());
            CHECK(tv.isValid());
            CHECK(textureViewHandle.isValid());
            CHECK(api->resourceManager()->getTextureView(textureViewHandle) != nullptr);
        }

        // THEN
        CHECK(api->resourceManager()->getTextureView(textureViewHandle) == nullptr);
    }

    TEST_CASE("Comparison")
    {
        SUBCASE("Compare default contructed Textures")
        {
            // GIVEN
            TextureView a;
            TextureView b;

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
                .usage = TextureUsageFlags(static_cast<uint32_t>(TextureUsageFlagBits::SampledBit) | static_cast<uint32_t>(TextureUsageFlagBits::ColorAttachmentBit)),
                .memoryUsage = MemoryUsage::GpuOnly
            };

            // WHEN
            Texture t = device.createTexture(textureOptions);
            TextureView a = t.createView();
            TextureView b = t.createView();

            // THEN
            CHECK(a != b);
        }
    }
}
