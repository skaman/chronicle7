// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Renderer/Common.h"
#include "Renderer/RendererError.h"

namespace chronicle {

inline vk::ShaderStageFlagBits shaderStageToVulkan(ShaderStage stage)
{
    switch (stage) {
    case ShaderStage::Fragment:
        return vk::ShaderStageFlagBits::eFragment;
    case ShaderStage::Vertex:
        return vk::ShaderStageFlagBits::eVertex;
    default:
        throw RendererError("Unsupported shader stage");
    }
}

inline vk::Format formatToVulkan(Format format)
{
    switch (format) {
    case Format::Undefined:
        return vk::Format::eUndefined;
    case Format::B8G8R8A8Unorm:
        return vk::Format::eB8G8R8A8Unorm;
    case Format::R32G32Sfloat:
        return vk::Format::eR32G32Sfloat;
    case Format::R32G32B32Sfloat:
        return vk::Format::eR32G32B32Sfloat;
    case Format::D32Sfloat:
        return vk::Format::eD32Sfloat;
    case Format::D32SfloatS8Uint:
        return vk::Format::eD32SfloatS8Uint;
    case Format::D24UnormS8Uint:
        return vk::Format::eD24UnormS8Uint;
    default:
        throw RendererError("Unsupported format");
    }
}

inline Format formatFromVulkan(vk::Format format)
{
    switch (format) {
    case vk::Format::eUndefined:
        return Format::Undefined;
    case vk::Format::eB8G8R8A8Unorm:
        return Format::B8G8R8A8Unorm;
    case vk::Format::eR32G32Sfloat:
        return Format::R32G32Sfloat;
    case vk::Format::eR32G32B32Sfloat:
        return Format::R32G32B32Sfloat;
    case vk::Format::eD32Sfloat:
        return Format::D32Sfloat;
    case vk::Format::eD32SfloatS8Uint:
        return Format::D32SfloatS8Uint;
    case vk::Format::eD24UnormS8Uint:
        return Format::D24UnormS8Uint;
    default:
        throw RendererError("Unsupported format");
    }
}

struct DebugShowLinesEvent { };

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

struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

struct VulkanContext {
    // instance, debugger and surface
    static inline vk::Instance instance = nullptr;
    static inline VkDebugUtilsMessengerEXT debugCallback = VK_NULL_HANDLE;
    static inline vk::SurfaceKHR surface = nullptr;

    // devices
    static inline vk::PhysicalDevice physicalDevice = nullptr;
    static inline vk::Device device = nullptr;

    // queues
    static inline vk::Queue graphicsQueue = nullptr;
    static inline vk::Queue presentQueue = nullptr;

    // families
    static inline uint32_t graphicsFamily = 0;
    static inline uint32_t presentFamily = 0;

    // swapchain
    static inline vk::SwapchainKHR swapChain = nullptr;
    static inline std::vector<vk::Image> swapChainImages = {};
    static inline std::vector<vk::ImageView> swapChainImageViews = {};
    static inline vk::Format swapChainImageFormat = vk::Format::eUndefined;
    static inline vk::Extent2D swapChainExtent = {};
    static inline bool swapChainInvalidated = false;

    // depth image
    static inline vk::Image depthImage = nullptr;
    static inline vk::DeviceMemory depthImageMemory = nullptr;
    static inline vk::ImageView depthImageView = nullptr;
    static inline vk::Format depthImageFormat = vk::Format::eUndefined;

    // command pool
    static inline vk::CommandPool commandPool = nullptr;

    // draw pass
    static inline vk::RenderPass renderPass = nullptr;
    static inline vk::RenderPass debugRenderPass = nullptr;

    // framebuffers
    static inline std::vector<vk::Framebuffer> framebuffers = {};
    static inline std::vector<vk::Framebuffer> debugFramebuffers = {};

    // multisampling
    static inline vk::SampleCountFlagBits msaaSamples = vk::SampleCountFlagBits::e1;
    static inline vk::Image colorImage = nullptr;
    static inline vk::DeviceMemory colorImageMemory = nullptr;
    static inline vk::ImageView colorImageView = nullptr;

    // sync objects
    static inline std::vector<vk::Semaphore> imageAvailableSemaphores = {};
    static inline std::vector<vk::Semaphore> renderFinishedSemaphores = {};
    static inline std::vector<vk::Fence> inFlightFences = {};

    // command buffers
    static inline std::vector<CommandBufferRef> commandBuffers = {};

    // descriptor sets
    static inline vk::DescriptorPool descriptorPool = nullptr; ///< Descriptor pool used to allocate resources.
    static inline std::vector<DescriptorSetRef> descriptorSets = {};

    // current frame
    static inline int currentFrame = 0;
    static inline int currentImage = 0;

    // options
    static inline int maxFramesInFlight = 3;
    static inline bool enabledValidationLayer = true;

    // debug
    static inline bool debugShowLines = false;

    // events
    static inline entt::dispatcher dispatcher = {};
};

} // namespace chronicle