// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "VulkanCommon.h"

namespace chronicle {

/// @brief Vulkan utilities.
class VulkanUtils {
public:
    /// @brief Create an image and allocate his memory.
    /// @param width Image width.
    /// @param height Image height.
    /// @param mipLevels Mip levels.
    /// @param numSamples Number of samples.
    /// @param format Image format.
    /// @param tiling Image tiling.
    /// @param usage Image usage flags.
    /// @param properties Memory properties.
    /// @return A pair with an image and a device memory.
    [[nodiscard]] static std::pair<vk::DeviceMemory, vk::Image> createImage(uint32_t width, uint32_t height,
        uint32_t mipLevels, vk::SampleCountFlagBits numSamples, vk::Format format, vk::ImageTiling tiling,
        vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties);

    /// @brief Create an image view.
    /// @param image Source image.
    /// @param format Image format.
    /// @param aspectFlags Image aspect flags.
    /// @param mipLevels Image mip levels.
    /// @return Image view.
    [[nodiscard]] static vk::ImageView createImageView(
        vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags, uint32_t mipLevels);

    /// @brief Create a texture sampler.
    /// @param mipLevels Mip levels.
    /// @return Texture sampler.
    [[nodiscard]] static vk::Sampler createTextureSampler(uint32_t mipLevels);

    /// @brief Create a buffer.
    /// @param size Buffer size.
    /// @param usage Buffer usage flags.
    /// @param properties Memory property flags.
    /// @return A pair with a buffer and a device memory.
    static std::pair<vk::DeviceMemory, vk::Buffer> createBuffer(
        vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);

    /// @brief Copy one buffer into another.
    /// @param srcBuffer Source buffer.
    /// @param dstBuffer Destination buffer.
    /// @param size Size to copy.
    static void copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);

    /// @brief Copy a buffer into an image.
    /// @param srcBuffer Source buffer.
    /// @param dstImage Destination image.
    /// @param width Image width.
    /// @param height Image height.
    static void copyBufferToImage(vk::Buffer srcBuffer, vk::Image dstImage, uint32_t width, uint32_t height);

    /// @brief Transition the layout of an image.
    /// @param image Image.
    /// @param oldLayout Old layout.
    /// @param newLayout New layout.
    /// @param mipLevels Mip levels.
    static void transitionImageLayout(
        vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipLevels);

    /// @brief Generate mipmaps for an image.
    /// @param image Image.
    /// @param format Image format.
    /// @param width Image width.
    /// @param height Image height.
    /// @param mipLevels Mip levels.
    static void generateMipmaps(
        vk::Image image, vk::Format format, uint32_t width, uint32_t height, uint32_t mipLevels);

