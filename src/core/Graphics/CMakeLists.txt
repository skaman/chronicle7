add_subdirectory(Vulkan)

target_sources(chronicle-core
PRIVATE
    "BindGroup.h"
    "BindGroupLayout.h"
    "Buffer.h"
    "CommandEncoder.h"
    "Common.h"
    "Device.h"
    "PipelineLayout.h"
    "Sampler.h"
    "ShaderModule.h"
    "System.h"
    "Texture.h"
    "TextureView.h"
)