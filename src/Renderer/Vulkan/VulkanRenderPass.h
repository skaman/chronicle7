#pragma once

#include "pch.h"

namespace chronicle {

class VulkanRenderer;
struct RenderPassInfo;
class Image;
class VulkanRenderPass;

struct ImageUpdateData {
    uint32_t index = 0;
    VulkanRenderPass* renderPass = nullptr;
};

class VulkanRenderPass {
public:
    explicit VulkanRenderPass(const vk::Device& device, const RenderPassInfo& renderPassInfo);
    ~VulkanRenderPass();

    [[nodiscard]] inline const vk::RenderPass& renderPass() const { return _renderPass; }
    [[nodiscard]] inline const vk::Framebuffer& frameBuffer(uint32_t imageIndex) const
    {
        return _framebuffers[imageIndex];
    }

private:
    vk::Device _device;
    vk::RenderPass _renderPass;
    std::vector<vk::Framebuffer> _framebuffers;
    std::vector<std::shared_ptr<Image>> _images;
    std::vector<ImageUpdateData> _updateData;

    vk::Framebuffer createFrameBuffer(const std::shared_ptr<Image>& image) const;
    void recreateFrameBuffer(uint32_t imageIndex);

    static void imageUpdatedEvent(ImageUpdateData* data);
};

} // namespace chronicle