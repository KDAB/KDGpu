/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDGpu/render_pass_command_recorder_options.h>
#include <KDGpu/vulkan/vulkan_render_pass.h>
#include <KDGpu/vulkan/vulkan_graphics_api.h>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

using namespace KDGpu;

namespace {

template<typename T>
struct FakeHandle : public Handle<T> {
    using Handle<T>::Handle; // Inherit constructors

    FakeHandle(uint32_t index, uint32_t generation)
        : Handle<T>(index, generation)
    {
    }
};

} // namespace

TEST_SUITE("VulkanRenderPassKey")
{
    FakeHandle<TextureView_t> fakeHandle{ 1, 1 }; // Just a dummy handle to create different keys

    TEST_CASE("VulkanRenderPassKeyColorAttachment")
    {
        SUBCASE("Check Different Keys for different loadOperations")
        {
            // GIVEN
            ColorAttachment a{
                .view = fakeHandle,
                .loadOperation = AttachmentLoadOperation::Clear,
            };
            ColorAttachment b{
                .view = fakeHandle,
                .loadOperation = AttachmentLoadOperation::Load,
            };
            ColorAttachment c{
                .view = fakeHandle,
                .loadOperation = AttachmentLoadOperation::DontCare,
            };

            // WHEN
            VulkanRenderPassKeyColorAttachment keyA(a, KDGpu::Format::UNDEFINED, KDGpu::Format::UNDEFINED);
            VulkanRenderPassKeyColorAttachment keyB(b, KDGpu::Format::UNDEFINED, KDGpu::Format::UNDEFINED);
            VulkanRenderPassKeyColorAttachment keyC(c, KDGpu::Format::UNDEFINED, KDGpu::Format::UNDEFINED);

            // THEN
            CHECK(keyA != keyB);
            CHECK(keyB != keyC);
            CHECK(keyA != keyC);
        }

        SUBCASE("Check Different Keys for different storeOperations")
        {
            // GIVEN
            ColorAttachment a{
                .view = fakeHandle,
                .storeOperation = AttachmentStoreOperation::Store,
            };
            ColorAttachment b{
                .view = fakeHandle,
                .storeOperation = AttachmentStoreOperation::DontCare,
            };

            // WHEN
            VulkanRenderPassKeyColorAttachment keyA(a, KDGpu::Format::UNDEFINED, KDGpu::Format::UNDEFINED);
            VulkanRenderPassKeyColorAttachment keyB(b, KDGpu::Format::UNDEFINED, KDGpu::Format::UNDEFINED);

            // THEN
            CHECK(keyA != keyB);
        }

        SUBCASE("Check Different Keys for different initialLayout")
        {
            // GIVEN
            ColorAttachment a{
                .view = fakeHandle,
                .initialLayout = TextureLayout::ColorAttachmentOptimal,
            };
            ColorAttachment b{
                .view = fakeHandle,
                .initialLayout = TextureLayout::General,
            };

            // WHEN
            VulkanRenderPassKeyColorAttachment keyA(a, KDGpu::Format::UNDEFINED, KDGpu::Format::UNDEFINED);
            VulkanRenderPassKeyColorAttachment keyB(b, KDGpu::Format::UNDEFINED, KDGpu::Format::UNDEFINED);

            // THEN
            CHECK(keyA != keyB);
        }

        SUBCASE("Check Different Keys for different finalLayout")
        {
            // GIVEN
            ColorAttachment a{
                .view = fakeHandle,
                .finalLayout = TextureLayout::ColorAttachmentOptimal,
            };
            ColorAttachment b{
                .view = fakeHandle,
                .finalLayout = TextureLayout::PresentSrc,
            };

            // WHEN
            VulkanRenderPassKeyColorAttachment keyA(a, KDGpu::Format::UNDEFINED, KDGpu::Format::UNDEFINED);
            VulkanRenderPassKeyColorAttachment keyB(b, KDGpu::Format::UNDEFINED, KDGpu::Format::UNDEFINED);

            // THEN
            CHECK(keyA != keyB);
        }

        SUBCASE("Check Different Keys for different attachment view format")
        {
            // GIVEN
            ColorAttachment a{
                .view = fakeHandle,
                .finalLayout = TextureLayout::ColorAttachmentOptimal,
            };
            ColorAttachment b{
                .view = fakeHandle,
                .finalLayout = TextureLayout::ColorAttachmentOptimal,
            };

            // WHEN
            VulkanRenderPassKeyColorAttachment keyA(a, KDGpu::Format::R8G8B8A8_UNORM, KDGpu::Format::UNDEFINED);
            VulkanRenderPassKeyColorAttachment keyB(b, KDGpu::Format::R16G16B16A16_UNORM, KDGpu::Format::UNDEFINED);

            // THEN
            CHECK(keyA != keyB);
        }

        SUBCASE("Check Different Keys for different attachment resolveView format")
        {
            // GIVEN
            ColorAttachment a{
                .view = fakeHandle,
                .finalLayout = TextureLayout::ColorAttachmentOptimal,
            };
            ColorAttachment b{
                .view = fakeHandle,
                .finalLayout = TextureLayout::ColorAttachmentOptimal,
            };

            // WHEN
            VulkanRenderPassKeyColorAttachment keyA(a, KDGpu::Format::R8G8B8A8_UNORM, KDGpu::Format::R8G8B8A8_SNORM);
            VulkanRenderPassKeyColorAttachment keyB(b, KDGpu::Format::R8G8B8A8_UNORM, KDGpu::Format::R8G8B8A8_UNORM);

            // THEN
            CHECK(keyA != keyB);
        }
    }

    TEST_CASE("VulkanRenderPassKeyDepthStencilAttachment")
    {
        SUBCASE("Check Different Keys for different depthLoadOperations")
        {
            // GIVEN
            DepthStencilAttachment a{
                .view = fakeHandle,
                .depthLoadOperation = AttachmentLoadOperation::Clear,
            };
            DepthStencilAttachment b{
                .view = fakeHandle,
                .depthLoadOperation = AttachmentLoadOperation::Load,
            };
            DepthStencilAttachment c{
                .view = fakeHandle,
                .depthLoadOperation = AttachmentLoadOperation::DontCare,
            };

            // WHEN
            VulkanRenderPassKeyDepthStencilAttachment keyA(a, KDGpu::Format::UNDEFINED, KDGpu::Format::UNDEFINED);
            VulkanRenderPassKeyDepthStencilAttachment keyB(b, KDGpu::Format::UNDEFINED, KDGpu::Format::UNDEFINED);
            VulkanRenderPassKeyDepthStencilAttachment keyC(c, KDGpu::Format::UNDEFINED, KDGpu::Format::UNDEFINED);

            // THEN
            CHECK(keyA != keyB);
            CHECK(keyB != keyC);
            CHECK(keyA != keyC);
        }

        SUBCASE("Check Different Keys for different depthStoreOperations")
        {
            // GIVEN
            DepthStencilAttachment a{
                .view = fakeHandle,
                .depthStoreOperation = AttachmentStoreOperation::Store,
            };
            DepthStencilAttachment b{
                .view = fakeHandle,
                .depthStoreOperation = AttachmentStoreOperation::DontCare,
            };
            // WHEN
            VulkanRenderPassKeyDepthStencilAttachment keyA(a, KDGpu::Format::UNDEFINED, KDGpu::Format::UNDEFINED);
            VulkanRenderPassKeyDepthStencilAttachment keyB(b, KDGpu::Format::UNDEFINED, KDGpu::Format::UNDEFINED);

            // THEN
            CHECK(keyA != keyB);
        }

        SUBCASE("Check Different Keys for different stencilLoadOperations")
        {
            // GIVEN
            DepthStencilAttachment a{
                .view = fakeHandle,
                .stencilLoadOperation = AttachmentLoadOperation::Clear,
            };
            DepthStencilAttachment b{
                .view = fakeHandle,
                .stencilLoadOperation = AttachmentLoadOperation::Load,
            };
            DepthStencilAttachment c{
                .view = fakeHandle,
                .stencilLoadOperation = AttachmentLoadOperation::DontCare,
            };

            // WHEN
            VulkanRenderPassKeyDepthStencilAttachment keyA(a, KDGpu::Format::UNDEFINED, KDGpu::Format::UNDEFINED);
            VulkanRenderPassKeyDepthStencilAttachment keyB(b, KDGpu::Format::UNDEFINED, KDGpu::Format::UNDEFINED);
            VulkanRenderPassKeyDepthStencilAttachment keyC(c, KDGpu::Format::UNDEFINED, KDGpu::Format::UNDEFINED);

            // THEN
            CHECK(keyA != keyB);
            CHECK(keyB != keyC);
            CHECK(keyA != keyC);
        }

        SUBCASE("Check Different Keys for different stencilStoreOperations")
        {
            // GIVEN
            DepthStencilAttachment a{
                .view = fakeHandle,
                .stencilStoreOperation = AttachmentStoreOperation::Store,
            };
            DepthStencilAttachment b{
                .view = fakeHandle,
                .stencilStoreOperation = AttachmentStoreOperation::DontCare,
            };
            // WHEN
            VulkanRenderPassKeyDepthStencilAttachment keyA(a, KDGpu::Format::UNDEFINED, KDGpu::Format::UNDEFINED);
            VulkanRenderPassKeyDepthStencilAttachment keyB(b, KDGpu::Format::UNDEFINED, KDGpu::Format::UNDEFINED);

            // THEN
            CHECK(keyA != keyB);
        }

        SUBCASE("Check Different Keys for different initialLayout")
        {
            // GIVEN
            DepthStencilAttachment a{
                .view = fakeHandle,
                .initialLayout = TextureLayout::Undefined,
            };
            DepthStencilAttachment b{
                .view = fakeHandle,
                .initialLayout = TextureLayout::General,
            };

            // WHEN
            VulkanRenderPassKeyDepthStencilAttachment keyA(a, KDGpu::Format::UNDEFINED, KDGpu::Format::UNDEFINED);
            VulkanRenderPassKeyDepthStencilAttachment keyB(b, KDGpu::Format::UNDEFINED, KDGpu::Format::UNDEFINED);

            // THEN
            CHECK(keyA != keyB);
        }

        SUBCASE("Check Different Keys for different finalLayout")
        {
            // GIVEN
            DepthStencilAttachment a{
                .view = fakeHandle,
                .finalLayout = TextureLayout::ColorAttachmentOptimal,
            };
            DepthStencilAttachment b{
                .view = fakeHandle,
                .finalLayout = TextureLayout::DepthStencilAttachmentOptimal,
            };

            // WHEN
            VulkanRenderPassKeyDepthStencilAttachment keyA(a, KDGpu::Format::UNDEFINED, KDGpu::Format::UNDEFINED);
            VulkanRenderPassKeyDepthStencilAttachment keyB(b, KDGpu::Format::UNDEFINED, KDGpu::Format::UNDEFINED);

            // THEN
            CHECK(keyA != keyB);
        }

        SUBCASE("Check Different Keys for different attachment view format")
        {
            // GIVEN
            DepthStencilAttachment a{
                .view = fakeHandle,
                .finalLayout = TextureLayout::ColorAttachmentOptimal,
            };
            DepthStencilAttachment b{
                .view = fakeHandle,
                .finalLayout = TextureLayout::ColorAttachmentOptimal,
            };

            // WHEN
            VulkanRenderPassKeyDepthStencilAttachment keyA(a, KDGpu::Format::D16_UNORM, KDGpu::Format::R8G8B8A8_SNORM);
            VulkanRenderPassKeyDepthStencilAttachment keyB(b, KDGpu::Format::D24_UNORM_S8_UINT, KDGpu::Format::R8G8B8A8_UNORM);

            // THEN
            CHECK(keyA != keyB);
        }

        SUBCASE("Check Different Keys for different attachment resolveView format")
        {
            // GIVEN
            DepthStencilAttachment a{
                .view = fakeHandle,
                .finalLayout = TextureLayout::ColorAttachmentOptimal,
            };
            DepthStencilAttachment b{
                .view = fakeHandle,
                .finalLayout = TextureLayout::ColorAttachmentOptimal,
            };

            // WHEN
            VulkanRenderPassKeyDepthStencilAttachment keyA(a, KDGpu::Format::D16_UNORM, KDGpu::Format::D32_SFLOAT);
            VulkanRenderPassKeyDepthStencilAttachment keyB(b, KDGpu::Format::D16_UNORM, KDGpu::Format::D16_UNORM);

            // THEN
            CHECK(keyA != keyB);
        }
    }

    TEST_CASE("VulkanRenderPassKey")
    {
        VulkanResourceManager resourceManager;

        SUBCASE("Check Different Keys for different loadOperations")
        {
            // GIVEN
            VulkanRenderPassKey a(RenderPassCommandRecorderOptions{
                                          .colorAttachments = {
                                                  {
                                                          .view = fakeHandle,
                                                          .loadOperation = AttachmentLoadOperation::Load,
                                                  },
                                          },
                                          .depthStencilAttachment = {
                                                  .view = fakeHandle,
                                                  .depthLoadOperation = AttachmentLoadOperation::Load,
                                          },
                                  },
                                  &resourceManager);

            VulkanRenderPassKey b(RenderPassCommandRecorderOptions{
                                          .colorAttachments = {
                                                  {
                                                          .view = fakeHandle,
                                                          .loadOperation = AttachmentLoadOperation::Clear,
                                                  },
                                          },
                                          .depthStencilAttachment = {
                                                  .view = fakeHandle,
                                                  .depthLoadOperation = AttachmentLoadOperation::Clear,
                                          },
                                  },
                                  &resourceManager);

            // THEN
            CHECK(a != b);
        }
    }
}
