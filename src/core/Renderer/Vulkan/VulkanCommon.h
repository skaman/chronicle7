// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Renderer/Common.h"
#include "Renderer/RendererError.h"

namespace chronicle {

/// @brief Data structure for find queue families.
struct VulkanQueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily; ///< Graphics family index.
    std::optional<uint32_t> presentFamily; ///< Present family index.

    /// @brief Check if families are setted.
    /// @return True if all families are setted, otherwise false.
    [[nodiscard]] bool IsComplete() const { return graphicsFamily.has_value() && presentFamily.has_value(); }
};

/// @brief Data structure for find swapchain support.
struct VulkanSwapChainSupportDetails {
    vk::SurfaceCapabilitiesKHR capabilities; ///< Swapchain capabilities.
    std::vector<vk::SurfaceFormatKHR> formats; ///< Swapchain formats.
    std::vector<vk::PresentModeKHR> presentModes; ///< Present modes.
};

/// @brief Default uniform buffer object with model-view-projection matrix
struct UniformBufferObject {
    alignas(16) glm::mat4 model; ///< Model.
    alignas(16) glm::mat4 view; ///< View.
    alignas(16) glm::mat4 proj; ///< Projection.
};

struct VulkanContext {
    // instance, debugger and surface
    static inline vk::Instance instance = nullptr; ///< Vulkan instance.
    static inline VkDebugUtilsMessengerEXT debugCallback = VK_NULL_HANDLE; ///< Debug messenger callback.
    static inline vk::SurfaceKHR surface = nullptr; ///< Draw surface.

    // devices
    static inline vk::PhysicalDevice physicalDevice = nullptr; ///< Physical device.
    static inline vk::Device device = nullptr; ///< Logical device.

    // queues
    static inline vk::Queue graphicsQueue = nullptr; ///< Graphics queue.
    static inline vk::Queue presentQueue = nullptr; ///< Presentation queue.

    // families
    static inline uint32_t graphicsFamily = 0; ///< Graphics family index.
    static inline uint32_t presentFamily = 0; ///< Present family index.

    // swapchain
    static inline vk::SwapchainKHR swapChain = nullptr; ///< Swapchain.
    static inline std::vector<vk::Image> swapChainImages = {}; ///< Swapchain images.
    static inline std::vector<vk::ImageView> swapChainImageViews = {}; ///< Swapchain image views.
    static inline vk::Format swapChainImageFormat = vk::Format::eUndefined; ///< Swapchain image format.
    static inline vk::Extent2D swapChainExtent = {}; ///< Swapchain extent.
    static inline bool swapChainInvalidated = false; ///< Indicate if the swapchain is invalidated and need recreation.

    // depth image
    static inline vk::Image depthImage = nullptr; ///< Depth image.
    static inline vk::DeviceMemory depthImageMemory = nullptr; ///< Memory for depth image.
    static inline vk::ImageView depthImageView = nullptr; ///< Depth image view.
    static inline vk::Format depthImageFormat = vk::Format::eUndefined; ///< Depth image format.

    // command pool
    static inline vk::CommandPool commandPool = nullptr; ///< Command pool.

    // draw pass
    static inline vk::RenderPass renderPass = nullptr; ///< Main render pass.
    static inline vk::RenderPass debugRenderPass = nullptr; ///< Debug render pass.

    // framebuffers
    static inline std::vector<vk::Framebuffer> framebuffers = {}; ///< Framebuffer main render pass.
    static inline std::vector<vk::Framebuffer> debugFramebuffers = {}; ///< Framebuffer debug render pass.

    // multisampling
    static inline vk::SampleCountFlagBits msaaSamples = vk::SampleCountFlagBits::e1; ///< Number of samples.
    static inline vk::Image colorImage = nullptr; ///< Multisampling color image.
    static inline vk::DeviceMemory colorImageMemory = nullptr; ///< Memory for multisampling color image.
    static inline vk::ImageView colorImageView = nullptr; ///< Multisampling color image view.

    // sync objects
    static inline std::vector<vk::Semaphore> imageAvailableSemaphores = {}; ///< Image available semaphores.
    static inline std::vector<vk::Semaphore> renderFinishedSemaphores = {}; ///< Render finished semaphores.
    static inline std::vector<vk::Fence> inFlightFences = {}; ///< Fences for frames in flight.

    // command buffers
    static inline std::vector<CommandBufferRef> commandBuffers = {}; ///< Command Buffers.

    // descriptor sets
    static inline vk::DescriptorPool descriptorPool = nullptr; ///< Descriptor pool used to allocate resources.
    static inline std::vector<DescriptorSetRef> descriptorSets = {}; ///< Descriptor sets.

    // current frame
    static inline int currentFrame = 0; ///< Index of current frame in flight.
    static inline int currentImage = 0; ///< Index of current swapchain image.

    // options
    static inline int maxFramesInFlight = 3; ///< Number of max frames in flights.
    static inline bool enabledValidationLayer = true; ///< Enabled state for debug validation layers.

    // debug
    static inline bool debugShowLines = false; ///< Debug show lines.

    // events
    static inline entt::dispatcher dispatcher = {}; ///< Events dispatcher.
};

} // namespace chronicle