#include "VulkanFence.h"

namespace chronicle {

VulkanFence::VulkanFence(const vk::Device& device)
    : _device(device)
{
    _fence = _device.createFence({ vk::FenceCreateFlagBits::eSignaled });
}

    VulkanFence::~VulkanFence()
{
    _device.destroyFence(_fence);
}

} // namespace chronicle