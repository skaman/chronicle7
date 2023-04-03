#pragma once

#include "pch.h"

#ifdef VULKAN_RENDERER
#include "Vulkan/VulkanPipeline.h"
#endif

namespace chronicle {

class Renderer;

class Pipeline {
public:
    explicit Pipeline(const Renderer* renderer, const PipelineInfo& pipelineInfo);

#ifdef VULKAN_RENDERER
    [[nodiscard]] inline const VulkanPipeline& native() const { return _pipeline; };
#endif

private:
#ifdef VULKAN_RENDERER
    VulkanPipeline _pipeline;
#endif
};

} // namespace chronicle