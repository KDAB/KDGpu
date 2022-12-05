#pragma once

#include <toy_renderer/handle.h>
#include <toy_renderer/gpu_core.h>

#include <vector>

namespace ToyRenderer {

struct TextureView_t;

struct ColorAttachment {
    Handle<TextureView_t> view;
    AttachmentLoadOperation loadOperation{ AttachmentLoadOperation::Clear };
    AttachmentStoreOperation storeOperation{ AttachmentStoreOperation::Store };
    ColorClearValue clearValue;
};

struct DepthStencilAttachment {
    Handle<TextureView_t> view;
    AttachmentLoadOperation depthLoadOperation{ AttachmentLoadOperation::Clear };
    AttachmentStoreOperation depthStoreOperation{ AttachmentStoreOperation::Store };
    float depthClearValue{ 1.0f };
    AttachmentLoadOperation stencilLoadOperation{ AttachmentLoadOperation::Clear };
    AttachmentStoreOperation stencilStoreOperation{ AttachmentStoreOperation::Store };
    uint32_t stencilClearValue{ 0 };
};

struct RenderPassOptions {
    std::vector<ColorAttachment> colorAttachments;
    DepthStencilAttachment depthStencilAttachment;
};

} // namespace ToyRenderer
