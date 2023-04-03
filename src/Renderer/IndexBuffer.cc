#include "IndexBuffer.h"

#include "Renderer.h"

namespace chronicle {

IndexBuffer::IndexBuffer(const Renderer* renderer)
    : _indexBuffer(renderer->native().device(), renderer->native().physicalDevice(), renderer->native().commandPool(),
        renderer->native().graphicsQueue())
{
}

} // namespace chronicle