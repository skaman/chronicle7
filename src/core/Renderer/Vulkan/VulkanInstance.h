// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Renderer/RendererI.h"

#include "VulkanCommon.h"

namespace chronicle {

struct VulkanQueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    [[nodiscard]] bool IsComplete() const { return graphicsFamily.has_value() && presentFamily.has_value(); }
};

struct VulkanSwapChainSupportDetails {
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
};

class VulkanInstance : public RendererI<VulkanInstance>, private NonCopyable<VulkanInstance> {
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
    static void createFramebuffers();
    static void createSyncObjects();
    static void createCommandBuffers();
    static void createDescriptorSets();

    [[nodiscard]] static bool checkValidationLayerSupport();
    [[nodiscard]] static std::vector<const char*> getRequiredExtensions();
    [[nodiscard]] static bool isDeviceSuitable(const vk::PhysicalDevice& physicalDevice);
    [[nodiscard]] static bool checkDeviceExtensionSupport(const vk::PhysicalDevice& physicalDevice);
    [[nodiscard]] static VulkanQueueFamilyIndices findQueueFamilies(vk::PhysicalDevice physicalDevice);
    [[nodiscard]] static VulkanSwapChainSupportDetails querySwapChainSupport(const vk::PhysicalDevice& physicalDevice);
    [[nodiscard]] static vk::SurfaceFormatKHR chooseSwapSurfaceFormat(
        const std::vector<vk::SurfaceFormatKHR>& availableFormats);
    [[nodiscard]] static vk::PresentModeKHR chooseSwapPresentMode(
        const std::vector<vk::PresentModeKHR>& availablePresentModes);
    [[nodiscard]] static vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);
    [[nodiscard]] static vk::Format findSupportedFormat(
        const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);
    [[nodiscard]] static vk::Format findDepthFormat();
    [[nodiscard]] static bool hasStencilComponent(vk::Format format);

    friend class VulkanRenderer;
};

} // namespace chronicle