#pragma once

#include "pch.h"

#include "Renderer/Semaphore.h"

namespace chronicle {

class VulkanSemaphore : public SemaphoreI<VulkanSemaphore>, private NonCopyable<VulkanSemaphore> {
protected:
    explicit VulkanSemaphore(const vk::Device& device);

public:
    ~VulkanSemaphore();

    [[nodiscard]] const vk::Semaphore& semaphore() const { return _semaphore; }

    static SemaphoreRef create(const Renderer* renderer);

private:
    vk::Device _device;
    vk::Semaphore _semaphore;
};

} // namespace chronicle