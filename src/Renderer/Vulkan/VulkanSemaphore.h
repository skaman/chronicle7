#pragma once

#include "pch.h"

namespace chronicle {

class VulkanSemaphore {
public:
    explicit VulkanSemaphore(const vk::Device& device);
    ~VulkanSemaphore();

    [[nodiscard]] inline const vk::Semaphore& semaphore() const { return _semaphore; }

private:
    vk::Device _device;
    vk::Semaphore _semaphore;
};

} // namespace chronicle