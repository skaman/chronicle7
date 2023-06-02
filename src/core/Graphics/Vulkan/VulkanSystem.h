// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

// This must be defined before vulkan.hpp, because vulkan include Windows.h and in Microsoft had the brilliant idea to
// create a macro called "max", that just go in conflict with the entt and standard libraries
#include <entt/entt.hpp>

#include <vulkan/vulkan.hpp>

#include "Common/Common.h"
#include "Graphics/Interfaces/ISystem.h"

namespace chronicle::graphics::internal::vulkan
{

class VulkanSystem : public ISystem<VulkanSystem>, private NonCopyable<VulkanSystem>
{
  public:
    static void init(const SystemInitInfo &systemInitInfo);
    static void deinit();

  private:
    static inline vk::Instance _instance;
    static inline bool _enableDebug;

    [[nodiscard]] static bool checkValidationLayerSupport(const std::vector<const char *> &validationLayers);
    [[nodiscard]] static std::vector<const char *> getRequiredExtensions();
    static void populateDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT &createInfo);
};

} // namespace chronicle::graphics::internal::vulkan