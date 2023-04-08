#pragma once

#include "pch.h"

#include "Renderer/Fence.h"

namespace chronicle {

class VulkanFence : public FenceI<VulkanFence>, private NonCopyable<VulkanFence> {
public:
    ~VulkanFence();

    [[nodiscard]] const vk::Fence& fence() const { return _fence; }

protected:
    explicit VulkanFence(const vk::Device& device);

    static std::shared_ptr<FenceI<VulkanFence>> createImpl(const Renderer* renderer);

private:
    vk::Device _device;
    vk::Fence _fence;

    friend class FenceI;
};

} // namespace chronicle