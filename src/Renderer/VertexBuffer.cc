#include "VertexBuffer.h"

#include "Renderer.h"

namespace chronicle {

VertexBuffer::VertexBuffer(const Renderer* renderer)
    : _vertexBuffer(renderer->native().device(), renderer->native().physicalDevice(), renderer->native().commandPool(),
        renderer->native().graphicsQueue())
{
}

} // namespace chronicle