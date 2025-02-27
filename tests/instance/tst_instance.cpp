/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDGpu/device.h>
#include <KDGpu/instance.h>
#include <KDGpu/vulkan/vulkan_graphics_api.h>

#include <KDGpuKDGui/view.h>

#include <KDGui/gui_application.h>

#include <set>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

using namespace KDGui;
using namespace KDGpu;
using namespace KDGpuKDGui;

TEST_SUITE("Instance")
{
    TEST_CASE("Vulkan")
    {
        // GIVEN
        std::unique_ptr<GraphicsApi> api = std::make_unique<VulkanGraphicsApi>();

        // WHEN
        Instance instance = api->createInstance(InstanceOptions{
                .applicationName = "instance",
                .applicationVersion = KDGPU_MAKE_API_VERSION(0, 1, 0, 0) });

        // THEN
        CHECK(instance.isValid());

        SUBCASE("Has Adapters")
        {
            // WHEN
            const auto adapters = instance.adapters();

            // THEN
            CHECK(!adapters.empty());
        }

        SUBCASE("Has AdapterGroups")
        {
            // WHEN
            const auto adapterGroups = instance.adapterGroups();

            // THEN
            CHECK(!adapterGroups.empty());
        }

        SUBCASE("Can query instance extensions")
        {
            // WHEN
            const auto extensions = instance.extensions();

            // THEN
            // In theory this could be empty if a driver really doesn't have any extensions
            CHECK(!extensions.empty());
        }

        SUBCASE("Can create Device")
        {
            // WHEN
            Adapter *discreteGPUAdapter = instance.selectAdapter(AdapterDeviceType::Default);

            // THEN
            CHECK(discreteGPUAdapter);
            CHECK(discreteGPUAdapter->isValid());

            // WHEN
            Device device = discreteGPUAdapter->createDevice();

            // THEN
            CHECK(device.isValid());
        }

        SUBCASE("Can create Device with user extension")
        {
            // WHEN
            Adapter *discreteGPUAdapter = instance.selectAdapter(AdapterDeviceType::Default);

            // THEN
            CHECK(discreteGPUAdapter);
            CHECK(discreteGPUAdapter->isValid());

            // WHEN
            using namespace std::string_literals;

            DeviceOptions options{
                .extensions = { "VK_KHR_dynamic_rendering"s },
                .requestedFeatures = discreteGPUAdapter->features(),
            };
            Device device = discreteGPUAdapter->createDevice(options);

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
            CHECK(aAndD.adapter);
            CHECK(aAndD.adapter->isValid());
            CHECK(aAndD.device.isValid());
        }

        SUBCASE("Can create an Instance from an existing vkInstance")
        {
            // GIVEN
            VkApplicationInfo appInfo = {};
            appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            appInfo.pApplicationName = "createInstanceFromExistingVkInstance";
            appInfo.applicationVersion = 0;
            appInfo.pEngineName = "KDGpu";
            appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
            appInfo.apiVersion = VK_API_VERSION_1_2;

            VkInstanceCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            createInfo.pApplicationInfo = &appInfo;

#if defined(KDGPU_PLATFORM_MACOS)
            // On macOS we need to enable the VK_KHR_PORTABILITY_subset instance extension so that
            // the MoltenVK driver is allowed to be used even though it is technically non-conformant
            // at present. Also see vulkan_config.h. For more detail see the
            // Encountered VK_ERROR_INCOMPATIBLE_DRIVER section of
            // https://vulkan.lunarg.com/doc/sdk/1.3.216.0/mac/getting_started.html
            createInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
            std::vector<const char *> extensions;
            extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
            extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
            createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
            createInfo.ppEnabledExtensionNames = extensions.data();
#endif

            VkInstance vkInstance = VK_NULL_HANDLE;

            // WHEN
            {
                const VkResult result = vkCreateInstance(&createInfo, nullptr, &vkInstance);
                CHECK(result == VK_SUCCESS);
                Instance instanceFromExistingVkInstance = static_cast<VulkanGraphicsApi *>(api.get())->createInstanceFromExistingVkInstance(vkInstance);

                // THEN
                CHECK(instanceFromExistingVkInstance.isValid());

                // WHEN
                CHECK(instanceFromExistingVkInstance.adapters().size() > 0);
            }

            // THEN -> Shouldn't have crashed when Instance went out of scope and was destroyed
            vkDestroyInstance(vkInstance, nullptr);
        }
    }

    TEST_CASE("Destruction")
    {
        // GIVEN
        std::unique_ptr<GraphicsApi> api = std::make_unique<VulkanGraphicsApi>();

        const InstanceOptions options{
            .applicationName = "instance",
            .applicationVersion = KDGPU_MAKE_API_VERSION(0, 1, 0, 0)
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

        SUBCASE("Move assignment")
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

    TEST_CASE("Move")
    {
        // GIVEN
        std::unique_ptr<GraphicsApi> api = std::make_unique<VulkanGraphicsApi>();

        const InstanceOptions options{
            .applicationName = "instance",
            .applicationVersion = KDGPU_MAKE_API_VERSION(0, 1, 0, 0)
        };

        SUBCASE("Adapter Pointers remain valid on Move")
        {
            // GIVEN
            Instance instance = api->createInstance(options);
            Handle<Instance_t> handle = instance.handle();

            const std::vector<Adapter *> adapterPtrs = instance.adapters();

            // WHEN
            Instance instance2 = std::move(instance);
            Handle<Instance_t> handle2 = instance2.handle();

            // THEN
            CHECK(!instance.isValid());
            const std::vector<Adapter *> adapterPtrsFromMoved = instance2.adapters();

            CHECK(adapterPtrs == adapterPtrsFromMoved);
            CHECK(handle == handle2);
        }
    }
}
