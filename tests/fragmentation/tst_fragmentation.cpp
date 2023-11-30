/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDGpu/config.h>
#include <KDGpu/texture.h>
#include <KDGpu/texture_options.h>
#include <KDGpu/device.h>
#include <KDGpu/instance.h>
#include <KDGpu/vulkan/vulkan_graphics_api.h>

#include <queue>
#include <cstdlib>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

#include <cuda.h>
#include <cuda_runtime.h>

using namespace KDGpu;

namespace {

void checkCudaErrors()
{
    const cudaError_t err = cudaGetLastError();
    if (static_cast<int>(err) != static_cast<int>(CUDA_SUCCESS)) {
        SPDLOG_ERROR("{}({}): {}", cudaGetErrorName(cudaError_t(err)), int(err),
                     cudaGetErrorString(err));
    }
}

class CudaTexture
{
public:
    CudaTexture(Texture &&t)
        : texture(std::move(t))
    {
        importExternalMemory();
    }

    ~CudaTexture()
    {
        if (texture.isValid())
            destroyExternalMemory();
    }

    CudaTexture(CudaTexture &&cT)
    {
        texture = std::move(cT.texture);
        cudaExtMemImageBuffer_ = std::move(cT.cudaExtMemImageBuffer_);

        cT.cudaExtMemImageBuffer_ = nullptr;
    }

    CudaTexture &operator=(CudaTexture &&cT)
    {
        if (this != &cT) {
            texture = std::move(cT.texture);
            cudaExtMemImageBuffer_ = std::move(cT.cudaExtMemImageBuffer_);

            cT.cudaExtMemImageBuffer_ = nullptr;
        }
        return *this;
    }

    CudaTexture(const Texture &) = delete;
    CudaTexture &operator=(const Texture &) = delete;

private:
    void importExternalMemory()
    {
        const KDGpu::MemoryHandle &h = texture.externalMemoryHandle();

        const cudaExternalMemoryHandleDesc cudaExtMemHandleDesc = {
#if defined(KDGPU_PLATFORM_LINUX)
            .type = cudaExternalMemoryHandleTypeOpaqueFd,
            .handle = { .fd = std::get<int>(h.handle) },
#elif defined(KDGPU_PLATFORM_WIN32)
            .type = cudaExternalMemoryHandleTypeOpaqueWin32,
            .handle = { .win32{ .handle = std::get<HANDLE>(h.handle) } },
#endif
            .size = h.allocationSize,
            .flags = 0,
        };

        const cudaError_t result =
                cudaImportExternalMemory(&cudaExtMemImageBuffer_, &cudaExtMemHandleDesc);
        if (static_cast<int>(result) != static_cast<int>(CUDA_SUCCESS)) {
            SPDLOG_ERROR("Failed to Import External Memory");
            checkCudaErrors();
        }
    }

    void destroyExternalMemory()
    {
        // Destroy imported shared memory
        cudaDestroyExternalMemory(cudaExtMemImageBuffer_);
        checkCudaErrors();
        cudaExtMemImageBuffer_ = nullptr;
    }

    Texture texture;
    cudaExternalMemory_t cudaExtMemImageBuffer_{ nullptr };
};

} // namespace

TEST_SUITE("Fragmentation")
{
    std::unique_ptr<VulkanGraphicsApi> api = std::make_unique<VulkanGraphicsApi>();
    Instance instance = api->createInstance(InstanceOptions{
            .applicationName = "buffer",
            .applicationVersion = KDGPU_MAKE_API_VERSION(0, 1, 0, 0) });
    Adapter *discreteGPUAdapter = instance.selectAdapter(AdapterDeviceType::Default);
    Device device = discreteGPUAdapter->createDevice();

    TEST_CASE("Allocates Many Textures of Random Size")
    {
        const std::vector<KDGpu::Extent3D> availableExtents = {
            { 512, 512, 512 },
            { 32, 64, 128 },
            { 128, 32, 128 },
            { 64, 32, 128 },
            { 64, 64, 64 },
        };

        const size_t allocationCount = 1024 * 10;
        const size_t keepAliveTextures = 5;

        TextureOptions baseOptions = {
            .type = TextureType::TextureType3D,
            .format = Format::R8G8B8A8_SNORM,
            .mipLevels = 1,
            .usage = TextureUsageFlagBits::SampledBit,
            .memoryUsage = MemoryUsage::GpuOnly,
        };

        SUBCASE("Regular 3D Textures")
        {
            std::queue<Texture> textureQueue;
            for (size_t i = 0; i < allocationCount; ++i) {

                const size_t randomExtentIdx = std::rand() % availableExtents.size();
                baseOptions.extent = availableExtents[randomExtentIdx];

                Texture t = device.createTexture(baseOptions);

                textureQueue.push(std::move(t));

                if (textureQueue.size() > keepAliveTextures)
                    textureQueue.pop();

                // Periodically print out allocation stats to check for fragmentation
                // if (i % 256)
                //     SPDLOG_WARN("{}", api->getMemoryStats(device));
            }
        }

#if defined(KDGPU_PLATFORM_LINUX)
        SUBCASE("3D Textures with OpaqueFD")
        {
            baseOptions.externalMemoryHandleType = ExternalMemoryHandleTypeFlagBits::OpaqueFD;
            std::queue<CudaTexture> textureQueue;

            for (size_t i = 0; i < allocationCount; ++i) {

                const size_t randomExtentIdx = rand() % availableExtents.size();
                baseOptions.extent = availableExtents[randomExtentIdx];

                Texture t = device.createTexture(baseOptions);

                CudaTexture cudaTex(std::move(t));

                textureQueue.push(std::move(cudaTex));

                if (textureQueue.size() > keepAliveTextures)
                    textureQueue.pop();
            }
        }
#elif defined(KDGPU_PLATFORM_WIN32)
        SUBCASE("3D Textures with OpaqueWin32")
        {
            baseOptions.externalMemoryHandleType = ExternalMemoryHandleTypeFlagBits::OpaqueWin32;
            std::queue<CudaTexture> textureQueue;

            for (size_t i = 0; i < allocationCount; ++i) {

                const size_t randomExtentIdx = rand() % availableExtents.size();
                baseOptions.extent = availableExtents[randomExtentIdx];

                Texture t = device.createTexture(baseOptions);

                textureQueue.push(CudaTexture(std::move(t)));

                if (textureQueue.size() > keepAliveTextures)
                    textureQueue.pop();
            }
        }
#endif
    }
}
