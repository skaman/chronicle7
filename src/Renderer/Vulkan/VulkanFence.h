#pragma once

#include "pch.h"

namespace chronicle {

class VulkanFence {
public:
    explicit VulkanFence(const vk::Device& device);
    ~VulkanFence();

    [[nodiscard]] inline const vk::Fence fence() const { return _fence; }

private:
    vk::Device _device;
    vk::Fence _fence;
};

} // namespace chronicle