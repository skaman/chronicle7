#include "VulkanSemaphore.h"

namespace chronicle {

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

} // namespace chronicle