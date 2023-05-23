// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Renderer/Common/Common.h"

namespace chronicle {

/// @brief ImGui context
struct VulkanImGuiContext {
    static inline vk::DescriptorPool descriptorPool; ///< ImGui descriptor pool
    static inline vk::PipelineCache pipelineCache; ///< ImGui pipeline cache
};

/// @brief ImGui integration.
class VulkanImGui {
public:
    /// @brief Initialize ImGui.
    static void init();

    /// @brief Deinitialize ImGui.
    static void deinit();

    /// @brief Begin an ImGui new frame.
    static void newFrame();

    /// @brief Draw the UI data into the command buffer.
    /// @param commandBuffer Command where where to draw the UI.
    static void draw(CommandBufferId commandBufferId);
};

} // namespace chronicle