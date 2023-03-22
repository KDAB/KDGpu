#include <toy_renderer/buffer.h>
#include <toy_renderer/buffer_options.h>
#include <toy_renderer/device.h>
#include <toy_renderer/instance.h>
#include <toy_renderer/vulkan/vulkan_graphics_api.h>

#include <set>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

using namespace ToyRenderer;

TEST_SUITE("Buffer")
{
    std::unique_ptr<GraphicsApi> api = std::make_unique<VulkanGraphicsApi>();
    Instance instance = api->createInstance(InstanceOptions{
            .applicationName = "buffer",
            .applicationVersion = SERENITY_MAKE_API_VERSION(0, 1, 0, 0) });
    Adapter *discreteGPUAdapter = instance.selectAdapter(AdapterDeviceType::DiscreteGpu);
    // CHECK(discreteGPUAdapter.isValid());
    Device device = discreteGPUAdapter->createDevice();
    // CHECK(device.isValid());

    TEST_CASE("Construction")
    {
        SUBCASE("A default constructed Buffer is invalid")
        {
            // GIVEN
            Buffer buffer;
            // THEN
            REQUIRE(!buffer.isValid());
        }

        SUBCASE("A constructed Buffer from a Vulkan API with no initial data")
        {
            // GIVEN
            const BufferOptions bufferOptions = {
                .size = 4 * sizeof(float),
                .usage = BufferUsageFlagBits::VertexBufferBit,
                .memoryUsage = MemoryUsage::CpuToGpu
            };

            // WHEN
            Buffer b = device.createBuffer(bufferOptions);

            // THEN
            CHECK(b.isValid());
        }

        SUBCASE("A constructed Buffer from a Vulkan API with initial data")
        {
            // GIVEN
            const BufferOptions bufferOptions = {
                .size = 4 * sizeof(float),
                .usage = BufferUsageFlagBits::VertexBufferBit,
                .memoryUsage = MemoryUsage::CpuToGpu
            };

            const std::vector<float> vertexData = {
                1.0f, -1.0f, 0.0f, 1.0f
            };

            // WHEN
            Buffer b = device.createBuffer(bufferOptions, vertexData.data());

            // THEN
            CHECK(b.isValid());
        }
    }

    TEST_CASE("Destruction")
    {
        // GIVEN
        const BufferOptions bufferOptions = {
            .size = 4 * sizeof(float),
            .usage = BufferUsageFlagBits::VertexBufferBit,
            .memoryUsage = MemoryUsage::CpuToGpu
        };

        const std::vector<float> vertexData = {
            1.0f, -1.0f, 0.0f, 1.0f
        };

        Handle<Buffer_t> bufferHandle;

        SUBCASE("Going Out Of Scope")
        {
            {
                // WHEN
                Buffer b = device.createBuffer(bufferOptions, vertexData.data());
                bufferHandle = b.handle();

                // THEN
                CHECK(b.isValid());
                CHECK(bufferHandle.isValid());
                CHECK(api->resourceManager()->getBuffer(bufferHandle) != nullptr);
            }

            // THEN
            CHECK(api->resourceManager()->getBuffer(bufferHandle) == nullptr);
        }

        SUBCASE("Move assigment")
        {
            // WHEN
            Buffer b = device.createBuffer(bufferOptions, vertexData.data());
            bufferHandle = b.handle();

            // THEN
            CHECK(b.isValid());
            CHECK(bufferHandle.isValid());
            CHECK(api->resourceManager()->getBuffer(bufferHandle) != nullptr);

            // WHEN
            b = {};

            // THEN
            CHECK(api->resourceManager()->getBuffer(bufferHandle) == nullptr);
        }
    }

    TEST_CASE("Map/Unmap")
    {
        SUBCASE("Invalid Buffer")
        {
            // GIVEN
            Buffer buffer;

            // WHEN
            void *m = buffer.map();

            // THEN
            CHECK(m == nullptr);

            // WHEN
            buffer.unmap();

            // THEN -> Shouldn't crash
        }

        SUBCASE("Valid Buffer")
        {
            // GIVEN
            const BufferOptions bufferOptions = {
                .size = 4 * sizeof(float),
                .usage = BufferUsageFlagBits::VertexBufferBit,
                .memoryUsage = MemoryUsage::CpuToGpu
            };

            const std::vector<float> vertexData = {
                1.0f, -1.0f, 0.0f, 1.0f
            };

            // WHEN
            Buffer b = device.createBuffer(bufferOptions, vertexData.data());

            // THEN
            CHECK(b.isValid());

            // WHEN
            const float *rawData = reinterpret_cast<const float *>(b.map());

            // THEN
            CHECK(rawData != nullptr);
            CHECK(rawData[0] == vertexData[0]);
            CHECK(rawData[1] == vertexData[1]);
            CHECK(rawData[2] == vertexData[2]);
            CHECK(rawData[3] == vertexData[3]);

            // WHEN
            b.unmap();

            // THEN -> It's all good
        }
    }

    TEST_CASE("Comparison")
    {
        SUBCASE("Compare default contructed Buffers")
        {
            // GIVEN
            Buffer a;
            Buffer b;

            // THEN
            CHECK(a == b);
        }

        SUBCASE("Compare device create buffers")
        {
            // GIVEN
            const BufferOptions bufferOptions = {
                .size = 4 * sizeof(float),
                .usage = BufferUsageFlagBits::VertexBufferBit,
                .memoryUsage = MemoryUsage::CpuToGpu
            };

            // WHEN
            Buffer b = device.createBuffer(bufferOptions);
            Buffer a = device.createBuffer(bufferOptions);

            // THEN
            CHECK(a != b);
        }
    }
}
