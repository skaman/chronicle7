// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Renderer/Common.h"
#include "Renderer/RendererError.h"

namespace chronicle {

/// @brief Data structure for find queue families.
struct VulkanQueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily {}; ///< Graphics family index.
    std::optional<uint32_t> presentFamily {}; ///< Present family index.

    /// @brief Check if families are setted.
    /// @return True if all families are setted, otherwise false.
    [[nodiscard]] bool IsComplete() const { return graphicsFamily.has_value() && presentFamily.has_value(); }
};

/// @brief Data structure for find swapchain support.
struct VulkanSwapChainSupportDetails {
    vk::SurfaceCapabilitiesKHR capabilities {}; ///< Swapchain capabilities.
    std::vector<vk::SurfaceFormatKHR> formats {}; ///< Swapchain formats.
    std::vector<vk::PresentModeKHR> presentModes {}; ///< Present modes.
};

/// @brief Default uniform buffer object with model-view-projection matrix
struct UniformBufferObject {
    alignas(16) glm::mat4 model {}; ///< Model.
    alignas(16) glm::mat4 view {}; ///< View.
    alignas(16) glm::mat4 proj {}; ///< Projection.
};

/// @brief Data related to a single frame in flight.
struct VulkanFrameData {
    // sync objects
    vk::Semaphore imageAvailableSemaphore {}; ///< Image available semaphore.
    vk::Semaphore renderFinishedSemaphore {}; ///< Render finished semaphore.
    vk::Fence inFlightFence {}; ///< Fence for frames in flight.

    // command buffers
    CommandBufferRef commandBuffer {}; ///< Command Buffer.

    // descriptor sets
    DescriptorSetRef descriptorSet {}; ///< Descriptor set.
};

/// @brief Data related to a single swapchain image.
struct VulkanImageData {
    // swapchain
    TextureRef swapChainTexture {}; ///< Swapchain texture.

    // framebuffers
    FrameBufferRef framebuffer {}; ///< Framebuffer main render pass.
};

struct VulkanContext {
    // instance, debugger and surface
    static inline vk::Instance instance {}; ///< Vulkan instance.
    static inline VkDebugUtilsMessengerEXT debugCallback { VK_NULL_HANDLE }; ///< Debug messenger callback.
    static inline vk::SurfaceKHR surface {}; ///< Draw surface.

    // devices
    static inline vk::PhysicalDevice physicalDevice {}; ///< Physical device.
    static inline vk::Device device {}; ///< Logical device.

    // queues
    static inline vk::Queue graphicsQueue {}; ///< Graphics queue.
    static inline vk::Queue presentQueue {}; ///< Presentation queue.

    // families
    static inline uint32_t graphicsFamily {}; ///< Graphics family index.
    static inline uint32_t presentFamily {}; ///< Present family index.

    // swapchain
    static inline vk::SwapchainKHR swapChain {}; ///< Swapchain.
    static inline vk::Format swapChainImageFormat { vk::Format::eUndefined }; ///< Swapchain image format.
    static inline vk::Extent2D swapChainExtent {}; ///< Swapchain extent.
    static inline bool swapChainInvalidated {
        false
    }; ///< Indicate if the swapchain is invalidated and need recreation.

    // command pool
    static inline vk::CommandPool commandPool {}; ///< Command pool.

    // draw pass
    static inline RenderPassRef renderPass {}; ///< Main render pass.

    // descriptor sets
    static inline vk::DescriptorPool descriptorPool {}; ///< Descriptor pool used to allocate resources.

    // frame data
    static inline std::vector<VulkanFrameData> framesData {}; ///< Data related to the frames in flight.

    // images data
    static inline std::vector<VulkanImageData> imagesData {}; ///< Data related to the swapchain images.

    // current frame
    static inline int currentFrame {}; ///< Index of current frame in flight.
    static inline int currentImage {}; ///< Index of current swapchain image.

    // options
    static inline int maxFramesInFlight { 3 }; ///< Number of max frames in flights.
    static inline bool enabledValidationLayer { true }; ///< Enabled state for debug validation layers.

    // debug
    static inline bool debugShowLines { false }; ///< Debug show lines.

    // events
    static inline entt::dispatcher dispatcher {}; ///< Events dispatcher.
};

} // namespace chronicle