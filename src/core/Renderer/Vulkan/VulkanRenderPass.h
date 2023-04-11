#pragma once

#include "pch.h"

#include "Renderer/RenderPassI.h"

namespace chronicle {

struct ImageUpdateData {
    uint32_t index = 0;
    VulkanRenderPass* renderPass = nullptr;
};

class VulkanRenderPass : public RenderPassI<VulkanRenderPass>, private NonCopyable<VulkanRenderPass> {
protected:
    explicit VulkanRenderPass(const RenderPassInfo& renderPassInfo);

public:
    ~VulkanRenderPass();

    [[nodiscard]] const vk::RenderPass& renderPass() const { return _renderPass; }
    [[nodiscard]] const vk::Framebuffer& frameBuffer(uint32_t imageIndex) const { return _framebuffers[imageIndex]; }

    static RenderPassRef create(const RenderPassInfo& renderPassInfo);

private:
    vk::RenderPass _renderPass;
    std::vector<vk::Framebuffer> _framebuffers;
    std::vector<ImageRef> _images;
    ImageRef _depthImage;
    std::vector<ImageUpdateData> _updateData;

    vk::Framebuffer createFrameBuffer(const ImageRef& image, const ImageRef& depthImage) const;
    void recreateFrameBuffer(uint32_t imageIndex);

    static void imageUpdatedEvent(ImageUpdateData* data);
};

} // namespace chronicle