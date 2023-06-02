// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include <spdlog/spdlog.h>

namespace chronicle
{

template <typename... Args>
constexpr void log(spdlog::source_loc source, spdlog::level::level_enum lvl, spdlog::format_string_t<Args...> fmt,
                   Args &&...args)
{
    spdlog::log(source, lvl, fmt, std::forward<Args>(args)...);
#ifdef TRACY_ENABLE
    std::string message = fmt::format(fmt, std::forward<Args>(args)...);
    tracy::Profiler::MessageColor(message.c_str(), message.size(), colorFromErrorLevel(lvl), 0);
#endif
}

} // namespace chronicle

#define CHRLOG_INFO(...)                                                                                               \
    chronicle::log(spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, spdlog::level::info, __VA_ARGS__)
#define CHRLOG_WARN(...)                                                                                               \
    chronicle::log(spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, spdlog::level::warn, __VA_ARGS__)
#define CHRLOG_ERROR(...)                                                                                              \
    chronicle::log(spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, spdlog::level::err, __VA_ARGS__)

#ifdef NDEBUG
#define CHRLOG_DEBUG(...) (void)0
#define CHRLOG_TRACE(...) (void)0
#else
#define CHRLOG_DEBUG(...)                                                                                              \
    chronicle::log(spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, spdlog::level::debug, __VA_ARGS__)
#define CHRLOG_TRACE(...)                                                                                              \
    chronicle::log(spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, spdlog::level::trace, __VA_ARGS__)
#endif