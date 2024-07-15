/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDGpu/command_recorder.h>
#include <KDGpu/buffer.h>
#include <KDGpu/buffer_options.h>
#include <KDGpu/device.h>
#include <KDGpu/instance.h>
#include <KDGpu/acceleration_structure.h>
#include <KDGpu/acceleration_structure_options.h>
#include <KDGpu/vulkan/vulkan_graphics_api.h>

#include <cstring>
#include <type_traits>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

using namespace KDGpu;

namespace {
inline std::string assetPath()
{
#if defined(KDGPU_ASSET_PATH)
    return KDGPU_ASSET_PATH;
#else
    return "";
#endif
}

struct Vertex {
    float x, y, z;
};

} // namespace

TEST_SUITE("AccelerationStructure")
{
    std::unique_ptr<GraphicsApi> api = std::make_unique<VulkanGraphicsApi>();
    Instance instance = api->createInstance(InstanceOptions{
            .applicationName = "AccelerationStructure",
            .applicationVersion = KDGPU_MAKE_API_VERSION(0, 1, 0, 0) });

    Adapter *discreteGPUAdapter = instance.selectAdapter(AdapterDeviceType::Default);
    Device device = discreteGPUAdapter->createDevice(DeviceOptions{
            .requestedFeatures = discreteGPUAdapter->features(),
    });
    const bool supportsRayTracing = discreteGPUAdapter->features().accelerationStructures;

    TEST_CASE("Construction" * doctest::skip(!supportsRayTracing))
    {
        SUBCASE("A default constructed Acceleration is invalid")
        {
            // GIVEN
            AccelerationStructure accelerationStructure;
            // THEN
            REQUIRE(!accelerationStructure.isValid());
        }

        SUBCASE("BottomLevel Acceleration Structure from AABB")
        {
            // WHEN
            const AccelerationStructureOptions accelerationStructureOptions = {
                .type = AccelerationStructureType::BottomLevel,
                .geometryTypesAndCount = {
                        {
                                .geometry = AccelerationStructureGeometryAabbsData{
                                        .data = {}, // actual data is not needed for the creation
                                        .stride = sizeof(VkAabbPositionsKHR),
                                },
                        },
                },
            };

            AccelerationStructure accelerationStructure = device.createAccelerationStructure(accelerationStructureOptions);

            // THEN
            CHECK(accelerationStructure.isValid());
        }

        SUBCASE("BottomLevel Acceleration Structure from TriangleData")
        {
            // WHEN
            const AccelerationStructureOptions accelerationStructureOptions = {
                .type = AccelerationStructureType::BottomLevel,
                .geometryTypesAndCount = {
                        {
                                .geometry = AccelerationStructureGeometryTrianglesData{
                                        .vertexFormat = Format::R32G32B32_SFLOAT,
                                        .vertexData = {}, // actual data is not needed for the creation
                                        .vertexStride = sizeof(Vertex),
                                        .maxVertex = 5,
                                },
                                .maxPrimitiveCount = 1,
                        },
                },
            };

            AccelerationStructure accelerationStructure = device.createAccelerationStructure(accelerationStructureOptions);

            // THEN
            CHECK(accelerationStructure.isValid());
        }
    }

    TEST_CASE("Destruction" * doctest::skip(!supportsRayTracing))
    {
        const AccelerationStructureOptions accelerationStructureOptions = {
            .type = AccelerationStructureType::BottomLevel,
            .geometryTypesAndCount = {
                    {
                            .geometry = AccelerationStructureGeometryAabbsData{
                                    .data = {}, // actual data is not needed for the creation
                                    .stride = sizeof(VkAabbPositionsKHR),
                            },
                    },
            },
        };

        Handle<AccelerationStructure_t> asHandle;

        SUBCASE("Going Out of Scope")
        {
            {
                // WHEN
                AccelerationStructure accelerationStructure = device.createAccelerationStructure(accelerationStructureOptions);
                asHandle = accelerationStructure.handle();

                // THEN
                CHECK(accelerationStructure.isValid());
                CHECK(asHandle.isValid());
                CHECK(api->resourceManager()->getAccelerationStructure(asHandle) != nullptr);
            }

            // THEN
            CHECK(api->resourceManager()->getAccelerationStructure(asHandle) == nullptr);
        }

        SUBCASE("Move assignment")
        {
            // WHEN
            AccelerationStructure accelerationStructure = device.createAccelerationStructure(accelerationStructureOptions);
            asHandle = accelerationStructure.handle();

            // THEN
            CHECK(accelerationStructure.isValid());
            CHECK(asHandle.isValid());
            CHECK(api->resourceManager()->getAccelerationStructure(asHandle) != nullptr);

            // WHEN
            accelerationStructure = {};

            // THEN
            CHECK(api->resourceManager()->getAccelerationStructure(asHandle) == nullptr);
        }
    }

    TEST_CASE("Build Acceleration Structures" * doctest::skip(!supportsRayTracing))
    {
        Queue graphicsQueue = device.queues()[0];

        SUBCASE("Build BottomLevel Acceleration Structure from AABB")
        {
            // GIVEN
            Buffer aabbBuffer = device.createBuffer(BufferOptions{
                    .size = sizeof(VkAabbPositionsKHR),
                    .usage = BufferUsageFlagBits::TransferSrcBit | BufferUsageFlagBits::TransferDstBit | BufferUsageFlagBits::AccelerationStructureBuildInputReadOnlyBit | BufferUsageFlagBits::ShaderDeviceAddressBit,
                    .memoryUsage = MemoryUsage::CpuToGpu,
            });

            const AccelerationStructureGeometryAabbsData aabbGeometry{
                .data = aabbBuffer,
                .stride = sizeof(VkAabbPositionsKHR),
            };

            AccelerationStructure accelerationStructure = device.createAccelerationStructure(AccelerationStructureOptions{
                    .type = AccelerationStructureType::BottomLevel,
                    .geometryTypesAndCount = {
                            {
                                    .geometry = aabbGeometry,
                            },
                    },
            });

            // WHEN
            {
                const VkAabbPositionsKHR aabb{
                    .minX = -1.0f,
                    .minY = -1.0f,
                    .minZ = -1.0f,
                    .maxX = 1.0f,
                    .maxY = 1.0f,
                    .maxZ = 1.0f,
                };

                std::memcpy(aabbBuffer.map(), &aabb, sizeof(aabb));
                aabbBuffer.unmap();

                CommandRecorder c = device.createCommandRecorder();
                c.buildAccelerationStructures(BuildAccelerationStructureOptions{
                        .buildGeometryInfos = {
                                {
                                        .geometries = { aabbGeometry },
                                        .sourceStructure = {},
                                        .destinationStructure = accelerationStructure,
                                        .buildRangeInfos = {
                                                {
                                                        .primitiveCount = 1,
                                                        .primitiveOffset = 0,
                                                        .firstVertex = 0,
                                                        .transformOffset = 0,
                                                },
                                        },
                                },
                        },
                });

                auto commandBuffer = c.finish();
                graphicsQueue.submit(SubmitOptions{
                        .commandBuffers = { commandBuffer },
                });
                device.waitUntilIdle();
            }

            // THEN -> Shouldn't log validation errors
        }

        SUBCASE("Build BottomLevel Acceleration Structure from TriangleData")
        {
            // GIVEN
            Buffer vertexBuffer = device.createBuffer(BufferOptions{
                    .size = 6 * sizeof(Vertex),
                    .usage = BufferUsageFlagBits::TransferSrcBit | BufferUsageFlagBits::TransferDstBit | BufferUsageFlagBits::VertexBufferBit | BufferUsageFlagBits::AccelerationStructureBuildInputReadOnlyBit | BufferUsageFlagBits::ShaderDeviceAddressBit,
                    .memoryUsage = MemoryUsage::CpuToGpu,
            });

            const AccelerationStructureGeometryTrianglesData triangleDataGeometry{
                .vertexFormat = Format::R32G32B32_SFLOAT,
                .vertexData = vertexBuffer,
                .vertexStride = sizeof(Vertex),
                .maxVertex = 5,
            };

            AccelerationStructure accelerationStructure = device.createAccelerationStructure(AccelerationStructureOptions{
                    .type = AccelerationStructureType::BottomLevel,
                    .geometryTypesAndCount = {
                            {
                                    .geometry = triangleDataGeometry,
                            },
                    },
            });

            // WHEN
            {
                const std::vector<Vertex> vertices{
                    { -1.0f, 1.0f, 0.5f },
                    { -1.0f, -1.0f, 0.5f },
                    { 1.0f, -1.0f, 0.5f },
                    { 1.0f, -1.0f, 0.5f },
                    { 1.0f, 1.0f, 0.5f },
                    { -1.0f, 1.0f, 0.5f },
                };
                std::memcpy(vertexBuffer.map(), vertices.data(), vertices.size() * sizeof(Vertex));
                vertexBuffer.unmap();

                CommandRecorder c = device.createCommandRecorder();
                c.buildAccelerationStructures(BuildAccelerationStructureOptions{
                        .buildGeometryInfos = {
                                {
                                        .geometries = { triangleDataGeometry },
                                        .sourceStructure = {},
                                        .destinationStructure = accelerationStructure,
                                        .buildRangeInfos = {
                                                {
                                                        .primitiveCount = 1,
                                                        .primitiveOffset = 0,
                                                        .firstVertex = 0,
                                                        .transformOffset = 0,
                                                },
                                        },
                                },
                        },
                });

                auto commandBuffer = c.finish();
                graphicsQueue.submit(SubmitOptions{
                        .commandBuffers = { commandBuffer },
                });
                device.waitUntilIdle();
            }

            // THEN -> Shouldn't log validation errors
        }

        SUBCASE("Build TopLevel Acceleration Structure")
        {
            Buffer aabbBuffer = device.createBuffer(BufferOptions{
                    .size = sizeof(VkAabbPositionsKHR),
                    .usage = BufferUsageFlagBits::TransferSrcBit | BufferUsageFlagBits::TransferDstBit | BufferUsageFlagBits::AccelerationStructureBuildInputReadOnlyBit | BufferUsageFlagBits::ShaderDeviceAddressBit,
                    .memoryUsage = MemoryUsage::CpuToGpu,
            });

            const AccelerationStructureGeometryAabbsData aabbGeometry{
                .data = aabbBuffer,
                .stride = sizeof(VkAabbPositionsKHR),
            };

            AccelerationStructure bottomLevelAs = device.createAccelerationStructure(AccelerationStructureOptions{
                    .label = "BottomLevelAS",
                    .type = AccelerationStructureType::BottomLevel,
                    .geometryTypesAndCount = {
                            {
                                    .geometry = aabbGeometry,
                            },
                    },
            });

            const AccelerationStructureGeometryInstancesData geometryInstance{
                .data = {
                        AccelerationStructureGeometryInstance{ .accelerationStructure = bottomLevelAs },
                },
            };

            AccelerationStructure topLevelAs = device.createAccelerationStructure(AccelerationStructureOptions{
                    .label = "TopLevelAS",
                    .type = AccelerationStructureType::TopLevel,
                    .geometryTypesAndCount = {
                            {
                                    .geometry = geometryInstance,
                            },
                    },
            });

            // WHEN
            {
                const VkAabbPositionsKHR aabb{
                    .minX = -1.0f,
                    .minY = -1.0f,
                    .minZ = -1.0f,
                    .maxX = 1.0f,
                    .maxY = 1.0f,
                    .maxZ = 1.0f,
                };

                std::memcpy(aabbBuffer.map(), &aabb, sizeof(aabb));
                aabbBuffer.unmap();

                CommandRecorder c = device.createCommandRecorder();
                c.buildAccelerationStructures(BuildAccelerationStructureOptions{
                        .buildGeometryInfos = {
                                {
                                        .geometries = { aabbGeometry },
                                        .sourceStructure = {},
                                        .destinationStructure = bottomLevelAs,
                                        .buildRangeInfos = {
                                                {
                                                        .primitiveCount = 1,
                                                        .primitiveOffset = 0,
                                                        .firstVertex = 0,
                                                        .transformOffset = 0,
                                                },
                                        },
                                },
                        },
                });

                c.buildAccelerationStructures(BuildAccelerationStructureOptions{
                        .buildGeometryInfos = {
                                {
                                        .geometries = { geometryInstance },
                                        .sourceStructure = bottomLevelAs,
                                        .destinationStructure = topLevelAs,
                                        .buildRangeInfos = {
                                                {
                                                        .primitiveCount = 1,
                                                        .primitiveOffset = 0,
                                                        .firstVertex = 0,
                                                        .transformOffset = 0,
                                                },
                                        },
                                },
                        },
                });

                auto commandBuffer = c.finish();
                graphicsQueue.submit(SubmitOptions{
                        .commandBuffers = { commandBuffer },
                });
                device.waitUntilIdle();
            }

            // THEN -> Shouldn't log validation errors
        }
    }
}
