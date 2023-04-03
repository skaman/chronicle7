#pragma once

#include "pch.h"

#ifdef VULKAN_RENDERER
#include "Vulkan/VulkanVertexBuffer.h"
#endif

namespace chronicle {

class Renderer;

class VertexBuffer {
public:
    explicit VertexBuffer(const Renderer* renderer);

    void set(void* src, size_t size) { _vertexBuffer.set(src, size); }

#ifdef VULKAN_RENDERER
    [[nodiscard]] inline const VulkanVertexBuffer& native() const { return _vertexBuffer; };
#endif

private:
#ifdef VULKAN_RENDERER
    VulkanVertexBuffer _vertexBuffer;
#endif
};

} // namespace chronicle