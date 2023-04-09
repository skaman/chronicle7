#include "VulkanFence.h"

#include "Renderer/Renderer.h"

namespace chronicle {

CHR_CONCRETE(VulkanFence)

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

FenceRef VulkanFence::create(const Renderer* renderer)
{
    return std::make_shared<ConcreteVulkanFence>(renderer->native().device());
}

} // namespace chronicle