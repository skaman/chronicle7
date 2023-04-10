#include "VulkanSemaphore.h"

#include "VulkanInstance.h"

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
    const auto vulkanInstance = static_cast<const VulkanInstance*>(renderer);
    return std::make_shared<ConcreteVulkanSemaphore>(vulkanInstance->device());
}

} // namespace chronicle