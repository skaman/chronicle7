// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Renderer/RenderPassI.h"

namespace chronicle {

/// @brief Vulkan implementation for @ref RenderPassI
class VulkanRenderPass : public RenderPassI<VulkanRenderPass>, private NonCopyable<VulkanRenderPass> {
protected:
    /// @brief Constructor.
    /// @param renderPassInfo Informations used to create the render pass.
    explicit VulkanRenderPass(const RenderPassInfo& renderPassInfo);

public:
    /// @brief Destructor.
    ~VulkanRenderPass();

    /// @brief @see RenderPassI#renderPassId
    [[nodiscard]] RenderPassId renderPassId() const { return static_cast<RenderPassId>(&_renderPass); }

    /// @brief @see RenderPassI#create
    [[nodiscard]] static RenderPassRef create(const RenderPassInfo& renderPassInfo);

private:
    vk::RenderPass _renderPass = nullptr;

    vk::AttachmentDescription createAttachmentDescription(const RenderPassAttachment& attachment) const;
    vk::AttachmentReference createAttachmentReference(const RenderPassAttachment& attachment, uint32_t index) const;
};

} // namespace chronicle