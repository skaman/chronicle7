#pragma once

#include "pch.h"

#include "RenderPassInfo.h"

#ifdef VULKAN_RENDERER
#include "Vulkan/VulkanRenderPass.h"
#endif

namespace chronicle {

class Renderer;
struct RenderPassInfo;

class RenderPass {
public:
    explicit RenderPass(const Renderer* renderer, const RenderPassInfo& renderPassInfo);

#ifdef VULKAN_RENDERER
    [[nodiscard]] inline const VulkanRenderPass& native() const { return _renderPass; };
#endif

private:
#ifdef VULKAN_RENDERER
    VulkanRenderPass _renderPass;
#endif
};

} // namespace chronicle