// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Common.h"
#include "GLFW/GLFWCommon.h"

namespace chronicle {

template <class T> class PlatformI {
public:
    static void init() { T::init(); }
    static void deinit() { T::deinit(); }
    static bool poll(double& delta) { return T::poll(delta); }

    [[nodiscard]] static float windowDpiScale() { return T::windowDpiScale(); }

    static void setWindowTitle(const std::string_view& title) { T::setWindowTitle(title); }
    static void setWindowSize(uint32_t width, uint32_t height) { T::setWindowSize(width, height); }

    [[nodiscard]] static CursorMode cursorMode() { return T::cursorMode(); }
    static void setCursorMode(CursorMode cursorMode) { T::setCursorMode(cursorMode); }

    [[nodiscard]] static CursorType cursorType() { return T::cursorType(); }
    static void setCursorType(CursorType cursorType) { T::setCursorType(cursorType); }

    [[nodiscard]] static std::pair<double, double> cursorPosition() { return T::cursorPosition(); }
    [[nodiscard]] static ButtonAction mouseButton(MouseButton button) { return T::mouseButton(button); }

    [[nodiscard]] static ButtonAction key(Key key) { return T::key(key); }

    [[nodiscard]] static bool gamepadAvailable(Gamepad gamepad) { return T::gamepadAvailable(gamepad); }
    [[nodiscard]] static GamepadData gamepadInput(Gamepad gamepad) { return T::gamepadInput(gamepad); }
    [[nodiscard]] static std::optional<std::string> gamepadName(Gamepad gamepad) { return T::gamepadName(gamepad); }

    [[nodiscard]] static std::optional<std::string> clipboardString() { return T::clipboardString(); }
    static void setClipboardString(const std::string& value) { T::setClipboardString(value); }

    [[nodiscard]] static std::pair<uint32_t, uint32_t> windowSize() { return T::windowSize(); }
    [[nodiscard]] static entt::dispatcher& dispatcher() { return T::dispatcher(); }

private:
    PlatformI() = default;
    friend T;
};

} // namespace chronicle