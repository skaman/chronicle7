#pragma once

#include "pch.h"

#include "Renderer/FenceI.h"

namespace chronicle {

class VulkanFence : public FenceI<VulkanFence>, private NonCopyable<VulkanFence> {
protected:
    explicit VulkanFence();

public:
    ~VulkanFence();

    [[nodiscard]] const vk::Fence& fence() const { return _fence; }

    static FenceRef create();

private:
    vk::Fence _fence;
};

} // namespace chronicle