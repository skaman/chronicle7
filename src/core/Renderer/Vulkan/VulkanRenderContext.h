// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Renderer/BaseRenderContext.h"
#include "VulkanCommon.h"
#include "VulkanEnums.h"
#include "VulkanUtils.h"

namespace chronicle::internal::vulkan {

/// @brief Vulkan implementation for @ref BaseRenderContext
class VulkanRenderContext : public BaseRenderContext<VulkanRenderContext>, private NonCopyable<VulkanRenderContext> {
public:
    /// @brief @see BaseRenderContext#init
    static void init();

    /// @brief @see BaseRenderContext#deinit
    static void deinit();

    /// @brief @see BaseRenderContext#waitIdle
    static void waitIdle();

    /// @brief @see BaseRenderContext#invalidateSwapChain
    static void invalidateSwapChain() { VulkanContext::swapChainInvalidated = true; }

    /// @brief @see BaseRenderContext#beginFrame
    static bool beginFrame();

    /// @brief @see BaseRenderContext#endFrame
    static void endFrame();

    /// @brief @see BaseRenderContext#beginRenderPass
    static void beginRenderPass();

    /// @brief @see BaseRenderContext#endRenderPass
    static void endRenderPass();

    /// @brief @see BaseRenderContext#debugShowLines
    [[nodiscard]] static bool debugShowLines();

    /// @brief @see BaseRenderContext#setDebugShowLines
    static void setDebugShowLines(bool enabled);

    /// @brief @see BaseRenderContext#descriptorSet
    [[nodiscard]] static const DescriptorSetRef& descriptorSet(uint32_t index)
    {
        assert(index >= 0);
        assert(index < VulkanContext::maxFramesInFlight);

        return VulkanContext::framesData[index].descriptorSet;
    }

    /// @brief @see BaseRenderContext#descriptorSet
    [[nodiscard]] static const DescriptorSetRef& descriptorSet()
    {
        return VulkanContext::framesData[VulkanContext::currentFrame].descriptorSet;
    }

    /// @brief @see BaseRenderContext#commandBuffer
    [[nodiscard]] static const CommandBufferRef& commandBuffer()
    {
        return VulkanContext::framesData[VulkanContext::currentFrame].commandBuffer;
    }

    /// @brief @see BaseRenderContext#width
    [[nodiscard]] static uint32_t width() { return VulkanContext::swapChainExtent.width; }

    /// @brief @see BaseRenderContext#height
    [[nodiscard]] static uint32_t height() { return VulkanContext::swapChainExtent.height; }

    /// @brief @see BaseRenderContext#maxFramesInFlight
    [[nodiscard]] static uint32_t maxFramesInFlight() { return VulkanContext::maxFramesInFlight; }

    /// @brief @see BaseRenderContext#currentFrame
    [[nodiscard]] static uint32_t currentFrame() { return VulkanContext::currentFrame; }

    /// @brief @see BaseRenderContext#swapChainImageFormat
    [[nodiscard]] static Format swapChainImageFormat()
    {
        return VulkanEnums::formatFromVulkan(VulkanContext::swapChainImageFormat);
    }

    /// @brief @see BaseRenderContext#findDepthFormat
    [[nodiscard]] static Format findDepthFormat()
    {
        return VulkanEnums::formatFromVulkan(VulkanUtils::findDepthFormat());
    }

    /// @brief @see BaseRenderContext#descriptorSetLayout
    [[nodiscard]] static DescriptorSetLayout descriptorSetLayout();
};

} // namespace chronicle