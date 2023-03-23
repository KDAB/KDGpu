#include <toy_renderer/fence.h>
#include <toy_renderer/device.h>
#include <toy_renderer/instance.h>
#include <toy_renderer/vulkan/vulkan_graphics_api.h>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

using namespace ToyRenderer;

TEST_SUITE("Fence")
{
    std::unique_ptr<GraphicsApi> api = std::make_unique<VulkanGraphicsApi>();
    Instance instance = api->createInstance(InstanceOptions{
            .applicationName = "Fence",
            .applicationVersion = SERENITY_MAKE_API_VERSION(0, 1, 0, 0) });
    Adapter *discreteGPUAdapter = instance.selectAdapter(AdapterDeviceType::Default);
    Device device = discreteGPUAdapter->createDevice();

    TEST_CASE("Construction")
    {
        SUBCASE("A default constructed Fence is invalid")
        {
            // GIVEN
            Fence s;
            // THEN
            REQUIRE(!s.isValid());
        }

        SUBCASE("A constructed Fence from a Vulkan API")
        {
            // GIVEN
            const FenceOptions fenceOptions{};

            // WHEN
            Fence s = device.createFence(fenceOptions);

            // THEN
            CHECK(s.isValid());
        }
    }

    TEST_CASE("Destruction")
    {
        // GIVEN
        const FenceOptions fenceOptions{};

        Handle<Fence_t> fenceHandle;

        SUBCASE("Going Out Of Scope")
        {
            {
                // WHEN
                Fence s = device.createFence(fenceOptions);
                fenceHandle = s.handle();

                // THEN
                CHECK(s.isValid());
                CHECK(fenceHandle.isValid());
                CHECK(api->resourceManager()->getFence(fenceHandle) != nullptr);
            }

            // THEN
            CHECK(api->resourceManager()->getFence(fenceHandle) == nullptr);
        }

        SUBCASE("Move assigment")
        {
            // WHEN
            Fence s = device.createFence(fenceOptions);
            fenceHandle = s.handle();

            // THEN
            CHECK(s.isValid());
            CHECK(fenceHandle.isValid());
            CHECK(api->resourceManager()->getFence(fenceHandle) != nullptr);

            // WHEN
            s = {};

            // THEN
            CHECK(api->resourceManager()->getFence(fenceHandle) == nullptr);
        }
    }

    TEST_CASE("Comparison")
    {
        SUBCASE("Compare default contructed Fences")
        {
            // GIVEN
            Fence a;
            Fence b;

            // THEN
            CHECK(a == b);
        }

        SUBCASE("Compare device created Fences")
        {
            // GIVEN
            const FenceOptions fenceOptions{};

            // WHEN
            Fence a = device.createFence(fenceOptions);
            Fence b = device.createFence(fenceOptions);

            // THEN
            CHECK(a != b);
        }
    }
}
