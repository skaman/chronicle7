// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "VulkanFence.h"

#include "VulkanInstance.h"

namespace chronicle {

CHR_CONCRETE(VulkanFence);

VulkanFence::VulkanFence()
{
    CHRZONE_RENDERER;

    CHRLOG_DEBUG("Create fence");

    _fence = VulkanContext::device.createFence({ vk::FenceCreateFlagBits::eSignaled });
}

VulkanFence::~VulkanFence()
{
    CHRZONE_RENDERER;

    CHRLOG_DEBUG("Destroy fence");

    VulkanContext::device.destroyFence(_fence);
}

FenceRef VulkanFence::create()
{
    return std::make_shared<ConcreteVulkanFence>();
}

} // namespace chronicle