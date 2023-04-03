#pragma once

#include "pch.h"

#ifdef VULKAN_RENDERER
#include "Vulkan/VulkanIndexBuffer.h"
#endif

namespace chronicle {

class Renderer;

class IndexBuffer {
public:
    explicit IndexBuffer(const Renderer* renderer);

    void set(void* src, size_t size) { _indexBuffer.set(src, size); }

#ifdef VULKAN_RENDERER
    [[nodiscard]] inline const VulkanIndexBuffer& native() const { return _indexBuffer; };
#endif

private:
#ifdef VULKAN_RENDERER
    VulkanIndexBuffer _indexBuffer;
#endif
};

} // namespace chronicle