// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Renderer/RenderContextI.h"
#include "VulkanCommon.h"
#include "VulkanEnums.h"
#include "VulkanUtils.h"

namespace chronicle {

/// @brief Vulkan implementation for @ref RenderContextI
class VulkanRenderContext : public RenderContextI<VulkanRenderContext>, private NonCopyable<VulkanRenderContext> {
public:
    /// @brief @see RenderContextI#init
    static void init();

    /// @brief @see RenderContextI#deinit
    static void deinit();

    /// @brief @see RenderContextI#waitIdle
    static void waitIdle();

    /// @brief @see RenderContextI#invalidateSwapChain
    static void invalidateSwapChain() { VulkanContext::swapChainInvalidated = true; }

    /// @brief @see RenderContextI#beginFrame
    static bool beginFrame();

    /// @brief @see RenderContextI#endFrame
    static void endFrame();

    /// @brief @see RenderContextI#beginRenderPass
    static void beginRenderPass();

    /// @brief @see RenderContextI#endRenderPass
    static void endRenderPass();

    /// @brief @see RenderContextI#debugShowLines
    [[nodiscard]] static bool debugShowLines();

    /// @brief @see RenderContextI#setDebugShowLines
    static void setDebugShowLines(bool enabled);

    /// @brief @see RenderContextI#descriptorSet
    [[nodiscard]] static const DescriptorSetRef& descriptorSet(uint32_t index)
    {
        assert(index >= 0);
        assert(index < VulkanContext::maxFramesInFlight);

        return VulkanContext::framesData[index].descriptorSet;
    }

    /// @brief @see RenderContextI#descriptorSet
    [[nodiscard]] static const DescriptorSetRef& descriptorSet()
    {
        return VulkanContext::framesData[VulkanContext::currentFrame].descriptorSet;
    }

    /// @brief @see RenderContextI#commandBuffer
    [[nodiscard]] static const CommandBufferRef& commandBuffer()
    {
        return VulkanContext::framesData[VulkanContext::currentFrame].commandBuffer;
    }

    /// @brief @see RenderContextI#width
    [[nodiscard]] static uint32_t width() { return VulkanContext::swapChainExtent.width; }

    /// @brief @see RenderContextI#height
    [[nodiscard]] static uint32_t height() { return VulkanContext::swapChainExtent.height; }

    /// @brief @see RenderContextI#maxFramesInFlight
    [[nodiscard]] static uint32_t maxFramesInFlight() { return VulkanContext::maxFramesInFlight; }

    /// @brief @see RenderContextI#currentFrame
    [[nodiscard]] static uint32_t currentFrame() { return VulkanContext::currentFrame; }

    /// @brief @see RenderContextI#swapChainImageFormat
    [[nodiscard]] static Format swapChainImageFormat()
    {
        return VulkanEnums::formatFromVulkan(VulkanContext::swapChainImageFormat);
    }

    /// @brief @see RenderContextI#findDepthFormat
    [[nodiscard]] static Format findDepthFormat()
    {
        return VulkanEnums::formatFromVulkan(VulkanUtils::findDepthFormat());
    }

    /// @brief @see RenderContextI#descriptorSetLayout
    [[nodiscard]] static DescriptorSetLayout descriptorSetLayout();
};

} // namespace chronicle