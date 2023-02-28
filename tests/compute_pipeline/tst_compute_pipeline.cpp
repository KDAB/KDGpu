#include <toy_renderer/compute_pipeline.h>
#include <toy_renderer/compute_pipeline_options.h>
#include <toy_renderer/device.h>
#include <toy_renderer/instance.h>
#include <toy_renderer/vulkan/vulkan_graphics_api.h>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

using namespace ToyRenderer;

namespace {
inline std::string assetPath()
{
#if defined(TOY_RENDERER_ASSET_PATH)
    return TOY_RENDERER_ASSET_PATH;
#else
    return "";
#endif
}
} // namespace

TEST_SUITE("ComputePipeline")
{
    std::unique_ptr<GraphicsApi> api = std::make_unique<VulkanGraphicsApi>();
    Instance instance = api->createInstance(InstanceOptions{
            .applicationName = "ComputePipeline",
            .applicationVersion = SERENITY_MAKE_API_VERSION(0, 1, 0, 0) });
    Adapter discreteGPUAdapter = instance.selectAdapter(AdapterDeviceType::DiscreteGpu).value_or(Adapter());
    Device device = discreteGPUAdapter.createDevice();

    const auto computeShaderPath = assetPath() + "/shaders/tests/compute_pipeline/empty_compute.comp.spv";
    auto computeShader = device.createShaderModule(ToyRenderer::readShaderFile(computeShaderPath));

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
            PipelineLayoutOptions pipelineLayoutOptions{};
            PipelineLayout pipelineLayout = device.createPipelineLayout(pipelineLayoutOptions);

            // clang-format off
            const ComputePipelineOptions computePipelineOptions {
                .layout = pipelineLayout,
                .shaderStage = ComputeShaderStage {
                    .shaderModule = computeShader.handle()
                }
            };
            // clang-format on

            // WHEN
            ComputePipeline c = device.createComputePipeline(computePipelineOptions);

            // THEN
            CHECK(c.isValid());
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
            PipelineLayoutOptions pipelineLayoutOptions{};
            PipelineLayout pipelineLayout = device.createPipelineLayout(pipelineLayoutOptions);

            // clang-format off
            const ComputePipelineOptions computePipelineOptions {
                .layout = pipelineLayout,
                .shaderStage = ComputeShaderStage {
                    .shaderModule = computeShader.handle()
                }
            };
            // clang-format on

            // WHEN
            ComputePipeline a = device.createComputePipeline(computePipelineOptions);
            ComputePipeline b = device.createComputePipeline(computePipelineOptions);

            // THEN
            CHECK(a != b);

            // WHEN
            a = b;

            // THEN
            CHECK(a == b);
        }
    }
}