    /// @brief Find memory type.
    /// @param typeFilter Type filter.
    /// @param properties Memory properties.
    /// @return Memory index.
    [[nodiscard]] static uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);

    /// @brief Begin a command buffer for a single time command.
    /// @return Command buffer.
    static vk::CommandBuffer beginSingleTimeCommands();

    /// @brief End a command buffer for a single time command.
    /// @param commandBuffer Command buffer to end.
    static void endSingleTimeCommands(vk::CommandBuffer commandBuffer);

    /// @brief Check if the validation layer support is available.
    /// @param validationLayers Validation layers to check.
    /// @return True if they are supported, otherwise false.
    [[nodiscard]] static bool checkValidationLayerSupport(const std::vector<const char*>& validationLayers);

    /// @brief Get required extensions.
    /// @return Required extensions.
    [[nodiscard]] static std::vector<const char*> getRequiredExtensions();

    /// @brief Check if the physical device is suitable.
    /// @param physicalDevice Physical device.
    /// @param extensions Required extensions.
    /// @return True if the device is suitable, otherwise false.
    [[nodiscard]] static bool isDeviceSuitable(
        const vk::PhysicalDevice& physicalDevice, const std::vector<const char*>& extensions);

    /// @brief Check device extensions support.
    /// @param physicalDevice Physical device.
    /// @param extensions  Device extensions.
    /// @return True if are supported, otherwise false.
    [[nodiscard]] static bool checkDeviceExtensionSupport(
        const vk::PhysicalDevice& physicalDevice, const std::vector<const char*>& extensions);

    /// @brief Find queues families.
    /// @param physicalDevice Physical device.
    /// @return Queue family indices.
    [[nodiscard]] static VulkanQueueFamilyIndices findQueueFamilies(vk::PhysicalDevice physicalDevice);

    /// @brief Query swapchain support details.
    /// @param physicalDevice Physical device.
    /// @return Swapchain support details.
    [[nodiscard]] static VulkanSwapChainSupportDetails querySwapChainSupport(const vk::PhysicalDevice& physicalDevice);

    /// @brief Choose swap surface format.
    /// @param availableFormats Available formats.
    /// @return Swap surface format.
    [[nodiscard]] static vk::SurfaceFormatKHR chooseSwapSurfaceFormat(
        const std::vector<vk::SurfaceFormatKHR>& availableFormats);

    /// @brief Choose swap present mode.
    /// @param availablePresentModes Available present modes.
    /// @return Swap present mode.
    [[nodiscard]] static vk::PresentModeKHR chooseSwapPresentMode(
        const std::vector<vk::PresentModeKHR>& availablePresentModes);

    /// @brief Choose swap extent.
    /// @param capabilities Surface capabilities.
    /// @return Extent.
    [[nodiscard]] static vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);

    /// @brief Find supported format.
    /// @param candidates Candidate formats.
    /// @param tiling Image tiling.
    /// @param features Format feature flags.
    /// @return Supported format.
    [[nodiscard]] static vk::Format findSupportedFormat(
        const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);

    /// @brief Find depth format.
    /// @return Depth format.
    [[nodiscard]] static vk::Format findDepthFormat();

    /// @brief Check if has stencil component.
    /// @param format Format.
    /// @return True if have the stencil component, otherwise false.
    [[nodiscard]] static bool hasStencilComponent(vk::Format format);

    /// @brief Get max usable sample count.
    /// @return Sample count flags.
    [[nodiscard]] static vk::SampleCountFlagBits getMaxUsableSampleCount();

    /// @brief Cleanup garbage collector data.
    /// @param data Gargabe collector data.
    static void cleanupGarbageCollector(std::vector<GarbageCollectorData>& data);

    /// @brief Set a debug name to the object.
    /// @param objectType Object type.
    /// @param handle Object handle.
    /// @param name Debug name.
    static void setDebugObjectName(vk::ObjectType objectType, uint64_t handle, const char* name);

    /// @brief Set a debug name to a buffer.
    /// @param buffer Buffer handle.
    /// @param name Debug name.
    static void setDebugObjectName(vk::Buffer buffer, const char* name);

    /// @brief Set a debug name to a buffer.
    /// @param commandBuffer Command buffer handle.
    /// @param name Debug name.
    static void setDebugObjectName(vk::CommandBuffer commandBuffer, const char* name);

    /// @brief Set a debug name to a buffer.
    /// @param descriptorSet Descriptor set handle.
    /// @param name Debug name.
    static void setDebugObjectName(vk::DescriptorSet descriptorSet, const char* name);

    /// @brief Set a debug name to a buffer.
    /// @param pipeline Pipeline handle.
    /// @param name Debug name.
    static void setDebugObjectName(vk::Pipeline pipeline, const char* name);

    /// @brief Begin a debug label.
    /// @param commandBuffer Command buffer where to add the label.
    /// @param name Label name.
    /// @param color Label color.
    static void beginDebugLabel(vk::CommandBuffer commandBuffer, const char* name, glm::vec4 color);

    /// @brief End a debug label.
    /// @param commandBuffer Command buffer where the label was added.
    static void endDebugLabel(vk::CommandBuffer commandBuffer);

    /// @brief Insert a debug label.
    /// @param commandBuffer Command buffer where to insert the labe.
    /// @param name Label name.
    /// @param color Label color.
    static void insertDebugLabel(vk::CommandBuffer commandBuffer, const char* name, glm::vec4 color);
};

} // namespace chronicle