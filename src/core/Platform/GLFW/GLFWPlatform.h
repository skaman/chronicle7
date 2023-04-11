#pragma once

#include "pch.h"

#include "Platform/PlatformI.h"

namespace chronicle {

class GLFWPlatform : public PlatformI<GLFWPlatform>, private NonCopyable<GLFWPlatform> {
public:
    static void init();
    static void deinit();
    static bool poll();

    static void setWindowTitle(const std::string_view& title);
    static void setWindowSize(uint32_t width, uint32_t height);

    [[nodiscard]] static std::pair<uint32_t, uint32_t> windowSize();

private:
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
};

} // namespace chronicle