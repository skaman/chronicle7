// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Renderer/RendererI.h"
#include "VulkanCommon.h"

namespace chronicle {

class VulkanRenderer : public RendererI<VulkanRenderer>, private NonCopyable<VulkanRenderer> {
public:
    static void init();
    static void deinit();

    static void waitIdle();
    static void invalidateSwapChain() { VulkanContext::swapChainInvalidated = true; }

    static bool beginFrame();
    static void endFrame();

    [[nodiscard]] static bool debugShowLines();
    static void setDebugShowLines(bool enabled);

    [[nodiscard]] static const std::vector<DescriptorSetRef>& descriptorSets() { return VulkanContext::descriptorSets; }
    [[nodiscard]] static const DescriptorSetRef& descriptorSet()
    {
        return VulkanContext::descriptorSets[VulkanContext::currentFrame];
    }
    [[nodiscard]] static const CommandBufferRef& commandBuffer()
    {
        return VulkanContext::commandBuffers[VulkanContext::currentFrame];
    }
    [[nodiscard]] static uint32_t width() { return VulkanContext::swapChainExtent.width; }
    [[nodiscard]] static uint32_t height() { return VulkanContext::swapChainExtent.height; }
};

} // namespace chronicle