find_package(Vulkan REQUIRED COMPONENTS glslc shaderc_combined)
find_package(Python REQUIRED COMPONENTS Interpreter)
find_program(glslc_executable NAMES glslc HINTS Vulkan::glslc)

function(embed_asset target)
    cmake_parse_arguments(PARSE_ARGV 1 arg "" "" "SOURCES")
    foreach(source ${arg_SOURCES})
        # Get short filename
        string(REGEX MATCH "([^/]+)$" filename ${source})
        # Replace filename spaces & extension separator for C compatibility
        string(REGEX REPLACE "\\.| |-" "_" filename ${filename})

        set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/foo.cc PROPERTIES GENERATED 1)
        add_custom_command(
            OUTPUT ${filename}.cc
            DEPENDS ${source}
            COMMAND
                ${Python_EXECUTABLE}
                "${CMAKE_CURRENT_SOURCE_DIR}/../../embed_file.py"
                ${filename}
                "${CMAKE_CURRENT_SOURCE_DIR}/${source}"
            COMMENT "Embedding asset ${source}"
        )
        target_sources(${target} PRIVATE "${filename}.cc")
    endforeach()
endfunction()

add_library(chronicle-core "")

add_subdirectory(Assets)
add_subdirectory(Loaders)
add_subdirectory(Platform)
add_subdirectory(Renderer)
add_subdirectory(Storage)
add_subdirectory(Utils)

target_sources(chronicle-core
PRIVATE
    "pch.cpp"

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
        RENDERER=VULKAN
        PLATFORM=GLFW
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
        "pch.h"
)

embed_asset(chronicle-core
    SOURCES
        "bfont.ttf"
        "Shaders/test.hlslh"
        "Shaders/MaterialPbr.hlsl"
        "Shaders/MaterialPbr.glsl"
)

if(WIN32)
    target_compile_definitions(chronicle-core PUBLIC CHRPLATFORM_WINDOWS)
elseif(APPLE)
    target_compile_definitions(chronicle-core PUBLIC CHRPLATFORM_MACOS)
elseif(UNIX)
    target_compile_definitions(chronicle-core PUBLIC CHRPLATFORM_LINUX)
endif()

target_link_libraries(chronicle-core
    PUBLIC
        glfw
        Vulkan::Headers
        Vulkan::Vulkan
        Vulkan::shaderc_combined
        EnTT::EnTT
        spdlog::spdlog
        glm::glm
        tinygltf
        tinyobjloader
        Tracy::TracyClient
        spirv-reflect-static
        magic_enum::magic_enum
)