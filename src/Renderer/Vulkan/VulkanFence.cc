#include "VulkanFence.h"

#include "VulkanRenderer.h"

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
    const auto vulkanRenderer = static_cast<const VulkanRenderer*>(renderer);
    return std::make_shared<ConcreteVulkanFence>(vulkanRenderer->device());
}

} // namespace chronicle