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

add_executable(chronicle
    "main.cc"
    "pch.cc"
    "Platform/App.h"
    "Platform/App.cc"
    "Renderer/CommandBuffer.h"
    "Renderer/Common.h"
    "Renderer/DescriptorSet.h"
    "Renderer/Fence.h"
    "Renderer/Image.h"
    "Renderer/ImageInfo.h"
    "Renderer/IndexBuffer.h"
    "Renderer/Pipeline.h"
    "Renderer/PipelineInfo.h"
    "Renderer/Renderer.h"
    "Renderer/Renderer.cc"
    "Renderer/RendererError.h"
    "Renderer/RenderPass.h"
    "Renderer/RenderPassInfo.h"
    "Renderer/Semaphore.h"
    "Renderer/VertexBuffer.h"
    "Renderer/VertexBufferInfo.h"
    "Renderer/Vulkan/VulkanBuffer.h"
    "Renderer/Vulkan/VulkanBuffer.cc"
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
    "Renderer/Vulkan/VulkanPipeline.h"
    "Renderer/Vulkan/VulkanPipeline.cc"
    "Renderer/Vulkan/VulkanRenderer.h"
    "Renderer/Vulkan/VulkanRenderer.cc"
    "Renderer/Vulkan/VulkanRenderPass.h"
    "Renderer/Vulkan/VulkanRenderPass.cc"
    "Renderer/Vulkan/VulkanVertexBuffer.h"
    "Renderer/Vulkan/VulkanVertexBuffer.cc"
    "Renderer/Vulkan/VulkanSemaphore.h"
    "Renderer/Vulkan/VulkanSemaphore.cc"
    "Storage/File.h"
    "Storage/File.cc"
    "Systems/MeshRendererSystem.h"
    "Systems/MeshRendererSystem.cc"
    "Systems/Systems.h"
    "Systems/System.h")

compile_shader(chronicle
    ENV vulkan1.1
    FORMAT bin
    SOURCES
        shaders/triangle.vert
        shaders/triangle.frag
)

set_property(TARGET chronicle PROPERTY CXX_STANDARD 20)

target_compile_definitions(chronicle PUBLIC RENDERER=VULKAN)

target_include_directories(chronicle    
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        "${CMAKE_CURRENT_SOURCE_DIR}/../vendor"
)

target_precompile_headers(chronicle
  PUBLIC
    "pch.h"
)

target_link_libraries(chronicle
    PUBLIC
        glfw
        Vulkan::Headers
        Vulkan::Vulkan
        EnTT::EnTT
        spdlog::spdlog
        glm::glm
        tinygltf
        Tracy::TracyClient
        spirv-reflect-static
)