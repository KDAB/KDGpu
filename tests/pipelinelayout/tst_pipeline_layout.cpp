#include <toy_renderer/pipeline_layout.h>
#include <toy_renderer/pipeline_layout_options.h>
#include <toy_renderer/device.h>
#include <toy_renderer/instance.h>
#include <toy_renderer/vulkan/vulkan_graphics_api.h>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

using namespace ToyRenderer;

TEST_SUITE("PipelineLayout")
{
    std::unique_ptr<GraphicsApi> api = std::make_unique<VulkanGraphicsApi>();
    Instance instance = api->createInstance(InstanceOptions{
            .applicationName = "PipelineLayout",
            .applicationVersion = SERENITY_MAKE_API_VERSION(0, 1, 0, 0) });
    Adapter discreteGPUAdapter = instance.selectAdapter(AdapterDeviceType::DiscreteGpu).value_or(Adapter());
    Device device = discreteGPUAdapter.createDevice();

    TEST_CASE("Construction")
    {
        SUBCASE("A default constructed PipelineLayout is invalid")
        {
            // GIVEN
            PipelineLayout t;
            // THEN
            REQUIRE(!t.isValid());
        }

        SUBCASE("A constructed PipelineLayout from a Vulkan API")
        {
            // GIVEN
            PipelineLayoutOptions pipelineLayoutOptions = {
                .bindGroupLayouts = {}
            };

            // WHEN
            PipelineLayout pipelineLayout = device.createPipelineLayout(pipelineLayoutOptions);

            // THEN
            CHECK(pipelineLayout.isValid());
        }
    }

    TEST_CASE("Destruction")
    {
        // GIVEN
        PipelineLayoutOptions pipelineLayoutOptions = {
            .bindGroupLayouts = {}
        };

        Handle<PipelineLayout_t> pipelineLayoutHandle;

        {
            // WHEN
            PipelineLayout pipelineLayout = device.createPipelineLayout(pipelineLayoutOptions);
            pipelineLayoutHandle = pipelineLayout.handle();

            // THEN
            CHECK(pipelineLayout.isValid());
            CHECK(pipelineLayoutHandle.isValid());
            CHECK(api->resourceManager()->getPipelineLayout(pipelineLayoutHandle) != nullptr);
        }

        // THEN
        CHECK(api->resourceManager()->getPipelineLayout(pipelineLayoutHandle) == nullptr);
    }

    TEST_CASE("Comparison")
    {
        SUBCASE("Compare default contructed PipelineLayouts")
        {
            // GIVEN
            PipelineLayout a;
            PipelineLayout b;

            // THEN
            CHECK(a == b);
        }

        SUBCASE("Compare device created PipelineLayouts")
        {
            // GIVEN
            PipelineLayoutOptions pipelineLayoutOptions = {
                .bindGroupLayouts = {}
            };

            // WHEN
            PipelineLayout a = device.createPipelineLayout(pipelineLayoutOptions);
            PipelineLayout b = device.createPipelineLayout(pipelineLayoutOptions);

            // THEN
            CHECK(a != b);
            CHECK(a == a);
        }
    }
}
