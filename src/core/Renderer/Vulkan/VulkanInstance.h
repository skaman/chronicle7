// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "VulkanCommon.h"

namespace chronicle {

class VulkanInstance {
public:
    static void init();
    static void deinit();

private:
    static void recreateSwapChain();
    static void cleanupSwapChain();
    static void createInstance();
    static void populateDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT& createInfo);
    static void setupDebugCallback();
    static void createSurface();
    static void pickPhysicalDevice();
    static void createLogicalDevice();
    static void createSwapChain();
    static void createCommandPool();
    static void createRenderPass();
    static void createDebugRenderPass();
    static void createFramebuffers();
    static void createDebugFramebuffers();
    static void createSyncObjects();
    static void createCommandBuffers();
    static void createDescriptorSets();
    static void createDescriptorPool();

    friend class VulkanRenderer;
};

} // namespace chronicle