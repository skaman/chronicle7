#include "Pipeline.h"

#include "Renderer.h"

namespace chronicle {

Pipeline::Pipeline(const Renderer* renderer, const PipelineInfo& pipelineInfo)
    : _pipeline(renderer->native().device(), pipelineInfo)
{
}

} // namespace chronicle