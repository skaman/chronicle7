#include "VulkanFence.h"

namespace chronicle {

VulkanFence::VulkanFence(const vk::Device& device)
    : _device(device)
{
    CHRZONE_VULKAN

    _fence = _device.createFence({ vk::FenceCreateFlagBits::eSignaled });
}

VulkanFence::~VulkanFence()
{
    CHRZONE_VULKAN

    _device.destroyFence(_fence);
}

} // namespace chronicle