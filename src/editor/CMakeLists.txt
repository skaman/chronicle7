find_package(Python REQUIRED COMPONENTS Interpreter)

add_executable(chronicle-editor
    "Widgets/DockArea.cpp"
    "Widgets/DockArea.h"
    "Widgets/DockPanel.cpp"
    "Widgets/DockPanel.h"
    "main.cpp"
    "MainWindow.cpp"
    "MainWindow.h"
    "Resources.h"
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

embed_asset(chronicle-editor
    SOURCES
        "Resources/Icons/WindowClose.png"
        "Resources/Icons/WindowMaximize.png"
        "Resources/Icons/WindowMinimize.png"
        "Resources/Icons/WindowRestore.png"
)

target_link_libraries(chronicle-editor
    PUBLIC
        chronicle::core
)