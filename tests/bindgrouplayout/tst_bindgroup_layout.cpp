#include <toy_renderer/bind_group.h>
#include <toy_renderer/bind_group_options.h>
#include <toy_renderer/bind_group_layout.h>
#include <toy_renderer/bind_group_layout_options.h>
#include <toy_renderer/bind_group_description.h>
#include <toy_renderer/buffer_options.h>
#include <toy_renderer/buffer.h>
#include <toy_renderer/device.h>
#include <toy_renderer/instance.h>
#include <toy_renderer/vulkan/vulkan_graphics_api.h>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

using namespace ToyRenderer;

TEST_SUITE("BindGroupLayout")
{
    std::unique_ptr<GraphicsApi> api = std::make_unique<VulkanGraphicsApi>();
    Instance instance = api->createInstance(InstanceOptions{
            .applicationName = "BindGroupLayout",
            .applicationVersion = SERENITY_MAKE_API_VERSION(0, 1, 0, 0) });
    Adapter *discreteGPUAdapter = instance.selectAdapter(AdapterDeviceType::Default);
    Device device = discreteGPUAdapter->createDevice();

    TEST_CASE("Construction")
    {
        SUBCASE("A default constructed BindGroupLayout is invalid")
        {
            // GIVEN
            BindGroupLayout t;
            // THEN
            REQUIRE(!t.isValid());
        }

        SUBCASE("A constructed BindGroupLayout from a Vulkan API")
        {
            // GIVEN
            const BindGroupLayoutOptions bindGroupLayoutOptions = {
                .bindings = { { // Camera uniforms
                                .binding = 0,
                                .count = 1,
                                .resourceType = ResourceBindingType::UniformBuffer,
                                .shaderStages = ShaderStageFlags(ShaderStageFlagBits::VertexBit) } }
            };

            const BindGroupLayout bindGroupLayout = device.createBindGroupLayout(bindGroupLayoutOptions);

            // THEN
            CHECK(bindGroupLayout.isValid());
        }
    }

    TEST_CASE("Destruction")
    {
        // GIVEN
        const BindGroupLayoutOptions bindGroupLayoutOptions = {
            .bindings = { { // Camera uniforms
                            .binding = 0,
                            .count = 1,
                            .resourceType = ResourceBindingType::UniformBuffer,
                            .shaderStages = ShaderStageFlags(ShaderStageFlagBits::VertexBit) } }
        };

        Handle<BindGroupLayout_t> bindGroupLayoutHandle;

        SUBCASE("Going Out Of Scope")
        {
            {
                // WHEN
                const BindGroupLayout bindGroupLayout = device.createBindGroupLayout(bindGroupLayoutOptions);
                bindGroupLayoutHandle = bindGroupLayout.handle();

                // THEN
                CHECK(bindGroupLayout.isValid());
                CHECK(bindGroupLayoutHandle.isValid());
                CHECK(api->resourceManager()->getBindGroupLayout(bindGroupLayoutHandle) != nullptr);
            }

            // THEN
            CHECK(api->resourceManager()->getBindGroupLayout(bindGroupLayoutHandle) == nullptr);
        }

        SUBCASE("Move assigment")
        {
            // WHEN
            BindGroupLayout bindGroupLayout = device.createBindGroupLayout(bindGroupLayoutOptions);
            bindGroupLayoutHandle = bindGroupLayout.handle();

            // THEN
            CHECK(bindGroupLayout.isValid());
            CHECK(bindGroupLayoutHandle.isValid());
            CHECK(api->resourceManager()->getBindGroupLayout(bindGroupLayoutHandle) != nullptr);

            // WHEN
            bindGroupLayout = {};

            // THEN
            CHECK(api->resourceManager()->getBindGroupLayout(bindGroupLayoutHandle) == nullptr);
        }
    }

    TEST_CASE("Comparison")
    {
        SUBCASE("Compare default contructed BindGroupLayouts")
        {
            // GIVEN
            BindGroupLayout a;
            BindGroupLayout b;

            // THEN
            CHECK(a == b);
        }

        SUBCASE("Compare device created BindGroupLayouts")
        {
            // GIVEN
            const BindGroupLayoutOptions bindGroupLayoutOptions = {
                .bindings = { { // Camera uniforms
                                .binding = 0,
                                .count = 1,
                                .resourceType = ResourceBindingType::UniformBuffer,
                                .shaderStages = ShaderStageFlags(ShaderStageFlagBits::VertexBit) } }
            };

            // WHEN
            BindGroupLayout a = device.createBindGroupLayout(bindGroupLayoutOptions);
            BindGroupLayout b = device.createBindGroupLayout(bindGroupLayoutOptions);

            // THEN
            CHECK(a != b);
        }
    }
}
