// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Renderer/RendererI.h"
#include "VulkanCommon.h"

namespace chronicle {

/// @brief Vulkan implementation for @ref RendererI
class VulkanRenderer : public RendererI<VulkanRenderer>, private NonCopyable<VulkanRenderer> {
public:
    /// @brief @see RendererI#init
    static void init();

    /// @brief @see RendererI#deinit
    static void deinit();

    /// @brief @see RendererI#waitIdle
    static void waitIdle();

    /// @brief @see RendererI#invalidateSwapChain
    static void invalidateSwapChain() { VulkanContext::swapChainInvalidated = true; }

    /// @brief @see RendererI#beginFrame
    static bool beginFrame();

    /// @brief @see RendererI#endFrame
    static void endFrame();

    /// @brief @see RendererI#debugShowLines
    [[nodiscard]] static bool debugShowLines();

    /// @brief @see RendererI#setDebugShowLines
    static void setDebugShowLines(bool enabled);

    /// @brief @see RendererI#descriptorSet
    [[nodiscard]] static const DescriptorSetRef& descriptorSet(uint32_t index)
    {
        assert(index >= 0);
        assert(index < VulkanContext::maxFramesInFlight);

        return VulkanContext::framesData[index].descriptorSet;
    }

    /// @brief @see RendererI#descriptorSet
    [[nodiscard]] static const DescriptorSetRef& descriptorSet()
    {
        return VulkanContext::framesData[VulkanContext::currentFrame].descriptorSet;
    }

    /// @brief @see RendererI#commandBuffer
    [[nodiscard]] static const CommandBufferRef& commandBuffer()
    {
        return VulkanContext::framesData[VulkanContext::currentFrame].commandBuffer;
    }

    /// @brief @see RendererI#width
    [[nodiscard]] static uint32_t width() { return VulkanContext::swapChainExtent.width; }

    /// @brief @see RendererI#height
    [[nodiscard]] static uint32_t height() { return VulkanContext::swapChainExtent.height; }

    /// @brief @see RendererI#maxFramesInFlight
    [[nodiscard]] static uint32_t maxFramesInFlight() { return VulkanContext::maxFramesInFlight; }

    /// @brief @see RendererI#descriptorSetLayout
    [[nodiscard]] static DescriptorSetLayout descriptorSetLayout();
};

} // namespace chronicle