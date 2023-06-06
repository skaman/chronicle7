// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "Common/Common.h"
#include "Graphics/Common.h"
#include "Graphics/Device.h"

#include <vulkan/vulkan.hpp>

namespace chronicle::graphics::internal::vulkan
{

/// @brief Data structure for find queue families.
struct VulkanQueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily{}; /// Graphics family index.
    std::optional<uint32_t> presentFamily{};  /// Present family index.

    /// @brief Check if families are setted.
    /// @return True if all families are setted, otherwise false.
    [[nodiscard]] bool IsComplete() const
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

/// @brief Data structure for find swapchain support.
struct VulkanSwapChainSupportDetails
{
    vk::SurfaceCapabilitiesKHR capabilities{};      /// Swapchain capabilities.
    std::vector<vk::SurfaceFormatKHR> formats{};    /// Swapchain formats.
    std::vector<vk::PresentModeKHR> presentModes{}; /// Present modes.
};

/// @brief Vulkan graphic system specialization.
class VulkanSystem
{
  public:
    /// @see ISystem#init
    static void init(const SystemInitInfo &systemInitInfo);

    /// @see ISystem#deinit
    static void deinit();

    /// @see ISystem#device
    [[nodiscard]] static std::shared_ptr<Device> requestDevice(const RequestDeviceInfo &requestDeviceInfo);

    /// @brief Get the enabled extensions required for initialize a vulkan device.
    /// @return Enabled extensions.
    [[nodiscard]] static std::vector<const char *> enabledExtensions();

    /// @brief Get the enabled layers required for initialize a vulkan device.
    /// @return Enabled layers.
    [[nodiscard]] static std::vector<const char *> enabledLayers();

    /// @brief Get the vulkan instance.
    /// @return Vulkan instance.
    [[nodiscard]] static vk::Instance vulkanInstance()
    {
        return _instance;
    }

  private:
    static inline vk::Instance _instance; /// Vulkan instance.
    static inline bool _enableDebug;      /// Graphic debug enable.

    /// @brief Check if the validation layer support is available.
    /// @param validationLayers Validation layers to check.
    /// @return True if they are supported, otherwise false.
    [[nodiscard]] static bool checkValidationLayerSupport(const std::vector<const char *> &validationLayers);

    /// @brief Get required extensions.
    /// @return Required extensions.
    [[nodiscard]] static std::vector<const char *> getRequiredExtensions();

    /// @brief Populate the debug structure creation info.
    /// @param createInfo Creation info data structure.
    static void populateDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT &createInfo);

    /// @brief Check if the physical device is suitable.
    /// @param physicalDevice Physical device.
    /// @param surface Surface.
    /// @param extensions Required extensions.
    /// @return True if the device is suitable, otherwise false.
    [[nodiscard]] static bool isDeviceSuitable(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface,
                                               const std::vector<const char *> &extensions);

    /// @brief Find queues families.
    /// @param physicalDevice Physical device.
    /// @param surface Surface.
    /// @return Queue family indices.
    [[nodiscard]] static VulkanQueueFamilyIndices findQueueFamilies(vk::PhysicalDevice physicalDevice,
                                                                    vk::SurfaceKHR surface);

    /// @brief Check device extensions support.
    /// @param physicalDevice Physical device.
    /// @param extensions Device extensions.
    /// @return True if are supported, otherwise false.
    [[nodiscard]] static bool checkDeviceExtensionSupport(vk::PhysicalDevice physicalDevice,
                                                          const std::vector<const char *> &extensions);

    /// @brief Query swapchain support details.
    /// @param physicalDevice Physical device.
    /// @param surface Surface.
    /// @return Swapchain support details.
    [[nodiscard]] static VulkanSwapChainSupportDetails querySwapChainSupport(vk::PhysicalDevice physicalDevice,
                                                                             vk::SurfaceKHR surface);
};

} // namespace chronicle::graphics::internal::vulkan