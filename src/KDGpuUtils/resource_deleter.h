/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpuUtils/kdgpuutils_export.h>

#include <KDGpu/buffer.h>
#include <KDGpu/bind_group.h>
#include <KDGpu/texture.h>
#include <KDGpu/texture_view.h>
#include <KDGpu/pipeline_layout.h>
#include <KDGpu/graphics_pipeline.h>
#include <KDGpu/compute_pipeline.h>
#include <KDGpu/raytracing_pipeline.h>
#include <KDGpu/acceleration_structure.h>
#include <KDGpu/raytracing_shader_binding_table.h>
#include <KDGpu/sampler.h>

#include <algorithm>
#include <vector>
#include <tuple>
#include <atomic>

namespace KDGpu {
class Device;
} // namespace KDGpu

namespace KDGpuUtils {

class ResourceDeleter;

template<typename... Resources>
class ResourcesHolder
{
public:
    using VectorTypes = std::tuple<std::vector<Resources>...>;

    template<typename Resource>
    std::vector<Resource> &get()
    {
        return getHelper<0, Resource>();
    }

    template<typename Resource>
    const std::vector<Resource> &get() const
    {
        return getHelper<0, Resource>();
    }

    template<typename Resource>
    void emplace_back(Resource &&r)
    {
        get<Resource>().emplace_back(r);
    }

private:
    VectorTypes m_vectors;

    template<size_t N, typename T>
    using isSame = std::is_same<T, typename std::tuple_element_t<N, VectorTypes>::value_type>;

    template<size_t N = 0, typename Resource, typename Tuple = VectorTypes>
    const std::vector<Resource> &getHelper() const
    {
        if constexpr (isSame<N, Resource>::value)
            return std::get<N>(m_vectors);
        else
            return getHelper<N + 1, Resource>();
    }

    template<size_t N = 0, typename Resource, typename Tuple = VectorTypes>
    std::vector<Resource> &getHelper()
    {
        if constexpr (isSame<N, Resource>::value)
            return std::get<N>(m_vectors);
        else
            return getHelper<N + 1, Resource>();
    }
};

class KDGPUUTILS_EXPORT ResourceDeleter
{
public:
    ResourceDeleter(KDGpu::Device *device, size_t maxFramesInFlight);
    ~ResourceDeleter();

    ResourceDeleter(ResourceDeleter const &other) = delete;
    ResourceDeleter &operator=(ResourceDeleter const &other) = delete;

    ResourceDeleter(ResourceDeleter &&other) = delete;
    ResourceDeleter &operator=(ResourceDeleter &&other) = delete;

    // Must be a unique frame number, always higher than the previous one
    void moveToNextFrame();
    uint64_t frameNumber() const noexcept { return m_frameNumber; }

    void derefFrameIndex(size_t frameIndex);

    template<typename Resource>
    void deleteLater(Resource &&r)
    {
        auto &bin = getBin();
        bin.resources.get<Resource>().emplace_back(std::move(r));
    }

    void deleteAll();

    struct FrameBin {
        explicit FrameBin(uint64_t _frameNumber, size_t _imageCount)
            : frameNumber{ _frameNumber }
            , frameReferences(_imageCount, true)
        {
        }

        bool canBeDestroyed() const noexcept
        {
            return std::all_of(
                    frameReferences.begin(),
                    frameReferences.end(),
                    [](bool b) { return b == false; });
        }

        uint64_t frameNumber{ 0 };
        // We use a vector and not a simpler counter
        std::vector<bool> frameReferences;
        ResourcesHolder<KDGpu::Buffer,
                        KDGpu::BindGroup,
                        KDGpu::Texture,
                        KDGpu::TextureView,
                        KDGpu::Sampler,
                        KDGpu::GraphicsPipeline,
                        KDGpu::ComputePipeline,
                        KDGpu::RayTracingPipeline,
                        KDGpu::PipelineLayout,
                        KDGpu::AccelerationStructure,
                        KDGpu::RayTracingShaderBindingTable>
                resources;

