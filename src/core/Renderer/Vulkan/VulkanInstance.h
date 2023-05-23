// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "VulkanCommon.h"

namespace chronicle {

/// @brief Handle all the base resources initialization required for vulkan renderer.
class VulkanInstance {
public:
    /// @brief Initialize vulkan.
    static void init();

    /// @brief definitialize vulkan.
    static void deinit();

private:
    /// @brief Recreate the swapchain and related resources.
    static void recreateSwapChain();

    /// @brief Cleanup swapchain and related resources.
    static void cleanupSwapChain();

    /// @brief Create the vulkan instance.
    static void createInstance();

    /// @brief Populate the debug structure creation info.
    /// @param createInfo Creation info data structure.
    static void populateDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT& createInfo);

    /// @brief Register the debug callbacks.
    static void setupDebugCallback();

    /// @brief Create the surface.
    static void createSurface();

    /// @brief Pick a physical device.
    static void pickPhysicalDevice();

    /// @brief Create the logical device.
    static void createLogicalDevice();

    /// @brief Create the swapchain and related resources.
    static void createSwapChain();

    /// @brief Create the command pool.
    static void createCommandPool();

    /// @brief Create the main render pass.
    static void createRenderPass();

    /// @brief Create the main frame buffers.
    static void createFramebuffers();

    /// @brief Create the synchronization objects.
    static void createSyncObjects();

    /// @brief Create the command buffers.
    static void createCommandBuffers();

    /// @brief Create the descriptor sets.
    static void createDescriptorSets();

    /// @brief Create the descriptor pool.
    static void createDescriptorPool();

    friend class VulkanRenderContext;
};

} // namespace chronicle