
/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/device.h>
#include <KDGpu/buffer.h>
#include <KDGpu/buffer_options.h>
#include <KDGpuUtils/resource_deleter.h>

#include <map>
#include <cassert>

constexpr unsigned long long operator""_Mb(unsigned long long const x)
{
    return 1024LL * 1024LL * x;
}

namespace KDGpuUtils {

template<uint16_t MinimumBinCount = 1, size_t BinSize = 2_Mb>
class StagingBufferPoolImpl
{
public:
    StagingBufferPoolImpl(KDGpu::Device *device, ResourceDeleter *deleter)
        : m_device(device)
        , m_deleter(deleter)
    {
    }

    ~StagingBufferPoolImpl()
    {
        cleanup();
    }

    void derefFrameIndex(size_t frameIndex)
    {
        assert(m_lastBin == nullptr); // Flush should have been called
        m_frameIndex = frameIndex;
    }

    void cleanup()
    {
        flush();
        for (auto &bin : m_bins)
            m_deleter->deleteLater(std::move(bin.buffer));
        m_bins.clear();
    }

    std::pair<size_t, KDGpu::Handle<KDGpu::Buffer_t>> stage(std::span<const uint8_t> data)
    {
        return stage(data.data(), data.size());
    }

    // Returns offset at which data was copied into the staging buffer
    // and handle to the underlying VkBuffer
    std::pair<size_t, KDGpu::Handle<KDGpu::Buffer_t>> stage(const void *data, size_t byteSize)
    {
        assert(byteSize <= BinSize); // We can allocate a buffer largen than the BinSize for the StagingBuffer Pool

        auto copyContent = [this](const void *data, size_t byteSize) {
            const Allocation alloc = m_lastBin->allocate(byteSize);

            // Copy content into buffer
            std::memcpy(reinterpret_cast<uint8_t *>(m_lastBin->mapped) + alloc.offset, data, byteSize);
            return std::make_pair(alloc.offset, m_lastBin->buffer.handle());
        };

        // Find Bin with enough empty space to store around allocation
        if (m_lastBin) {
            if (m_lastBin->canAccommodate(byteSize)) {
                // We can use the bin and bin already mapped
                return copyContent(data, byteSize);
            }
            // Else -> last bin not big enough

            // Unmap and unset lastBin
            m_lastBin->unmap();
            m_lastBin = nullptr;
        }

        // Check if any bin big enough
        for (auto it = m_bins.begin(), end = m_bins.end(); it != end; ++it) {
            if (it->frameIndex == m_frameIndex && it->canAccommodate(byteSize)) {
                // If satisfactory bin found, record it as lastBin
                // and map it
                m_lastBin = &(*it);
                m_lastBin->map();
                return copyContent(data, byteSize);
            }
        }

        // Else -> create new bin if nothing can accommodate
        m_bins.emplace_back(Bin{ m_frameIndex });
        m_lastBin = &m_bins.back();

        // Init bin (create VK Buffer) and map it
        m_lastBin->init(m_device);
        m_lastBin->map();
        return copyContent(data, byteSize);
    }

    void flush()
    {
        // Ensure we unmap last mapped bin
        if (m_lastBin)
            m_lastBin->unmap(); // Unmap
        m_lastBin = nullptr;
    }

    void moveToNextFrame()
    {
        // We should have been flushed before calling this
        assert(m_lastBin == nullptr);

        // Early return if we have no bins
        if (m_bins.empty())
            return;

        struct BinCounter {
            int value = 0;
        };
        std::map<int, BinCounter> binToImageIndex;

        // Destroy excess bins
        // We keep at most MinimumBinCount bins for each frameIndex
        for (int64_t m = m_bins.size() - 1; m >= 0; --m) {
            const Bin &bin = m_bins[m];
            BinCounter &binCounter = binToImageIndex[bin.frameIndex];
            if (binCounter.value + 1 > MinimumBinCount) {
                auto it = m_bins.begin() + m;
                auto &bin = *it;
                m_deleter->deleteLater(std::move(bin.buffer));
                m_bins.erase(it);
            } else {
                ++(binCounter.value);
            }
        }

        // Clean bins we keep alive
        for (auto &bin : m_bins)
            bin.clear();
    }

    struct Allocation {
        size_t offset = 0; // in bytes
        size_t size = 0; // in bytes
    };

    struct Bin {

        bool canAccommodate(size_t s)
        {
            assert(s <= BinSize);
            if (m_allocations.empty())
                return true;

            const Allocation &lastAlloc = m_allocations.back();
            const size_t remainingCapacity = BinSize - (lastAlloc.offset + lastAlloc.size);
            return remainingCapacity >= s;
        }

        const Allocation &allocate(size_t s)
        {
            // Assume we can accommodate
            Allocation lastAlloc;
            if (!m_allocations.empty())
                lastAlloc = m_allocations.back();

            m_allocations.push_back({ lastAlloc.offset + lastAlloc.size, s });
            return m_allocations.back();
        }

        void clear()
        {
            m_allocations.clear();
        }

        void init(KDGpu::Device *device)
        {
            buffer = device->createBuffer(KDGpu::BufferOptions{
                    .size = BinSize,
                    .usage = KDGpu::BufferUsageFlags(KDGpu::BufferUsageFlagBits::TransferSrcBit),
                    .memoryUsage = KDGpu::MemoryUsage::CpuOnly });
        }

        void map()
        {
            mapped = buffer.map();
            isMapped = true;
        }

        void unmap()
        {
            buffer.unmap();
            isMapped = false;
            mapped = nullptr;
        }

        size_t frameIndex;
        KDGpu::Buffer buffer;
        bool isMapped = false;
        void *mapped = nullptr;
        std::vector<Allocation> m_allocations;
    };

    const std::vector<Bin> &bins() const noexcept { return m_bins; }

private:
    std::vector<Bin> m_bins;
    Bin *m_lastBin = nullptr;
    KDGpu::Device *m_device = nullptr;
    ResourceDeleter *m_deleter = nullptr;
    size_t m_frameIndex = 0;
};

using StagingBufferPool = StagingBufferPoolImpl<1, 2_Mb>;

} // namespace KDGpuUtils
