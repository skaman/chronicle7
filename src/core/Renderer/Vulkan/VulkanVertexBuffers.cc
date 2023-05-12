// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "VulkanVertexBuffers.h"

#include "VulkanVertexBuffer.h"

namespace chronicle {

CHR_CONCRETE(VulkanVertexBuffers);

chronicle::VulkanVertexBuffers::VulkanVertexBuffers(
    const std::vector<VertexBufferRef>& vertexBuffers, const std::vector<uint32_t>& offsets)
    : _vertexBuffers(vertexBuffers)
{
    assert(_vertexBuffers.size() > 0);
    assert(_vertexBuffers.size() == offsets.size());

    _buffers.resize(_vertexBuffers.size());
    _offsets.resize(_vertexBuffers.size());
    for (auto i = 0; i < _vertexBuffers.size(); i++) {
        const auto vulkanVertexBuffer = static_cast<VulkanVertexBuffer*>(_vertexBuffers[i].get());
        _buffers[i] = vulkanVertexBuffer->buffer();
        _offsets[i] = offsets[i];
    }
}

VertexBuffersRef VulkanVertexBuffers::create(
    const std::vector<VertexBufferRef>& vertexBuffers, const std::vector<uint32_t>& offsets)
{
    return std::make_shared<ConcreteVulkanVertexBuffers>(vertexBuffers, offsets);
}

VertexBuffersRef VulkanVertexBuffers::create(const VertexBufferRef& vertexBuffer, uint32_t offset)
{
    std::vector<VertexBufferRef> vertexBuffers = {};
    std::vector<uint32_t> offsets = {};

    vertexBuffers.push_back(vertexBuffer);
    offsets.push_back(offset);

    return std::make_shared<ConcreteVulkanVertexBuffers>(vertexBuffers, offsets);
}

} // namespace chronicle