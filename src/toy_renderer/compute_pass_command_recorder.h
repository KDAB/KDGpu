#pragma once

#include <toy_renderer/gpu_core.h>
#include <toy_renderer/handle.h>
#include <toy_renderer/toy_renderer_export.h>

namespace ToyRenderer {

struct BindGroup_t;
struct Buffer_t;
struct Device_t;
struct ComputePipeline_t;
struct ComputePassCommandRecorder_t;

struct PushConstantRange;
class GraphicsApi;

struct ComputeCommand {
    uint32_t workGroupX{ 1 };
    uint32_t workGroupY{ 1 };
    uint32_t workGroupZ{ 1 };
};

struct ComputeCommandIndirect {
    Handle<Buffer_t> buffer;
    size_t offset{ 0 };
};

struct ComputePassCommandRecorderOptions {
};

class TOY_RENDERER_EXPORT ComputePassCommandRecorder
{
public:
    ~ComputePassCommandRecorder();

    const Handle<ComputePassCommandRecorder_t> &handle() const noexcept { return m_computePassCommandRecorder; }
    bool isValid() const noexcept { return m_computePassCommandRecorder.isValid(); }

    operator Handle<ComputePassCommandRecorder_t>() const noexcept { return m_computePassCommandRecorder; }

    void setPipeline(const Handle<ComputePipeline_t> &pipeline);

    void setBindGroup(uint32_t group, const Handle<BindGroup_t> &bindGroup);

    void dispatchCompute(const ComputeCommand &command);
    void dispatchCompute(const std::vector<ComputeCommand> &commands);

    void dispatchComputeIndirect(const ComputeCommandIndirect &command);
    void dispatchComputeIndirect(const std::vector<ComputeCommandIndirect> &commands);

    void pushConstant(const PushConstantRange &constantRange, const std::vector<uint8_t> &data);

    void end();

private:
    explicit ComputePassCommandRecorder(GraphicsApi *api,
                                        const Handle<Device_t> &device,
                                        const Handle<ComputePassCommandRecorder_t> &computePassCommandRecorder);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<ComputePassCommandRecorder_t> m_computePassCommandRecorder;

    friend class CommandRecorder;
};

} // namespace ToyRenderer
