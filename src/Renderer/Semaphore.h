#pragma once

#include "pch.h"

#ifdef VULKAN_RENDERER
#include "Vulkan/VulkanSemaphore.h"
#endif

namespace chronicle {

class Renderer;

class Semaphore {
public:
    explicit Semaphore(const Renderer* renderer);

#ifdef VULKAN_RENDERER
    [[nodiscard]] inline const VulkanSemaphore& native() const { return _semaphore; };
#endif

private:
#ifdef VULKAN_RENDERER
    VulkanSemaphore _semaphore;
#endif
};

} // namespace chronicle