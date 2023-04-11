#pragma once

#include "pch.h"

#include "Renderer/SemaphoreI.h"

namespace chronicle {

class VulkanSemaphore : public SemaphoreI<VulkanSemaphore>, private NonCopyable<VulkanSemaphore> {
protected:
    explicit VulkanSemaphore();

public:
    ~VulkanSemaphore();

    [[nodiscard]] const vk::Semaphore& semaphore() const { return _semaphore; }

    static SemaphoreRef create();

private:
    vk::Semaphore _semaphore;
};

} // namespace chronicle