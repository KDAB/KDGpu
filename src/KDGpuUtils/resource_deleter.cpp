/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDGpuUtils/resource_deleter.h>
#include <KDUtils/logging.h>

#include <functional>

namespace KDGpuUtils {

template<typename Signature>
class ScopedGuard
{
public:
    template<typename CreateSignature>
    ScopedGuard(std::function<CreateSignature> onCreate, std::function<Signature> &&onDestroy)
        : m_onDestroy{ onDestroy }
    {
        onCreate();
    }

    ~ScopedGuard()
    {
        m_onDestroy();
    }

private:
    std::function<Signature> m_onDestroy;
};

ResourceDeleter::ResourceDeleter(KDGpu::Device *device, size_t maxFramesInFlight)
    : m_device{ device }
    , m_maxFramesInFlight{ maxFramesInFlight }
{
}

ResourceDeleter::~ResourceDeleter()
{
    deleteAll();
}

void ResourceDeleter::deleteAll()
{
    // Delete any remaining resources
    ScopedGuard<void()> s(
            std::function<void()>([this]() { m_inDeleteAll = true; }),
            [this]() { m_inDeleteAll = false; });

    for (auto &bin : m_frameBins)
        destroyResources(bin);
}

void ResourceDeleter::moveToNextFrame()
{
    ++m_frameNumber;
}

void ResourceDeleter::derefFrameIndex(size_t frameIndex)
{
    // The renderer has finished processing command buffers that reference
    // resources from frameIndex. Iterate through the frame bins and
    // remove the reference for this frameIndex.
    //
    const auto currentFrameNumber = frameNumber();
    // If this leaves the framebin with no remaining references then it can
    // have its resources destroyed and removed from our set of framebins
    for (auto it = m_frameBins.begin(); it != m_frameBins.end();) {
        // Set frameReference for frameIndex as dirty for bins that don't match the currentFrameNumber
        if (it->frameNumber != currentFrameNumber)
            it->frameReferences[frameIndex] = false;
        if (it->canBeDestroyed()) {
            destroyResources(*it);
            it = m_frameBins.erase(it);
        } else {
            ++it;
        }
    }
}

auto ResourceDeleter::getBin() -> FrameBin &
{
    const uint64_t frameNumber = m_frameNumber.load();
    // First time init
    if (m_frameBins.empty()) {
        m_frameBins.emplace_back(FrameBin(frameNumber, m_maxFramesInFlight));
    }

    // Is the latest bin for this frame?
    auto &bin = m_frameBins.back();
    if (bin.frameNumber == frameNumber)
        return bin;

    // Create a new bin since we don't have one for this frame
    m_frameBins.emplace_back(FrameBin(frameNumber, m_maxFramesInFlight));
    return m_frameBins.back();
}

void ResourceDeleter::destroyResources(FrameBin &bin)
{
    if (!m_inDeleteAll && !bin.canBeDestroyed())
        SPDLOG_WARN("Deleting resources scheduled in frame {} which are still potentially referenced", bin.frameNumber);

    bin.releaseResources(this);
}

template<>
void ResourceDeleter::releaseResourcesOfType<KDGpu::Buffer>(const std::vector<KDGpu::Buffer> &buffers)
{
    // Nothing to do, Buffers will be implicitly destroyed
    // when entries are removed from the vector
}

template<>
void ResourceDeleter::releaseResourcesOfType<KDGpu::BindGroup>(const std::vector<KDGpu::BindGroup> &bindGroup)
{
    // Nothing to do, BindGroups will be implicitly destroyed
    // when entries are removed from the vector
}

template<>
void ResourceDeleter::releaseResourcesOfType<KDGpu::BindGroupLayout>(const std::vector<KDGpu::BindGroupLayout> &)
{
    // Nothing to do, BindGroupLayout will be implicitly destroyed
    // when entries are removed from the vector
}

template<>
void ResourceDeleter::releaseResourcesOfType<KDGpu::Texture>(const std::vector<KDGpu::Texture> &)
{
    // Nothing to do, Textures will be implicitly destroyed
    // when entries are removed from the vector
}

template<>
void ResourceDeleter::releaseResourcesOfType<KDGpu::TextureView>(const std::vector<KDGpu::TextureView> &)
{
    // Nothing to do, TextureViews will be implicitly destroyed
    // when entries are removed from the vector
}

template<>
void ResourceDeleter::releaseResourcesOfType<KDGpu::Sampler>(const std::vector<KDGpu::Sampler> &)
{
    // Nothing to do, Samplers will be implicitly destroyed
    // when entries are removed from the vector
}

template<>
void ResourceDeleter::releaseResourcesOfType<KDGpu::GraphicsPipeline>(const std::vector<KDGpu::GraphicsPipeline> &)
{
    // Nothing to do, GraphicsPipelines will be implicitly destroyed
    // when entries are removed from the vector
}

template<>
void ResourceDeleter::releaseResourcesOfType<KDGpu::ComputePipeline>(const std::vector<KDGpu::ComputePipeline> &)
{
    // Nothing to do, ComputePipelines will be implicitly destroyed
    // when entries are removed from the vector
}

template<>
void ResourceDeleter::releaseResourcesOfType<KDGpu::RayTracingPipeline>(const std::vector<KDGpu::RayTracingPipeline> &)
{
    // Nothing to do, RayTracingPipeline will be implicitly destroyed
    // when entries are removed from the vector
}

template<>
void ResourceDeleter::releaseResourcesOfType<KDGpu::PipelineLayout>(const std::vector<KDGpu::PipelineLayout> &)
{
    // Nothing to do, PipelineLayouts will be implicitly destroyed
    // when entries are removed from the vector
}

template<>
void ResourceDeleter::releaseResourcesOfType<KDGpu::AccelerationStructure>(const std::vector<KDGpu::AccelerationStructure> &)
{
    // Nothing to do, AccelerationStructure will be implicitly destroyed
    // when entries are removed from the vector
}

template<>
void ResourceDeleter::releaseResourcesOfType<KDGpu::RayTracingShaderBindingTable>(const std::vector<KDGpu::RayTracingShaderBindingTable> &)
{
    // Nothing to do, RayTracingShaderBindingTable will be implicitly destroyed
    // when entries are removed from the vector
}

template<>
void ResourceDeleter::releaseResourcesOfType<KDGpu::ShaderModule>(const std::vector<KDGpu::ShaderModule> &)
{
    // Nothing to do, ShaderModule will be implicitly destroyed
    // when entries are removed from the vector
}

} // namespace KDGpuUtils
