#include <kdgpu/sampler.h>
#include <kdgpu/sampler_options.h>
#include <kdgpu/device.h>
#include <kdgpu/instance.h>
#include <kdgpu/vulkan/vulkan_graphics_api.h>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

using namespace KDGpu;

TEST_SUITE("Sampler")
{
    std::unique_ptr<GraphicsApi> api = std::make_unique<VulkanGraphicsApi>();
    Instance instance = api->createInstance(InstanceOptions{
            .applicationName = "sampler",
            .applicationVersion = SERENITY_MAKE_API_VERSION(0, 1, 0, 0) });
    Adapter *discreteGPUAdapter = instance.selectAdapter(AdapterDeviceType::Default);
    Device device = discreteGPUAdapter->createDevice();

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

            // WHEN
            Sampler s = device.createSampler(samplerOptions);

            // THEN
            CHECK(s.isValid());
        }
    }

    TEST_CASE("Destruction")
    {
        // GIVEN
        const SamplerOptions samplerOptions{};

        Handle<Sampler_t> samplerHandle;

        SUBCASE("Going Out Of Scope")
        {
            {
                // WHEN
                Sampler s = device.createSampler(samplerOptions);
                samplerHandle = s.handle();

                // THEN
                CHECK(s.isValid());
                CHECK(samplerHandle.isValid());
                CHECK(api->resourceManager()->getSampler(samplerHandle) != nullptr);
            }

            // THEN
            CHECK(api->resourceManager()->getSampler(samplerHandle) == nullptr);
        }

        SUBCASE("Move assigment")
        {
            // WHEN
            Sampler s = device.createSampler(samplerOptions);
            samplerHandle = s.handle();

            // THEN
            CHECK(s.isValid());
            CHECK(samplerHandle.isValid());
            CHECK(api->resourceManager()->getSampler(samplerHandle) != nullptr);

            // WHEN
            s = {};

            // THEN
            CHECK(api->resourceManager()->getSampler(samplerHandle) == nullptr);
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
        }

        SUBCASE("Compare device created Samplers")
        {
            // GIVEN
            const SamplerOptions samplerOptions{};

            // WHEN
            Sampler a = device.createSampler(samplerOptions);
            Sampler b = device.createSampler(samplerOptions);

            // THEN
            CHECK(a != b);
        }
    }
}
