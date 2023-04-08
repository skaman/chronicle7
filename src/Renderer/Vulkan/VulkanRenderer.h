#pragma once

#include "pch.h"

#include "VulkanCommon.h"

namespace chronicle {

class App;
class Image;
class Fence;
class Semaphore;
class CommandBuffer;

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

class VulkanRenderer {
public:
    explicit VulkanRenderer(chronicle::App* app);
    ~VulkanRenderer();

    void waitIdle() const;
    void waitForFence(const std::shared_ptr<Fence>& fence) const;
    void resetFence(const std::shared_ptr<Fence>& fence) const;
    uint32_t acquireNextImage(const std::shared_ptr<Semaphore>& semaphore);
    void submit(const std::shared_ptr<Fence>& fence, const std::shared_ptr<Semaphore>& waitSemaphore,
        const std::shared_ptr<Semaphore>& signalSemaphore, const std::shared_ptr<CommandBuffer>& commandBuffer) const;
    bool present(const std::shared_ptr<Semaphore>& waitSemaphore, uint32_t imageIndex);

    void invalidateSwapChain() { _swapChainInvalidated = true; }

    [[nodiscard]] inline Format swapChainFormat() const { return formatFromVulkan(_swapChainImageFormat); }
    [[nodiscard]] inline const std::vector<std::shared_ptr<Image>>& swapChainImages() const { return _swapChainImages; }
    [[nodiscard]] inline ExtentInt2D swapChainExtent() const
    {
        return ExtentInt2D(_swapChainExtent.width, _swapChainExtent.height);
    }

    // internal
    [[nodiscard]] inline const vk::Device& device() const { return _device; }
    [[nodiscard]] inline const vk::PhysicalDevice& physicalDevice() const { return _physicalDevice; }
    [[nodiscard]] inline const vk::Queue& graphicsQueue() const { return _graphicsQueue; }
    [[nodiscard]] inline const vk::CommandPool& commandPool() const { return _commandPool; }

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
    std::vector<std::shared_ptr<Image>> _swapChainImages;
    vk::Format _swapChainImageFormat;
    vk::Extent2D _swapChainExtent;

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
};

} // namespace chronicle