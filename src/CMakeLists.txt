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
    "Platform/App.h"
    "Platform/App.cc"
    "Renderer/Renderer.h"
    "Renderer/Renderer.cc"
    "Systems/MeshRendererSystem.h"
    "Systems/MeshRendererSystem.cc"
    "Systems/Systems.h"
    "Systems/System.h" )

compile_shader(chronicle
    ENV vulkan1.1
    FORMAT bin
    SOURCES
        shaders/triangle.vert
        shaders/triangle.frag
)

set_property(TARGET chronicle PROPERTY CXX_STANDARD 20)

target_include_directories(chronicle
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
    INTERFACE
        ${CMAKE_SOURCE_DIR}/include
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
)