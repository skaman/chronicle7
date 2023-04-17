// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Platform/PlatformI.h"

#include "GLFWCommon.h"
#include "Platform/Common.h"

namespace chronicle {

class GLFWPlatform : public PlatformI<GLFWPlatform>, private NonCopyable<GLFWPlatform> {
public:
    static void init();
    static void deinit();
    static bool poll(double& delta);

    static float windowDpiScale();

    static void setWindowTitle(const std::string_view& title);
    static void setWindowSize(uint32_t width, uint32_t height);

    [[nodiscard]] static CursorMode cursorMode();
    static void setCursorMode(CursorMode cursorMode);

    [[nodiscard]] static CursorType cursorType();
    static void setCursorType(CursorType cursorType);

    [[nodiscard]] static std::pair<double, double> cursorPosition();
    [[nodiscard]] static ButtonAction mouseButton(MouseButton button);

    [[nodiscard]] static ButtonAction key(Key key);

    [[nodiscard]] static bool gamepadAvailable(Gamepad gamepad);
    [[nodiscard]] static GamepadData gamepadInput(Gamepad gamepad);
    [[nodiscard]] static std::optional<std::string> gamepadName(Gamepad gamepad);

    [[nodiscard]] static std::optional<std::string> clipboardString();
    static void setClipboardString(const std::string& value);

    [[nodiscard]] static std::pair<uint32_t, uint32_t> windowSize();
    [[nodiscard]] static entt::dispatcher& dispatcher() { return GLFWContext::dispatcher; }

private:
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void cursorEnterCallback(GLFWwindow* window, int entered);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void joystickCallback(int joystickId, int connectedStatus);
    static void characterCallback(GLFWwindow* window, unsigned int codepoint);
    static void dropCallback(GLFWwindow* window, int count, const char** paths);
};

} // namespace chronicle