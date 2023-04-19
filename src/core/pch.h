// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

// renderer defines
#if RENDERER == VULKAN
#define VULKAN_RENDERER
#endif

// platform defines
#if PLATFORM == GLFW
#define GLFW_PLATFORM
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

// imgui
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

// magic enums
#include <magic_enum.hpp>

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

#define COMBINE1(X, Y) X##Y // helper macro
#define COMBINE(X, Y) COMBINE1(X, Y)

namespace chronicle {
constexpr tracy::Color::ColorType colorFromErrorLevel(spdlog::level::level_enum lvl)
{
    switch (lvl) {
    case spdlog::level::level_enum::trace:
        return tracy::Color::Gray;
    case spdlog::level::level_enum::debug:
        return tracy::Color::LightGray;
    case spdlog::level::level_enum::info:
        return tracy::Color::White;
    case spdlog::level::level_enum::warn:
        return tracy::Color::Orange;
    case spdlog::level::level_enum::err:
        return tracy::Color::Red;
    case spdlog::level::level_enum::critical:
        return tracy::Color::DarkRed;
    };
    return tracy::Color::White;
}

template <typename... Args>
constexpr void log(
    spdlog::source_loc source, spdlog::level::level_enum lvl, spdlog::format_string_t<Args...> fmt, Args&&... args)
{
    spdlog::log(source, lvl, fmt, std::forward<Args>(args)...);
    std::string message = fmt::format(fmt, std::forward<Args>(args)...);
    tracy::Profiler::MessageColor(message.c_str(), message.size(), colorFromErrorLevel(lvl), 0);
}
} // namespace chronicle

#define CHRLOG_INFO(...)                                                                                               \
    chronicle::log(spdlog::source_loc { __FILE__, __LINE__, SPDLOG_FUNCTION }, spdlog::level::info, __VA_ARGS__)
#define CHRLOG_WARN(...)                                                                                               \
    chronicle::log(spdlog::source_loc { __FILE__, __LINE__, SPDLOG_FUNCTION }, spdlog::level::warn, __VA_ARGS__)
#define CHRLOG_ERROR(...)                                                                                              \
    chronicle::log(spdlog::source_loc { __FILE__, __LINE__, SPDLOG_FUNCTION }, spdlog::level::err, __VA_ARGS__)

// #ifdef NDEBUG
// #define CHRLOG_DEBUG(...) (void)0
// #define CHRLOG_TRACE(...) (void)0
// #else
#define CHRLOG_DEBUG(...)                                                                                              \
    chronicle::log(spdlog::source_loc { __FILE__, __LINE__, SPDLOG_FUNCTION }, spdlog::level::debug, __VA_ARGS__)
#define CHRLOG_TRACE(...)                                                                                              \
    chronicle::log(spdlog::source_loc { __FILE__, __LINE__, SPDLOG_FUNCTION }, spdlog::level::trace, __VA_ARGS__)
// #endif

// trace
#ifdef NDEBUG
#define CHRZONE_PLATFORM (void)0
#define CHRZONE_RENDERER (void)0
#define CHRZONE_STORAGE (void)0
#define CHRZONE_ASSETS (void)0
#else
#define CHRZONE_PLATFORM ZoneScopedC(tracy::Color::Blue4)
#define CHRZONE_RENDERER ZoneScopedC(tracy::Color::Red4)
#define CHRZONE_STORAGE ZoneScopedC(tracy::Color::Green4)
#define CHRZONE_ASSETS ZoneScopedC(tracy::Color::Yellow4)
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
    }

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
