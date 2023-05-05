add_executable(chronicle-example
    "main.cc"
)

set_property(TARGET chronicle-example PROPERTY CXX_STANDARD 20)

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