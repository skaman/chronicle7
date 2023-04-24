add_executable(chronicle-example
    "main.cc")

set_property(TARGET chronicle-example PROPERTY CXX_STANDARD 20)

compile_shader(chronicle-example
    ENV vulkan1.1
    FORMAT bin
    LANGUAGE hlsl
    STAGE vertex
    SOURCES
        Shaders/triangle.vert.hlsl
)

compile_shader(chronicle-example
    ENV vulkan1.1
    FORMAT bin
    LANGUAGE hlsl
    STAGE fragment
    SOURCES
        Shaders/triangle.frag.hlsl
)

target_include_directories(chronicle-example
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_precompile_headers(chronicle-example
  PUBLIC
    "pch.h"
)

target_link_libraries(chronicle-example
    PUBLIC
        chronicle::core
)