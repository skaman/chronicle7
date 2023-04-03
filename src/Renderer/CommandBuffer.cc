#include "CommandBuffer.h"

#include "Renderer.h"

namespace chronicle {

CommandBuffer::CommandBuffer(const Renderer* renderer)
    : _commandBuffer(renderer->native().device(), renderer->native().commandPool())
{
}

} // namespace chronicle