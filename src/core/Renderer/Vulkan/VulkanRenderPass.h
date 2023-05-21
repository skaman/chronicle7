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
    /// @param name Render pass name.
    explicit VulkanRenderPass(const RenderPassInfo& renderPassInfo, const std::string& name);

public:
    /// @brief Destructor.
    ~VulkanRenderPass();

    /// @brief @see RenderPassI#renderPassId
    [[nodiscard]] RenderPassId renderPassId() const { return _renderPass; }

    /// @brief @see RenderPassI#hash
    [[nodiscard]] size_t hash() const { return _hash; };

    /// @brief @see RenderPassI#format
    [[nodiscard]] Format format() const { return _format; };

    /// @brief @see RenderPassI#msaa
    [[nodiscard]] MSAA msaa() const { return _msaa; };

    /// @brief @see RenderPassI#create
    [[nodiscard]] static RenderPassRef create(const RenderPassInfo& renderPassInfo, const std::string& name);

private:
    std::string _name; ///< Name.
    vk::RenderPass _renderPass = nullptr; ///< Vulkan render pass.
    Format _format = Format::undefined; ///< Format for the surface used by the render pass.
    MSAA _msaa = MSAA::sampleCount1; ///< Multi sampling for the surface used by the render pass.
    size_t _hash; ///< Hash of the configuration used to create the render pass.

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