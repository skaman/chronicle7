#include "VulkanSemaphore.h"

namespace chronicle {

VulkanSemaphore::VulkanSemaphore(const vk::Device& device)
    : _device(device)
{
    _semaphore = _device.createSemaphore({});
}

VulkanSemaphore::~VulkanSemaphore() { _device.destroySemaphore(_semaphore); }

}