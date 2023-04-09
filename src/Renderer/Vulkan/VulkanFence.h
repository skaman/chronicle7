#pragma once

#include "pch.h"

#include "Renderer/Fence.h"

namespace chronicle {

class VulkanFence : public FenceI<VulkanFence>, private NonCopyable<VulkanFence> {
protected:
    explicit VulkanFence(const vk::Device& device);

public:
    ~VulkanFence();

    [[nodiscard]] const vk::Fence& fence() const { return _fence; }

    static FenceRef create(const Renderer* renderer);

private:
    vk::Device _device;
    vk::Fence _fence;
};

} // namespace chronicle