#pragma once

#include "pch.h"

#ifdef VULKAN_RENDERER
#include "Vulkan/VulkanFence.h"
#endif

namespace chronicle {

class Renderer;

class Fence {
public:
    explicit Fence(const Renderer* renderer);

#ifdef VULKAN_RENDERER
    [[nodiscard]] inline const VulkanFence& native() const { return _fence; };
#endif

private:
#ifdef VULKAN_RENDERER
    VulkanFence _fence;
#endif
};

} // namespace chronicle