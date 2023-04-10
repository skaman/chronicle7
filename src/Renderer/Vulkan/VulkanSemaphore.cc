#include "VulkanSemaphore.h"

#include "VulkanInstance.h"

namespace chronicle {

CHR_CONCRETE(VulkanSemaphore)

VulkanSemaphore::VulkanSemaphore()
{
    CHRZONE_VULKAN

    _semaphore = VulkanContext::device.createSemaphore({});
}

VulkanSemaphore::~VulkanSemaphore()
{
    CHRZONE_VULKAN

    VulkanContext::device.destroySemaphore(_semaphore);
}

SemaphoreRef VulkanSemaphore::create() { return std::make_shared<ConcreteVulkanSemaphore>(); }

} // namespace chronicle