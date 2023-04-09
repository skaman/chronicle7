#include "VulkanSemaphore.h"

#include "Renderer/Renderer.h"

namespace chronicle {

CHR_CONCRETE(VulkanSemaphore)

VulkanSemaphore::VulkanSemaphore(const vk::Device& device)
    : _device(device)
{
    CHRZONE_VULKAN

    _semaphore = _device.createSemaphore({});
}

VulkanSemaphore::~VulkanSemaphore()
{
    CHRZONE_VULKAN

    _device.destroySemaphore(_semaphore);
}

SemaphoreRef VulkanSemaphore::create(const Renderer* renderer)
{
    return std::make_shared<ConcreteVulkanSemaphore>(renderer->native().device());
}

} // namespace chronicle