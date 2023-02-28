#include <toy_renderer/sampler.h>
#include <toy_renderer/sampler_options.h>
#include <toy_renderer/device.h>
#include <toy_renderer/instance.h>
#include <toy_renderer/vulkan/vulkan_graphics_api.h>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

using namespace ToyRenderer;

TEST_SUITE("Sampler")
{
    std::unique_ptr<GraphicsApi> api = std::make_unique<VulkanGraphicsApi>();
    Instance instance = api->createInstance(InstanceOptions{
            .applicationName = "sampler",
            .applicationVersion = SERENITY_MAKE_API_VERSION(0, 1, 0, 0) });
    Adapter discreteGPUAdapter = instance.selectAdapter(AdapterDeviceType::DiscreteGpu).value_or(Adapter());
    Device device = discreteGPUAdapter.createDevice();

    TEST_CASE("Construction")
    {
        SUBCASE("A default constructed Sampler is invalid")
        {
            // GIVEN
            Sampler s;
            // THEN
            REQUIRE(!s.isValid());
        }

        SUBCASE("A constructed Sampler from a Vulkan API")
        {
            // GIVEN
            const SamplerOptions samplerOptions{};

            // // WHEN
            // Sampler s = device.createSampler(samplerOptions);

            // // THEN
            // CHECK(s.isValid());
        }
    }

    TEST_CASE("Comparison")
    {
        SUBCASE("Compare default contructed Samplers")
        {
            // GIVEN
            Sampler a;
            Sampler b;

            // THEN
            CHECK(a == b);

            // WHEN
            a = b;

            // THEN
            CHECK(a == b);
        }

        SUBCASE("Compare device created Samplers")
        {
            // GIVEN
            const SamplerOptions samplerOptions{};

            // // WHEN
            // Sampler a = device.createSampler(samplerOptions);
            // Sampler b = device.createSampler(samplerOptions);

            // // THEN
            // CHECK(a != b);

            // // WHEN
            // a = b;

            // // THEN
            // CHECK(a == b);
        }
    }
}
