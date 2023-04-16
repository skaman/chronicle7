// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Renderer/Common.h"

namespace chronicle {

struct VulkanImGuiContext {
    static inline vk::DescriptorPool descriptorPool;
    static inline vk::PipelineCache pipelineCache;
};

class VulkanImGui {
public:
    static void init();
    static void deinit();

    static void createRenderPass();
    static void createFramebuffers();

    static void newFrame();
    static void render(const CommandBufferRef& commandBuffer);
};

} // namespace chronicle