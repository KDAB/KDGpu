#pragma once

#include <toy_renderer/toy_renderer_export.h>

namespace ToyRenderer {

class TOY_RENDERER_EXPORT CommandBuffer
{
public:
    ~CommandBuffer();

private:
    CommandBuffer();

    friend class CommandRecorder;
};

} // namespace ToyRenderer
