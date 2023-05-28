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

// disable a bunch of intellisense warning that we don't want for external libraries
#pragma warning(push)
#pragma warning(disable : 6201)
#pragma warning(disable : 6285)
#pragma warning(disable : 26437)
#pragma warning(disable : 26451)
#pragma warning(disable : 26495)
#pragma warning(disable : 26498)

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
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>

// entt
#define ENTT_USE_ATOMIC
#include <entt/entt.hpp>

// tiny gltf
#include <tiny_gltf.h>

// tiny obj loader
#include <tinyobjloader/tiny_obj_loader.h>

// tracy
#include <tracy/Tracy.hpp>

// stb
#include <stb/stb_image.h>

// efsw
#include <efsw/include/efsw/efsw.hpp>

// imgui
#define IM_VEC2_CLASS_EXTRA                                                                                            \
    constexpr ImVec2(glm::vec2& f)                                                                                     \
        : x(f.x)                                                                                                       \
        , y(f.y)                                                                                                       \
    {                                                                                                                  \
    }                                                                                                                  \
    operator glm::vec2() const                                                                                         \
    {                                                                                                                  \
        return glm::vec2(x, y);                                                                                        \
    }

#define IM_VEC4_CLASS_EXTRA                                                                                            \
    constexpr ImVec4(const glm::vec4& f)                                                                               \
        : x(f.x)                                                                                                       \
        , y(f.y)                                                                                                       \
        , z(f.z)                                                                                                       \
        , w(f.w)                                                                                                       \
    {                                                                                                                  \
    }                                                                                                                  \
    operator glm::vec4() const                                                                                         \
    {                                                                                                                  \
        return glm::vec4(x, y, z, w);                                                                                  \
    }
#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

// magic enums
#include <magic_enum.hpp>

// restore warnings
#pragma warning(pop)

// std lib
#include <bit>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <regex>
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
    default:
        return tracy::Color::White;
    };
}

template <typename... Args>
constexpr void log(
    spdlog::source_loc source, spdlog::level::level_enum lvl, spdlog::format_string_t<Args...> fmt, Args&&... args)
{
    spdlog::log(source, lvl, fmt, std::forward<Args>(args)...);
#ifdef TRACY_ENABLE
    std::string message = fmt::format(fmt, std::forward<Args>(args)...);
    tracy::Profiler::MessageColor(message.c_str(), message.size(), colorFromErrorLevel(lvl), 0);
#endif
}
} // namespace chronicle

#define CHRLOG_INFO(...)                                                                                               \
    chronicle::log(spdlog::source_loc { __FILE__, __LINE__, SPDLOG_FUNCTION }, spdlog::level::info, __VA_ARGS__)
#define CHRLOG_WARN(...)                                                                                               \
    chronicle::log(spdlog::source_loc { __FILE__, __LINE__, SPDLOG_FUNCTION }, spdlog::level::warn, __VA_ARGS__)
#define CHRLOG_ERROR(...)                                                                                              \
    chronicle::log(spdlog::source_loc { __FILE__, __LINE__, SPDLOG_FUNCTION }, spdlog::level::err, __VA_ARGS__)

#ifdef NDEBUG
#define CHRLOG_DEBUG(...) (void)0
#define CHRLOG_TRACE(...) (void)0
#else
#define CHRLOG_DEBUG(...)                                                                                              \
    chronicle::log(spdlog::source_loc { __FILE__, __LINE__, SPDLOG_FUNCTION }, spdlog::level::debug, __VA_ARGS__)
#define CHRLOG_TRACE(...)                                                                                              \
    chronicle::log(spdlog::source_loc { __FILE__, __LINE__, SPDLOG_FUNCTION }, spdlog::level::trace, __VA_ARGS__)
#endif

// trace
#ifdef TRACY_ENABLE
#define CHRZONE_PLATFORM ZoneScopedC(tracy::Color::Blue4)
#define CHRZONE_RENDERER ZoneScopedC(tracy::Color::Red4)
#define CHRZONE_STORAGE ZoneScopedC(tracy::Color::Green4)
#define CHRZONE_ASSETS ZoneScopedC(tracy::Color::Yellow4)
#define CHRZONE_SCENE ZoneScopedC(tracy::Color::Cyan4)
#else
#define CHRZONE_PLATFORM (void)0
#define CHRZONE_RENDERER (void)0
#define CHRZONE_STORAGE (void)0
#define CHRZONE_ASSETS (void)0
#define CHRZONE_SCENE (void)0
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

// CRTP helpers
#define CRTP_CONST_THIS static_cast<const T*>(this)
#define CRTP_THIS static_cast<T*>(this)

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

// vector to strings
std::string join(const std::vector<std::string>& data);

} // namespace chronicle

// hashable
// https://stackoverflow.com/questions/2590677/how-do-i-combine-hash-values-in-c0x
namespace std {
inline void hash_combine([[maybe_unused]] const std::size_t& seed)
{
    // nothing to do, this is just the end of life for the hash_combine template.
}

template <typename T, typename... Rest> inline void hash_combine(std::size_t& seed, const T& v, Rest... rest)
{
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    std::hash_combine(seed, rest...);
}

inline void hash_add([[maybe_unused]] std::size_t& seed, std::size_t value)
{
    seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

} // namespace std