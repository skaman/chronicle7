find_package(Vulkan REQUIRED COMPONENTS glslc)
find_program(glslc_executable NAMES glslc HINTS Vulkan::glslc)

# https://stackoverflow.com/questions/60420700/cmake-invocation-of-glslc-with-respect-to-includes-dependencies
function(compile_shader target)
    cmake_parse_arguments(PARSE_ARGV 1 arg "" "ENV;FORMAT" "SOURCES")
    foreach(source ${arg_SOURCES})
        add_custom_command(
            OUTPUT ${source}.${arg_FORMAT}
            DEPENDS ${source}
            DEPFILE ${source}.d
            COMMAND
                ${glslc_executable}
                $<$<BOOL:${arg_ENV}>:--target-env=${arg_ENV}>
                $<$<BOOL:${arg_FORMAT}>:-mfmt=${arg_FORMAT}>
                -MD -MF ${source}.d
                -o ${source}.${arg_FORMAT}
                ${CMAKE_CURRENT_SOURCE_DIR}/${source}
            COMMENT "Compiling shader ${source}"
        )
        target_sources(${target} PRIVATE ${source}.${arg_FORMAT})
    endforeach()
endfunction()

add_library(chronicle-core
    "pch.cc"
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
    "Renderer/FenceI.h"
    "Renderer/ImageI.h"
    "Renderer/ImageInfo.h"
    "Renderer/IndexBufferI.h"
    "Renderer/PipelineI.h"
    "Renderer/PipelineInfo.h"
    "Renderer/Renderer.h"
    "Renderer/RendererI.h"
    "Renderer/RendererError.h"
    "Renderer/RenderPassI.h"
    "Renderer/RenderPassInfo.h"
    "Renderer/SemaphoreI.h"
    "Renderer/VertexBufferI.h"
    "Renderer/VertexBufferInfo.h"
    "Renderer/Vulkan/VulkanCommandBuffer.h"
    "Renderer/Vulkan/VulkanCommandBuffer.cc"
    "Renderer/Vulkan/VulkanCommon.h"
    "Renderer/Vulkan/VulkanDescriptorSet.h"
    "Renderer/Vulkan/VulkanDescriptorSet.cc"
    "Renderer/Vulkan/VulkanFence.h"
    "Renderer/Vulkan/VulkanFence.cc"
    "Renderer/Vulkan/VulkanImage.h"
    "Renderer/Vulkan/VulkanImage.cc"
    "Renderer/Vulkan/VulkanIndexBuffer.h"
    "Renderer/Vulkan/VulkanIndexBuffer.cc"
    "Renderer/Vulkan/VulkanInstance.h"
    "Renderer/Vulkan/VulkanInstance.cc"
    "Renderer/Vulkan/VulkanPipeline.h"
    "Renderer/Vulkan/VulkanPipeline.cc"
    "Renderer/Vulkan/VulkanRenderer.h"
    "Renderer/Vulkan/VulkanRenderer.cc"
    "Renderer/Vulkan/VulkanRenderPass.h"
    "Renderer/Vulkan/VulkanRenderPass.cc"
    "Renderer/Vulkan/VulkanSemaphore.h"
    "Renderer/Vulkan/VulkanSemaphore.cc"
    "Renderer/Vulkan/VulkanUtils.h"
    "Renderer/Vulkan/VulkanUtils.cc"
    "Renderer/Vulkan/VulkanVertexBuffer.h"
    "Renderer/Vulkan/VulkanVertexBuffer.cc"
    "Storage/File.h"
    "Storage/File.cc"
)

add_library(chronicle::core ALIAS chronicle-core)

set_property(TARGET chronicle-core PROPERTY CXX_STANDARD 20)

target_compile_definitions(chronicle-core
    PUBLIC
        RENDERER=VULKAN
        PLATFORM=GLFW
)

target_include_directories(chronicle-core    
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        "${CMAKE_CURRENT_SOURCE_DIR}/../../vendor"
)

target_precompile_headers(chronicle-core
  PUBLIC
    "pch.h"
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
        EnTT::EnTT
        spdlog::spdlog
        glm::glm
        tinygltf
        tinyobjloader
        Tracy::TracyClient
        spirv-reflect-static
)