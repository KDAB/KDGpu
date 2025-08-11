/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDGpu/vulkan/vulkan_framebuffer.h>
#include <KDGpu/vulkan/vulkan_graphics_api.h>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

using namespace KDGpu;

template<typename T>
struct MyHandle : public Handle<T> {

    MyHandle(uint32_t index, uint32_t generation)
        : Handle<T>(index, generation)
    {
    }
};

TEST_SUITE("VulkanFramebufferKey")
{
    TEST_CASE("VulkanAttachmentKey")
    {
        SUBCASE("Check Different Keys for different loadOperations")
        {
            // GIVEN
            Handle<TextureView_t> h1 = MyHandle<TextureView_t>(4, 99);
            Handle<TextureView_t> h2 = MyHandle<TextureView_t>(0, 174);

            // WHEN
            VulkanAttachmentKey key1;
            VulkanAttachmentKey key2;

            key1.addAttachmentView(h1);
            key2.addAttachmentView(h2);

            // THEN
            CHECK(key1 != key2);
        }
    }

    TEST_CASE("VulkanFramebufferKey")
    {
        VulkanResourceManager resourceManager;

        SUBCASE("Check Different Keys for different dimensions")
        {
            {
                // GIVEN
                const VulkanFramebufferKey keyA{
                    .renderPass = {},
                    .attachmentsKey = {},
                    .width = 1024,
                    .height = 1024,
                    .layers = 1,
                    .viewCount = 0,
                };
                const VulkanFramebufferKey keyB{
                    .renderPass = {},
                    .attachmentsKey = {},
                    .width = 1024,
                    .height = 800,
                    .layers = 1,
                    .viewCount = 0,
                };

                // THEN
                CHECK(keyA != keyB);
            }
            {
                // GIVEN
                const VulkanFramebufferKey keyA{
                    .renderPass = {},
                    .attachmentsKey = {},
                    .width = 800,
                    .height = 800,
                    .layers = 1,
                    .viewCount = 0,
                };
                const VulkanFramebufferKey keyB{
                    .renderPass = {},
                    .attachmentsKey = {},
                    .width = 1024,
                    .height = 800,
                    .layers = 1,
                    .viewCount = 0,
                };

                // THEN
                CHECK(keyA != keyB);
            }
        }

        SUBCASE("Check Different Keys for different attachmentsKey")
        {
            // GIVEN
            VulkanAttachmentKey attachmnetKeyA;
            VulkanAttachmentKey attachmnetKeyB;

            attachmnetKeyA.addAttachmentView(MyHandle<TextureView_t>(4, 99));
            attachmnetKeyB.addAttachmentView(MyHandle<TextureView_t>(0, 174));

            // WHEN
            const VulkanFramebufferKey keyA{
                .renderPass = {},
                .attachmentsKey = attachmnetKeyA,
                .width = 1024,
                .height = 800,
                .layers = 1,
                .viewCount = 0,
            };
            const VulkanFramebufferKey keyB{
                .renderPass = {},
                .attachmentsKey = attachmnetKeyB,
                .width = 1024,
                .height = 800,
                .layers = 1,
                .viewCount = 0,
            };

            // THEN
            CHECK(keyA != keyB);
        }

        SUBCASE("Check Different Keys for different layers")
        {
            // GIVEN
            VulkanAttachmentKey attachmnetKey;
            attachmnetKey.addAttachmentView(MyHandle<TextureView_t>(4, 99));

            // WHEN
            const VulkanFramebufferKey keyA{
                .renderPass = {},
                .attachmentsKey = attachmnetKey,
                .width = 1024,
                .height = 800,
                .layers = 1,
                .viewCount = 0,
            };
            const VulkanFramebufferKey keyB{
                .renderPass = {},
                .attachmentsKey = attachmnetKey,
                .width = 1024,
                .height = 800,
                .layers = 2,
                .viewCount = 0,
            };

            // THEN
            CHECK(keyA != keyB);
        }

        SUBCASE("Check Different Keys for different viewCount")
        {
            // GIVEN
            VulkanAttachmentKey attachmnetKey;
            attachmnetKey.addAttachmentView(MyHandle<TextureView_t>(4, 99));

            // WHEN
            const VulkanFramebufferKey keyA{
                .renderPass = {},
                .attachmentsKey = attachmnetKey,
                .width = 1024,
                .height = 800,
                .layers = 1,
                .viewCount = 1,
            };
            const VulkanFramebufferKey keyB{
                .renderPass = {},
                .attachmentsKey = attachmnetKey,
                .width = 1024,
                .height = 800,
                .layers = 1,
                .viewCount = 2,
            };

            // THEN
            CHECK(keyA != keyB);
        }

        SUBCASE("Check Different Keys for different renderPass")
        {
            // GIVEN
            Handle<RenderPass_t> renderPassA = MyHandle<RenderPass_t>(1, 1);
            Handle<RenderPass_t> renderPassB = MyHandle<RenderPass_t>(2, 1);

            VulkanAttachmentKey attachmnetKey;
            attachmnetKey.addAttachmentView(MyHandle<TextureView_t>(4, 99));

            // WHEN
            const VulkanFramebufferKey keyA{
                .renderPass = renderPassA,
                .attachmentsKey = attachmnetKey,
                .width = 1024,
                .height = 800,
                .layers = 1,
                .viewCount = 0,
            };
            const VulkanFramebufferKey keyB{
                .renderPass = renderPassB,
                .attachmentsKey = attachmnetKey,
                .width = 1024,
                .height = 800,
                .layers = 1,
                .viewCount = 0,
            };

            // THEN
            CHECK(keyA != keyB);
        }
    }
}
