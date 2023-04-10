#pragma once

#include "pch.h"

#include "Renderer/RendererI.h"

#include "VulkanCommon.h"

namespace chronicle {

class App;

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
protected:
    explicit VulkanInstance(chronicle::App* app);

public:
    ~VulkanInstance();

    void waitIdle() const;
    void waitForFence(const FenceRef& fence) const;
    void resetFence(const FenceRef& fence) const;
    uint32_t acquireNextImage(const SemaphoreRef& semaphore);
    void submit(const FenceRef& fence, const SemaphoreRef& waitSemaphore, const SemaphoreRef& signalSemaphore,
        const CommandBufferRef& commandBuffer) const;
    bool present(const SemaphoreRef& waitSemaphore, uint32_t imageIndex);

    void invalidateSwapChain() { _swapChainInvalidated = true; }

    [[nodiscard]] Format swapChainFormat() const { return formatFromVulkan(_swapChainImageFormat); }
    [[nodiscard]] const std::vector<ImageRef>& swapChainImages() const { return _swapChainImages; }
    [[nodiscard]] ExtentInt2D swapChainExtent() const
    {
        return ExtentInt2D(_swapChainExtent.width, _swapChainExtent.height);
    }
    [[nodiscard]] Format depthFormat() const { return formatFromVulkan(_depthImageFormat); }
    [[nodiscard]] const ImageRef& depthImage() const { return _depthImage; }

    [[nodiscard]] const vk::Device& device() const { return _device; }
    [[nodiscard]] const vk::PhysicalDevice& physicalDevice() const { return _physicalDevice; }
    [[nodiscard]] const vk::Queue& graphicsQueue() const { return _graphicsQueue; }
    [[nodiscard]] const vk::CommandPool& commandPool() const { return _commandPool; }

    static RendererUnique create(App* app);

private:
    App* _app;

    vk::Instance _instance;
    VkDebugUtilsMessengerEXT _debugCallback;
    vk::SurfaceKHR _surface;

    vk::PhysicalDevice _physicalDevice;
    vk::Device _device;

    vk::Queue _graphicsQueue;
    vk::Queue _presentQueue;

    vk::SwapchainKHR _swapChain;
    std::vector<ImageRef> _swapChainImages;
    vk::Format _swapChainImageFormat;
    vk::Extent2D _swapChainExtent;
    ImageRef _depthImage;
    vk::Format _depthImageFormat;

    vk::CommandPool _commandPool;

    bool _swapChainInvalidated = false;

    void recreateSwapChain();

    void createInstance();
    void populateDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT& createInfo) const;
    void setupDebugCallback();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSwapChain();
    void createCommandPool();

    [[nodiscard]] bool checkValidationLayerSupport() const;
    [[nodiscard]] std::vector<const char*> getRequiredExtensions() const;
    [[nodiscard]] bool isDeviceSuitable(const vk::PhysicalDevice& physicalDevice) const;
    [[nodiscard]] bool checkDeviceExtensionSupport(const vk::PhysicalDevice& device) const;
    [[nodiscard]] VulkanQueueFamilyIndices findQueueFamilies(vk::PhysicalDevice device) const;
    [[nodiscard]] VulkanSwapChainSupportDetails querySwapChainSupport(const vk::PhysicalDevice& device) const;
    [[nodiscard]] vk::SurfaceFormatKHR chooseSwapSurfaceFormat(
        const std::vector<vk::SurfaceFormatKHR>& availableFormats) const;
    [[nodiscard]] vk::PresentModeKHR chooseSwapPresentMode(
        const std::vector<vk::PresentModeKHR>& availablePresentModes) const;
    [[nodiscard]] vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) const;
    [[nodiscard]] vk::Format findSupportedFormat(
        const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features) const;
    [[nodiscard]] vk::Format findDepthFormat() const;
    [[nodiscard]] bool hasStencilComponent(vk::Format format) const;
};

} // namespace chronicle