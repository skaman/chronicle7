#pragma once

// renderer defines
#if RENDERER == VULKAN
#define VULKAN_RENDERER
#endif

// vulkan hpp
#ifdef VULKAN_RENDERER
#include <vulkan/vulkan.hpp>
#endif

// glfw
#include <GLFW/glfw3.h>

// spdlog
#include <spdlog/spdlog.h>

// glm
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

// entt
#include <entt/entt.hpp>

// tiny gltf
#include <tiny_gltf.h>

// tiny obj loader
#include <tinyobjloader/tiny_obj_loader.h>

// tracy
#include <tracy/Tracy.hpp>

// stb
#include <stb/stb_image.h>

// std lib
#include <bit>
#include <chrono>
#include <fstream>
#include <iostream>
#include <optional>
#include <set>
#include <string>
#include <vector>

// logs
#ifdef NDEBUG
#define CHRLOG_DEBUG(...) (void)0
#define CHRLOG_INFO(...) (void)0
#define CHRLOG_WARN(...) (void)0
#define CHRLOG_ERROR(...) (void)0
#else
#define CHRLOG_DEBUG(...)                                                                                              \
    {                                                                                                                  \
        SPDLOG_DEBUG(__VA_ARGS__);                                                                                     \
        std::string message = fmt::format(__VA_ARGS__);                                                                \
        TracyMessage(message.c_str(), message.size());                                                                 \
    }

#define CHRLOG_INFO(...)                                                                                               \
    {                                                                                                                  \
        SPDLOG_INFO(__VA_ARGS__);                                                                                      \
        std::string message = fmt::format(__VA_ARGS__);                                                                \
        TracyMessage(message.c_str(), message.size());                                                                 \
    }

#define CHRLOG_WARN(...)                                                                                               \
    {                                                                                                                  \
        SPDLOG_WARN(__VA_ARGS__);                                                                                      \
        std::string message = fmt::format(__VA_ARGS__);                                                                \
        TracyMessage(message.c_str(), message.size());                                                                 \
    }

#define CHRLOG_ERROR(...)                                                                                              \
    {                                                                                                                  \
        SPDLOG_ERROR(__VA_ARGS__);                                                                                     \
        std::string message = fmt::format(__VA_ARGS__);                                                                \
        TracyMessage(message.c_str(), message.size());                                                                 \
    }
#endif

// trace
#ifdef NDEBUG
#define CHRZONE_PLATFORM
#define CHRZONE_VULKAN
#define CHRZONE_RENDERER_SYSTEM
#define CHRZONE_STORAGE
#else
#define CHRZONE_PLATFORM ZoneScopedC(tracy::Color::Blue4);
#define CHRZONE_VULKAN ZoneScopedC(tracy::Color::Red4);
#define CHRZONE_RENDERER_SYSTEM ZoneScopedC(tracy::Color::Green4);
#define CHRZONE_STORAGE ZoneScopedC(tracy::Color::Chocolate4);
#endif

// stringify
#define STRINGIFY(x) #x

// concrete for private constructors
#define CHR_CONCRETE(x)                                                                                                \
    class Concrete##x : public x {                                                                                     \
    public:                                                                                                            \
        template <typename... Args>                                                                                    \
        explicit Concrete##x(Args&&... args)                                                                           \
            : x(std::forward<Args>(args)...)                                                                           \
        {                                                                                                              \
        }                                                                                                              \
    };

namespace chronicle {

// non copyable classes
template <class T> class NonCopyable {
protected:
    constexpr NonCopyable() = default;
    ~NonCopyable() = default;

    // disallow copy constructor
    NonCopyable(const NonCopyable&) = delete;

    // disallow copy assignment
    NonCopyable& operator=(const NonCopyable&) = delete;
};

} // namespace chronicle
