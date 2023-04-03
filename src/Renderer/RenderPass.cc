#include "RenderPass.h"

#include "Renderer.h"

namespace chronicle {

RenderPass::RenderPass(const Renderer* renderer, const RenderPassInfo& renderPassInfo)
    : _renderPass(renderer->native().device(), renderPassInfo)
{
}

} // namespace chronicle