        // clang-format off
        template<size_t N = 0, typename Tuple = decltype(resources)::VectorTypes>
        typename std::enable_if<N == std::tuple_size<Tuple>::value, void>::type releaseResources(ResourceDeleter *)
        {
        }

        template<size_t N = 0, typename Tuple = decltype(resources)::VectorTypes>
        typename std::enable_if<N < std::tuple_size<Tuple>::value, void>::type releaseResources(ResourceDeleter *deleter)
        {
            using Resource = typename std::tuple_element_t<N, Tuple>::value_type;
            auto &resourcesVec = resources.get<Resource>();
            deleter->releaseResourcesOfType(resourcesVec);
            resourcesVec.clear();
                // Iterate
            if constexpr (N + 1 < std::tuple_size<Tuple>::value)
                releaseResources<N + 1>(deleter);
        }
        // clang-format on
    };

    const std::vector<FrameBin> &frameBins() const noexcept { return m_frameBins; }

private:
    auto getBin() -> FrameBin &;
    void destroyResources(FrameBin &bin);

    KDGpu::Device *m_device{ nullptr };
    std::atomic<uint64_t> m_frameNumber{ 0 };
    std::vector<FrameBin> m_frameBins;
    bool m_inDeleteAll{ false };
    size_t m_maxFramesInFlight{ 2 };

    friend struct FrameBin;

    template<typename Resource>
    void releaseResourcesOfType(const std::vector<Resource> &)
    {
        static_assert(sizeof(Resource) == -1, "releaseResourcesOfType should have been specialized");
    }
};

// clang-format off
template<> KDGPUUTILS_EXPORT void ResourceDeleter::releaseResourcesOfType<KDGpu::Buffer>(const std::vector<KDGpu::Buffer> &);
template<> KDGPUUTILS_EXPORT void ResourceDeleter::releaseResourcesOfType<KDGpu::BindGroup>(const std::vector<KDGpu::BindGroup> &);
template<> KDGPUUTILS_EXPORT void ResourceDeleter::releaseResourcesOfType<KDGpu::Texture>(const std::vector<KDGpu::Texture> &);
template<> KDGPUUTILS_EXPORT void ResourceDeleter::releaseResourcesOfType<KDGpu::TextureView>(const std::vector<KDGpu::TextureView> &);
template<> KDGPUUTILS_EXPORT void ResourceDeleter::releaseResourcesOfType<KDGpu::Sampler>(const std::vector<KDGpu::Sampler> &);
template<> KDGPUUTILS_EXPORT void ResourceDeleter::releaseResourcesOfType<KDGpu::GraphicsPipeline>(const std::vector<KDGpu::GraphicsPipeline> &);
template<> KDGPUUTILS_EXPORT void ResourceDeleter::releaseResourcesOfType<KDGpu::ComputePipeline>(const std::vector<KDGpu::ComputePipeline> &);
template<> KDGPUUTILS_EXPORT void ResourceDeleter::releaseResourcesOfType<KDGpu::PipelineLayout>(const std::vector<KDGpu::PipelineLayout> &);
template<> KDGPUUTILS_EXPORT void ResourceDeleter::releaseResourcesOfType<KDGpu::RayTracingPipeline>(const std::vector<KDGpu::RayTracingPipeline> &);
template<> KDGPUUTILS_EXPORT void ResourceDeleter::releaseResourcesOfType<KDGpu::AccelerationStructure>(const std::vector<KDGpu::AccelerationStructure> &);
template<> KDGPUUTILS_EXPORT void ResourceDeleter::releaseResourcesOfType<KDGpu::RayTracingShaderBindingTable>(const std::vector<KDGpu::RayTracingShaderBindingTable> &);
// clang-format on

} // namespace KDGpuUtils
