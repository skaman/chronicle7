#include "VulkanFence.h"

#include "VulkanInstance.h"

namespace chronicle {

CHR_CONCRETE(VulkanFence)

VulkanFence::VulkanFence()
{
    CHRZONE_VULKAN

    _fence = VulkanContext::device.createFence({ vk::FenceCreateFlagBits::eSignaled });
}

VulkanFence::~VulkanFence()
{
    CHRZONE_VULKAN

    VulkanContext::device.destroyFence(_fence);
}

FenceRef VulkanFence::create()
{
    return std::make_shared<ConcreteVulkanFence>();
}

} // namespace chronicle