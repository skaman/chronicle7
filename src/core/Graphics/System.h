// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#ifdef CHR_GRAPHICS_VULKAN
#include "Vulkan/VulkanSystem.h"
#endif // CHR_GRAPHICS_VULKAN

#include "Common.h"
#include "Device.h"

#include <string>

namespace chronicle::graphics
{

/// @brief Handle base resources for the graphic system.
/// @tparam T Specialization of the graphic system.
class System
{
  public:
    /// @brief Initialize the graphic system.
    /// @param systemInitInfo Informations used to initialize the system.
    static void init(const SystemInitInfo &systemInitInfo)
    {
        return internal::vulkan::VulkanSystem::init(systemInitInfo);
    }

    /// @brief Deinitialize the graphic system.
    static void deinit()
    {
        return internal::vulkan::VulkanSystem::deinit();
    }

    /// @brief Request a graphic device.
    /// @param requestDeviceInfo Informations used to request and identity the right graphic device.
    /// @return Graphic device.
    [[nodiscard]] static std::shared_ptr<Device> requestDevice(const RequestDeviceInfo &requestDeviceInfo)
    {
        return internal::vulkan::VulkanSystem::requestDevice(requestDeviceInfo);
    }
};

} // namespace chronicle::graphics