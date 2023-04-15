// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "VulkanCommon.h"

namespace chronicle {

class VulkanUtils {
public:
    [[nodiscard]] static std::pair<vk::DeviceMemory, vk::Image> createImage(uint32_t width, uint32_t height,
        uint32_t mipLevels, vk::SampleCountFlagBits numSamples, vk::Format format, vk::ImageTiling tiling,
        vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties);
    [[nodiscard]] static vk::ImageView createImageView(
        vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags, uint32_t mipLevels);
    [[nodiscard]] static vk::Sampler createTextureSampler(uint32_t mipLevels);

    static void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties,
        vk::Buffer& buffer, vk::DeviceMemory& bufferMemory);
    static void copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);
    static void copyBufferToImage(vk::Buffer srcBuffer, vk::Image dstImage, uint32_t width, uint32_t height);

    static void transitionImageLayout(
        vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipLevels);
    static void generateMipmaps(
        vk::Image image, vk::Format format, uint32_t width, uint32_t height, uint32_t mipLevels);
    [[nodiscard]] static uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);

    static vk::CommandBuffer beginSingleTimeCommands();
    static void endSingleTimeCommands(vk::CommandBuffer commandBuffer);

    [[nodiscard]] static bool checkValidationLayerSupport(const std::vector<const char*>& validationLayers);
    [[nodiscard]] static std::vector<const char*> getRequiredExtensions();
    [[nodiscard]] static bool isDeviceSuitable(
        const vk::PhysicalDevice& physicalDevice, const std::vector<const char*>& extensions);
    [[nodiscard]] static bool checkDeviceExtensionSupport(
        const vk::PhysicalDevice& physicalDevice, const std::vector<const char*>& extensions);
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
    [[nodiscard]] static vk::SampleCountFlagBits getMaxUsableSampleCount();
};

} // namespace chronicle