#include <toy_renderer/compute_pipeline.h>
#include <toy_renderer/compute_pipeline_options.h>
#include <toy_renderer/device.h>
#include <toy_renderer/instance.h>
#include <toy_renderer/vulkan/vulkan_graphics_api.h>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

using namespace ToyRenderer;

TEST_SUITE("ComputePipeline")
{
    std::unique_ptr<GraphicsApi> api = std::make_unique<VulkanGraphicsApi>();
    Instance instance = api->createInstance(InstanceOptions{
            .applicationName = "ComputePipeline",
            .applicationVersion = SERENITY_MAKE_API_VERSION(0, 1, 0, 0) });
    Adapter discreteGPUAdapter = instance.selectAdapter(AdapterDeviceType::DiscreteGpu).value_or(Adapter());
    Device device = discreteGPUAdapter.createDevice();

    TEST_CASE("Construction")
    {
        SUBCASE("A default constructed ComputePipeline is invalid")
        {
            // GIVEN
            ComputePipeline c;
            // THEN
            REQUIRE(!c.isValid());
        }

        SUBCASE("A constructed ComputePipeline from a Vulkan API")
        {
            // GIVEN
            const ComputePipelineOptions options{};

            // // WHEN
            // ComputePipeline c = device.createComputePipeline(options);

            // // THEN
            // CHECK(c.isValid());
        }
    }

    TEST_CASE("Comparison")
    {
        SUBCASE("Compare default contructed ComputePipelines")
        {
            // GIVEN
            ComputePipeline a;
            ComputePipeline b;

            // THEN
            CHECK(a == b);

            // WHEN
            a = b;

            // THEN
            CHECK(a == b);
        }

        SUBCASE("Compare device created ComputePipelines")
        {
            // GIVEN
            const ComputePipelineOptions options{};

            // WHEN
            // ComputePipeline a = device.createComputePipeline(options);
            // ComputePipeline b = device.createComputePipeline(options);

            // // THEN
            // CHECK(a != b);

            // // WHEN
            // a = b;

            // // THEN
            // CHECK(a == b);
        }
    }
}
