# Add SPDLOG
set(SPDLOG_NO_EXCEPTIONS OFF CACHE BOOL "" FORCE)
add_subdirectory(spdlog EXCLUDE_FROM_ALL)

# Add ENTT
add_subdirectory(entt EXCLUDE_FROM_ALL)

# Add GLM
set(BUILD_STATIC_LIBS ON CACHE BOOL "" FORCE)
set(GLM_TEST_ENABLE_CXX_20 ON CACHE BOOL "" FORCE)
add_subdirectory(glm EXCLUDE_FROM_ALL)

# Add GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(glfw EXCLUDE_FROM_ALL)

# Add Tiny GLTF
set(TINYGLTF_HEADER_ONLY ON CACHE INTERNAL "" FORCE)
set(TINYGLTF_BUILD_EXAMPLES OFF CACHE INTERNAL "" FORCE)
set(TINYGLTF_INSTALL OFF CACHE INTERNAL "" FORCE)
add_subdirectory(tinygltf EXCLUDE_FROM_ALL)

# Add Tiny OBJ loader
add_subdirectory(tinyobjloader EXCLUDE_FROM_ALL)

# Add Magic Enum
add_subdirectory(magic_enum EXCLUDE_FROM_ALL)

# Add Tracy Profiler
# available options: TRACY_ENABLE , TRACY_ON_DEMAND , TRACY_NO_BROADCAST , TRACY_NO_CODE_TRANSFER , ...
option(TRACY_ENABLE "" OFF)
#option(TRACY_ON_DEMAND "" ON)
add_subdirectory(tracy)

# Add Entropia File System Watcher
add_subdirectory(efsw)