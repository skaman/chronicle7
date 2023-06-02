add_subdirectory(Interfaces)
add_subdirectory(Win32)

target_sources(chronicle-core
PRIVATE
    
    "Platform.h"
    "Window.h"
)