#pragma once

#include <KDGpu/gpu_core.h>
#include <KDGpu/handle.h>
#include <KDGpu/kdgpu_export.h>

#include <vector>

namespace KDGpu {

struct BindGroup_t;
struct Buffer_t;
struct Device_t;
struct ComputePipeline_t;
struct ComputePassCommandRecorder_t;
struct PipelineLayout_t;

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

class KDGPU_EXPORT ComputePassCommandRecorder
{
public:
    ~ComputePassCommandRecorder();

    ComputePassCommandRecorder(ComputePassCommandRecorder &&);
    ComputePassCommandRecorder &operator=(ComputePassCommandRecorder &&);

    ComputePassCommandRecorder(const ComputePassCommandRecorder &) = delete;
    ComputePassCommandRecorder &operator=(const ComputePassCommandRecorder &) = delete;

    const Handle<ComputePassCommandRecorder_t> &handle() const noexcept { return m_computePassCommandRecorder; }
    bool isValid() const noexcept { return m_computePassCommandRecorder.isValid(); }

    operator Handle<ComputePassCommandRecorder_t>() const noexcept { return m_computePassCommandRecorder; }

    void setPipeline(const Handle<ComputePipeline_t> &pipeline);

    void setBindGroup(uint32_t group, const Handle<BindGroup_t> &bindGroup,
                      const Handle<PipelineLayout_t> &pipelineLayout = Handle<PipelineLayout_t>(),
                      const std::vector<uint32_t> &dynamicBufferOffsets = {});

    void dispatchCompute(const ComputeCommand &command);
    void dispatchCompute(const std::vector<ComputeCommand> &commands);

    void dispatchComputeIndirect(const ComputeCommandIndirect &command);
    void dispatchComputeIndirect(const std::vector<ComputeCommandIndirect> &commands);

    void pushConstant(const PushConstantRange &constantRange, const void *data);

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

} // namespace KDGpu
