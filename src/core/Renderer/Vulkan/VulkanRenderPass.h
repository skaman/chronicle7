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
    vk::RenderPass _renderPass = nullptr; ///< Vulkan render pass.

    /// @brief Create a vulkan attachment description from a render pass attachment.
    /// @param attachment Render pass attachment.
    /// @return Vulkan attachment description.
    vk::AttachmentDescription createAttachmentDescription(const RenderPassAttachment& attachment) const;

    /// @brief Create a vulkan attachment reference from a render pass attachment.
    /// @param index Related attachment description index.
    /// @param isDepthAttachment Related attachment is a depth buffer attachment.
    /// @return Vulkan attachment reference.
    vk::AttachmentReference createAttachmentReference(uint32_t index, bool isDepthAttachment) const;
};

} // namespace chronicle