find_package(Vulkan REQUIRED COMPONENTS shaderc_combined SPIRV-Tools)

add_library(chronicle-core "")

add_subdirectory(Common)
add_subdirectory(Graphics)
add_subdirectory(Platform)

target_sources(chronicle-core
PRIVATE
    "Chronicle.cpp"
    "Chronicle.h"
    
    # IMGUI
    "../../vendor/imgui/imgui.cpp"
    "../../vendor/imgui/imgui_demo.cpp"
    "../../vendor/imgui/imgui_draw.cpp"
    "../../vendor/imgui/imgui_tables.cpp"
    "../../vendor/imgui/imgui_widgets.cpp"
    "../../vendor/imgui/backends/imgui_impl_glfw.cpp"
    "../../vendor/imgui/backends/imgui_impl_vulkan.cpp"
)

add_library(chronicle::core ALIAS chronicle-core)

set_property(TARGET chronicle-core PROPERTY CXX_STANDARD 20)

target_compile_definitions(chronicle-core
    PUBLIC
        CHR_GRAPHICS_VULKAN
    PRIVATE
        $<$<CONFIG:Debug>:VULKAN_ENABLE_DEBUG_MARKER>
)

#IF(CMAKE_BUILD_TYPE MATCHES DEBUG)
#    message("debug mode")
#ENDIF(CMAKE_BUILD_TYPE MATCHES DEBUG)

target_include_directories(chronicle-core    
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        "${CMAKE_CURRENT_SOURCE_DIR}/../../vendor"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../vendor/imgui"
)

target_precompile_headers(chronicle-core
    PUBLIC
        "Common/Common.h"

        <string>
        <memory>
        <stdexcept>
        <algorithm>
        <vector>
        <unordered_map>
        <vulkan/vulkan.hpp>
        <spdlog/spdlog.h>
)

#target_precompile_headers(chronicle-core
#    PUBLIC
#        "pch.h"
#)

#embed_asset(chronicle-core
#    SOURCES
#        "bfont.ttf"
#        "Shaders/test.hlslh"
#        "Shaders/MaterialPbr.hlsl"
#        "Shaders/MaterialPbr.glsl"
#)

if(WIN32)
    target_compile_definitions(chronicle-core PUBLIC CHR_PLATFORM_WIN32 VK_USE_PLATFORM_WIN32_KHR )
elseif(APPLE)
    target_compile_definitions(chronicle-core PUBLIC CHR_PLATFORM_MACOS)
elseif(UNIX)
    target_compile_definitions(chronicle-core PUBLIC CHR_PLATFORM_LINUX)
endif()

# Add SPIRV-Cross from local vulkan SDK
cmake_path(GET Vulkan_SPIRV-Tools_LIBRARY PARENT_PATH Vulkan_LIBRARY_PATH)
add_library(Vulkan::SPIRV-Cross STATIC IMPORTED)
set_target_properties(Vulkan::SPIRV-Cross
    PROPERTIES
        IMPORTED_LOCATION_DEBUG "${Vulkan_LIBRARY_PATH}/spirv-cross-cored.lib"
        IMPORTED_LOCATION "${Vulkan_LIBRARY_PATH}/spirv-cross-core.lib"
)

target_link_libraries(chronicle-core
    PUBLIC
        glfw
        Vulkan::Headers
        Vulkan::Vulkan
        Vulkan::shaderc_combined
        Vulkan::SPIRV-Tools
        Vulkan::SPIRV-Cross
        EnTT::EnTT
        spdlog::spdlog
        glm::glm
        tinygltf
        tinyobjloader
        Tracy::TracyClient
        magic_enum::magic_enum
        efsw
)