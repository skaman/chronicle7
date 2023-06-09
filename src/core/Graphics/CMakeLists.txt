add_subdirectory(Vulkan)

target_sources(chronicle-core
PRIVATE
    "Buffer.h"
    "CommandEncoder.h"
    "Common.h"
    "Device.h"
    "System.h"
    "Texture.h"
    "TextureView.h"
)