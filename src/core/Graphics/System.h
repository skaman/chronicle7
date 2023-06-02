// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#ifdef CHR_GRAPHICS_VULKAN
#include "Vulkan/VulkanSystem.h"
#endif // CHR_GRAPHICS_VULKAN

namespace chronicle::graphics
{

#ifdef CHR_GRAPHICS_VULKAN
using System = ISystem<internal::vulkan::VulkanSystem>;
#endif // CHR_GRAPHICS_VULKAN

} // namespace chronicle::graphics