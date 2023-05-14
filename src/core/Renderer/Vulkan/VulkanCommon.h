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

/// @brief Entry types for garbage collector.
enum class GarbageType { pipeline, pipelineLayout, buffer, deviceMemory, descriptorSetLayout };

/// @brief Garbage collector data.
struct GarbageCollectorData {
    GarbageType type; ///< Entry type.
    union {
        vk::Pipeline pipeline; ///< Pipeline
        vk::PipelineLayout pipelineLayout; ///< Pipeline layout
        vk::Buffer buffer; ///< Buffer
        vk::DeviceMemory deviceMemory; ///< Device memory
        vk::DescriptorSetLayout descriptorSetLayout; ///< Descriptor set layout
    };

    explicit GarbageCollectorData(vk::Pipeline pipeline)
        : type(GarbageType::pipeline)
        , pipeline(pipeline)
    {
    }

    explicit GarbageCollectorData(vk::PipelineLayout pipelineLayout)
        : type(GarbageType::pipelineLayout)
        , pipelineLayout(pipelineLayout)
    {
    }

    explicit GarbageCollectorData(vk::Buffer buffer)
        : type(GarbageType::buffer)
        , buffer(buffer)
    {
    }

    explicit GarbageCollectorData(vk::DeviceMemory deviceMemory)
        : type(GarbageType::deviceMemory)
        , deviceMemory(deviceMemory)
    {
    }

    explicit GarbageCollectorData(vk::DescriptorSetLayout descriptorSetLayout)
        : type(GarbageType::descriptorSetLayout)
        , descriptorSetLayout(descriptorSetLayout)
    {
    }
};

/// @brief Data related to a single frame in flight.
struct VulkanFrameData {
    // sync objects
    vk::Semaphore imageAvailableSemaphore = nullptr; ///< Image available semaphore.
    vk::Semaphore renderFinishedSemaphore = nullptr; ///< Render finished semaphore.
    vk::Fence inFlightFence = nullptr; ///< Fence for frames in flight.

    // command buffers
    CommandBufferRef commandBuffer = nullptr; ///< Command Buffer.

    // descriptor sets
    DescriptorSetRef descriptorSet = nullptr; ///< Descriptor set.

    // resource to be destroyed
    std::vector<GarbageCollectorData> garbageCollector = {}; ///< Garbage collector data.
};

/// @brief Data related to a single swapchain image.
struct VulkanImageData {
    // swapchain
    TextureRef swapChainTexture = {}; ///< Swapchain texture.

    // framebuffers
    FrameBufferRef framebuffer = nullptr; ///< Framebuffer main render pass.
    FrameBufferRef debugFramebuffer = nullptr; ///< Framebuffer debug render pass.
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
    static inline vk::Format swapChainImageFormat = vk::Format::eUndefined; ///< Swapchain image format.
    static inline vk::Extent2D swapChainExtent = {}; ///< Swapchain extent.
    static inline bool swapChainInvalidated = false; ///< Indicate if the swapchain is invalidated and need recreation.

    // multisampling
    static inline vk::SampleCountFlagBits msaaSamples = vk::SampleCountFlagBits::e1; ///< Number of samples.
    static inline TextureRef colorTexture = {}; ///< Multisampling color image.

    // depth image
    static inline vk::Format depthImageFormat = vk::Format::eUndefined; ///< Depth image format.
    static inline TextureRef depthTexture = {}; ///< Depth image.

    // command pool
    static inline vk::CommandPool commandPool = nullptr; ///< Command pool.

    // draw pass
    static inline RenderPassRef renderPass = {}; ///< Main render pass.
    static inline RenderPassRef debugRenderPass = {}; ///< Debug render pass.

    // descriptor sets
    static inline vk::DescriptorPool descriptorPool = nullptr; ///< Descriptor pool used to allocate resources.

    // frame data
    static inline std::vector<VulkanFrameData> framesData = {}; ///< Data related to the frames in flight.

    // images data
    static inline std::vector<VulkanImageData> imagesData = {}; ///< Data related to the swapchain images.

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