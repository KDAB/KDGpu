#include "compute_pass_command_recorder.h"
#include <toy_renderer/graphics_api.h>
#include <toy_renderer/resource_manager.h>
#include <toy_renderer/api/api_compute_pass_command_recorder.h>

namespace ToyRenderer {

ComputePassCommandRecorder::ComputePassCommandRecorder(GraphicsApi *api,
                                                       const Handle<Device_t> &device,
                                                       const Handle<ComputePassCommandRecorder_t> &computePassCommandRecorder)
    : m_api(api)
    , m_device(device)
    , m_computePassCommandRecorder(computePassCommandRecorder)
{
}

ComputePassCommandRecorder::~ComputePassCommandRecorder()
{
}

void ComputePassCommandRecorder::setPipeline(const Handle<ComputePipeline_t> &pipeline)
{
    auto apiComputePassCommandRecorder = m_api->resourceManager()->getComputePassCommandRecorder(m_computePassCommandRecorder);
    apiComputePassCommandRecorder->setPipeline(pipeline);
}

void ComputePassCommandRecorder::setBindGroup(uint32_t group, const Handle<BindGroup_t> &bindGroup)
{
    auto apiComputePassCommandRecorder = m_api->resourceManager()->getComputePassCommandRecorder(m_computePassCommandRecorder);
    apiComputePassCommandRecorder->setBindGroup(group, bindGroup);
}

void ComputePassCommandRecorder::dispatchCompute(const ComputeCommand &command)
{
    auto apiComputePassCommandRecorder = m_api->resourceManager()->getComputePassCommandRecorder(m_computePassCommandRecorder);
    apiComputePassCommandRecorder->dispatchCompute(command);
}

void ComputePassCommandRecorder::dispatchCompute(const std::vector<ComputeCommand> &commands)
{
    auto apiComputePassCommandRecorder = m_api->resourceManager()->getComputePassCommandRecorder(m_computePassCommandRecorder);
    apiComputePassCommandRecorder->dispatchCompute(commands);
}

void ComputePassCommandRecorder::dispatchComputeIndirect(const ComputeCommandIndirect &command)
{
    auto apiComputePassCommandRecorder = m_api->resourceManager()->getComputePassCommandRecorder(m_computePassCommandRecorder);
    apiComputePassCommandRecorder->dispatchComputeIndirect(command);
}

void ComputePassCommandRecorder::dispatchComputeIndirect(const std::vector<ComputeCommandIndirect> &commands)
{
    auto apiComputePassCommandRecorder = m_api->resourceManager()->getComputePassCommandRecorder(m_computePassCommandRecorder);
    apiComputePassCommandRecorder->dispatchComputeIndirect(commands);
}

void ComputePassCommandRecorder::pushConstant(const PushConstantRange &constantRange, const std::vector<uint8_t> &data)
{
    auto apiComputePassCommandRecorder = m_api->resourceManager()->getComputePassCommandRecorder(m_computePassCommandRecorder);
    apiComputePassCommandRecorder->pushConstant(constantRange, data);
}

void ComputePassCommandRecorder::end()
{
    auto apiComputePassCommandRecorder = m_api->resourceManager()->getComputePassCommandRecorder(m_computePassCommandRecorder);
    apiComputePassCommandRecorder->end();
}

} // namespace ToyRenderer
