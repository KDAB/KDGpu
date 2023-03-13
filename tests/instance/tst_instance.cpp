#include <toy_renderer/device.h>
#include <toy_renderer/instance.h>
#include <toy_renderer/vulkan/vulkan_graphics_api.h>

#include <toy_renderer_kdgui/view.h>

#include <KDGui/gui_application.h>

#include <set>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

using namespace KDGui;
using namespace ToyRenderer;
using namespace ToyRendererKDGui;

TEST_SUITE("Instance")
{
    TEST_CASE("Vulkan")
    {
        // GIVEN
        std::unique_ptr<GraphicsApi> api = std::make_unique<VulkanGraphicsApi>();

        // WHEN
        Instance instance = api->createInstance(InstanceOptions{
                .applicationName = "instance",
                .applicationVersion = SERENITY_MAKE_API_VERSION(0, 1, 0, 0) });

        // THEN
        CHECK(instance.isValid());

        SUBCASE("Has Adapters")
        {
            // WHEN
            const auto adapters = instance.adapters();

            // THEN
            CHECK(!adapters.empty());
        }

        SUBCASE("Can create Device")
        {
            // WHEN
            Adapter discreteGPUAdapter = instance.selectAdapter(AdapterDeviceType::DiscreteGpu).value_or(Adapter());

            // THEN
            CHECK(discreteGPUAdapter.isValid());

            // WHEN
            Device device = discreteGPUAdapter.createDevice();

            // THEN
            CHECK(device.isValid());
        }

        SUBCASE("Can create Surface")
        {
            // GIVEN
            GuiApplication app;
            View v;

            // WHEN
            const SurfaceOptions surfaceOptions = View::surfaceOptions(&v);
            Surface s = instance.createSurface(surfaceOptions);

            // THEN
            CHECK(s.isValid());
        }

        SUBCASE("Can create Default Device and Adapter")
        {
            // GIVEN
            GuiApplication app;
            View v;
            const SurfaceOptions surfaceOptions = View::surfaceOptions(&v);
            Surface s = instance.createSurface(surfaceOptions);

            // WHEN
            AdapterAndDevice aAndD = instance.createDefaultDevice(s);

            // THEN
            CHECK(aAndD.adapter.isValid());
            CHECK(aAndD.device.isValid());
        }
    }

    TEST_CASE("Destruction")
    {
        // GIVEN
        std::unique_ptr<GraphicsApi> api = std::make_unique<VulkanGraphicsApi>();

        const InstanceOptions options{
            .applicationName = "instance",
            .applicationVersion = SERENITY_MAKE_API_VERSION(0, 1, 0, 0)
        };

        Handle<Instance_t> handle;

        SUBCASE("Going Out Of Scope")
        {
            {
                // WHEN
                Instance instance = api->createInstance(options);
                handle = instance.handle();

                // THEN
                CHECK(instance.isValid());
                CHECK(handle.isValid());
                CHECK(api->resourceManager()->getInstance(handle) != nullptr);
            }

            // THEN
            CHECK(api->resourceManager()->getInstance(handle) == nullptr);
        }

        SUBCASE("Move assigment")
        {
            // WHEN
            Instance instance = api->createInstance(options);
            handle = instance.handle();

            // THEN
            CHECK(instance.isValid());
            CHECK(handle.isValid());
            CHECK(api->resourceManager()->getInstance(handle) != nullptr);

            // WHEN
            instance = {};

            // THEN
            CHECK(api->resourceManager()->getInstance(handle) == nullptr);
        }
    }
}
