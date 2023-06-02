add_subdirectory(Interfaces)
add_subdirectory(Vulkan)

target_sources(chronicle-core
PRIVATE
    "Buffer.h"
    "Common.h"
    "Context.h"
    "Device.h"
    "System.h"
)