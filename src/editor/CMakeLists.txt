add_executable(chronicle-editor
    "main.cc"
)

set_property(TARGET chronicle-editor PROPERTY CXX_STANDARD 20)

target_include_directories(chronicle-editor
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_precompile_headers(chronicle-editor
  PUBLIC
    "pch.h"
)

target_link_libraries(chronicle-editor
    PUBLIC
        chronicle::core
)