// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "VulkanSemaphore.h"

#include "VulkanInstance.h"

namespace chronicle {

CHR_CONCRETE(VulkanSemaphore);

VulkanSemaphore::VulkanSemaphore()
{
    CHRZONE_RENDERER;

    CHRLOG_DEBUG("Create semaphore");

    _semaphore = VulkanContext::device.createSemaphore({});
}

VulkanSemaphore::~VulkanSemaphore()
{
    CHRZONE_RENDERER;

    CHRLOG_DEBUG("Destroy semaphore");

    VulkanContext::device.destroySemaphore(_semaphore);
}

SemaphoreRef VulkanSemaphore::create() { return std::make_shared<ConcreteVulkanSemaphore>(); }

} // namespace chronicle