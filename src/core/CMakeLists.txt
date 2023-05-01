find_package(Vulkan REQUIRED COMPONENTS glslc shaderc_combined)
find_package(Python REQUIRED COMPONENTS Interpreter)
find_program(glslc_executable NAMES glslc HINTS Vulkan::glslc)

# https://stackoverflow.com/questions/60420700/cmake-invocation-of-glslc-with-respect-to-includes-dependencies
function(compile_shader target)
    cmake_parse_arguments(PARSE_ARGV 1 arg "" "ENV;FORMAT;LANGUAGE;STAGE" "SOURCES")
    foreach(source ${arg_SOURCES})
        string(REGEX REPLACE "\\.[^.]*$" "" outputname ${source})
        add_custom_command(
            OUTPUT ${source}.${arg_FORMAT}
            DEPENDS ${source}
            DEPFILE ${source}.d
            COMMAND
                ${glslc_executable}
                $<$<BOOL:${arg_ENV}>:--target-env=${arg_ENV}>
                $<$<BOOL:${arg_FORMAT}>:-mfmt=${arg_FORMAT}>
                $<$<BOOL:${arg_LANGUAGE}>:-x${arg_LANGUAGE}>
                $<$<BOOL:${arg_STAGE}>:-fshader-stage=${arg_STAGE}>
                -MD -MF ${source}.d
                -o ${outputname}.${arg_FORMAT}
                ${CMAKE_CURRENT_SOURCE_DIR}/${source}
            COMMENT "Compiling shader ${source} ${outputname}"
        )
        target_sources(${target} PRIVATE ${source}.${arg_FORMAT})
    endforeach()
endfunction()

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

add_library(chronicle-core
    "pch.cc"
    "Assets/AssetLoader.h"
    "Assets/AssetLoader.cc"
    "Assets/MeshAsset.h"
    "Assets/MeshAsset.cc"
    "Assets/ShaderAsset.h"
    "Assets/ShaderAsset.cc"
    "Assets/TextureAsset.h"
    "Assets/TextureAsset.cc"
    "Platform/Platform.h"
    "Platform/PlatformI.h"
    "Platform/Common.h"
    "Platform/GLFW/GLFWPlatform.h"
    "Platform/GLFW/GLFWPlatform.cc"
    "Platform/GLFW/GLFWCommon.h"
    "Renderer/CommandBufferI.h"
    "Renderer/Common.h"
    "Renderer/DescriptorSetI.h"
    "Renderer/Enums.h"
    "Renderer/IndexBufferI.h"
    "Renderer/MaterialI.h"
    "Renderer/MeshI.h"
    "Renderer/PipelineI.h"
    "Renderer/PipelineInfo.h"
    "Renderer/Renderer.h"
    "Renderer/RendererError.h"
    "Renderer/RendererI.h"
    "Renderer/ShaderCompilerI.h"
    "Renderer/ShaderI.h"
    "Renderer/TextureI.h"
    "Renderer/TextureInfo.h"
    "Renderer/VertexBufferI.h"
    "Renderer/VertexBufferInfo.h"
    "Renderer/Vulkan/VulkanCommandBuffer.h"
    "Renderer/Vulkan/VulkanCommandBuffer.cc"
    "Renderer/Vulkan/VulkanCommon.h"
    "Renderer/Vulkan/VulkanDescriptorSet.h"
    "Renderer/Vulkan/VulkanDescriptorSet.cc"
    "Renderer/Vulkan/VulkanEnums.h"
    "Renderer/Vulkan/VulkanEvents.h"
    "Renderer/Vulkan/VulkanExtensions.h"
    "Renderer/Vulkan/VulkanImGui.h"
    "Renderer/Vulkan/VulkanImGui.cc"
    "Renderer/Vulkan/VulkanIndexBuffer.h"
    "Renderer/Vulkan/VulkanIndexBuffer.cc"
    "Renderer/Vulkan/VulkanInstance.h"
    "Renderer/Vulkan/VulkanInstance.cc"
    "Renderer/Vulkan/VulkanMaterial.h"
    "Renderer/Vulkan/VulkanMaterial.cc"
    "Renderer/Vulkan/VulkanMesh.h"
    "Renderer/Vulkan/VulkanMesh.cc"
    "Renderer/Vulkan/VulkanPipeline.h"
    "Renderer/Vulkan/VulkanPipeline.cc"
    "Renderer/Vulkan/VulkanRenderer.h"
    "Renderer/Vulkan/VulkanRenderer.cc"
    "Renderer/Vulkan/VulkanShaderCompiler.h"
    "Renderer/Vulkan/VulkanShaderCompiler.cc"
    "Renderer/Vulkan/VulkanShader.h"
    "Renderer/Vulkan/VulkanShader.cc"
    "Renderer/Vulkan/VulkanTexture.h"
    "Renderer/Vulkan/VulkanTexture.cc"
    "Renderer/Vulkan/VulkanUtils.h"
    "Renderer/Vulkan/VulkanUtils.cc"
    "Renderer/Vulkan/VulkanVertexBuffer.h"
    "Renderer/Vulkan/VulkanVertexBuffer.cc"
    "Storage/Storage.h"
    "Storage/Storage.cc"
    "Utils/Camera.h"
    "Utils/Camera.cc"

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
        "Shaders/MaterialPbr.hlsl"
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