// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "GLFWPlatform.h"

#include "Renderer/Renderer.h"

namespace chronicle {

void GLFWPlatform::init()
{
    CHRZONE_PLATFORM;

    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    GLFWContext::window
        = glfwCreateWindow(GLFWContext::width, GLFWContext::height, GLFWContext::title.c_str(), nullptr, nullptr);

    //glfwSetWindowAttrib(GLFWContext::window, GLFW_TITLEBAR, GLFW_FALSE);
    glfwSetInputMode(GLFWContext::window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);

    glfwSetFramebufferSizeCallback(GLFWContext::window, framebufferSizeCallback);
    glfwSetCursorPosCallback(GLFWContext::window, cursorPositionCallback);
    glfwSetMouseButtonCallback(GLFWContext::window, mouseButtonCallback);
    glfwSetCursorEnterCallback(GLFWContext::window, cursorEnterCallback);
    glfwSetScrollCallback(GLFWContext::window, scrollCallback);
    glfwSetJoystickCallback(joystickCallback);
    glfwSetCharCallback(GLFWContext::window, characterCallback);
    glfwSetDropCallback(GLFWContext::window, dropCallback);
}

void GLFWPlatform::deinit()
{
    CHRZONE_PLATFORM;

    if (GLFWContext::cursor != nullptr) {
        glfwSetCursor(GLFWContext::window, nullptr);
        glfwDestroyCursor(GLFWContext::cursor);
    }

    glfwDestroyWindow(GLFWContext::window);
    glfwTerminate();
}

bool GLFWPlatform::poll(double& delta)
{
    CHRZONE_PLATFORM;

    if (glfwWindowShouldClose(GLFWContext::window))
        return false;

    glfwPollEvents();

    static double lastTime = 0;
    auto time = glfwGetTime();
    delta = time - lastTime;
    lastTime = time;

    GLFWContext::dispatcher.update();

    return true;
}

float GLFWPlatform::windowDpiScale()
{
    float xscale;
    float yscale;
    auto* monitor = glfwGetPrimaryMonitor();
    glfwGetMonitorContentScale(monitor, &xscale, &yscale);

    return std::max(xscale, yscale);
}

void GLFWPlatform::setWindowTitle(const std::string_view& title) { GLFWContext::title = title; }

void GLFWPlatform::setWindowSize(uint32_t width, uint32_t height)
{
    GLFWContext::width = width;
    GLFWContext::height = height;
}

CursorMode GLFWPlatform::cursorMode()
{
    CHRZONE_PLATFORM;

    auto inputMode = glfwGetInputMode(GLFWContext::window, GLFW_CURSOR);
    switch (inputMode) {
    case GLFW_CURSOR_HIDDEN:
        return CursorMode::hidden;
    case GLFW_CURSOR_DISABLED:
        return CursorMode::disabled;
    case GLFW_CURSOR_NORMAL:
    default:
        return CursorMode::normal;
    }
}

void GLFWPlatform::setCursorMode(CursorMode cursorMode)
{
    CHRZONE_PLATFORM;

    switch (cursorMode) {
    case chronicle::CursorMode::normal:
        glfwSetInputMode(GLFWContext::window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        if (glfwRawMouseMotionSupported())
            glfwSetInputMode(GLFWContext::window, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
        break;
    case chronicle::CursorMode::hidden:
        glfwSetInputMode(GLFWContext::window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
        if (glfwRawMouseMotionSupported())
            glfwSetInputMode(GLFWContext::window, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
        break;
    case chronicle::CursorMode::disabled:
        glfwSetInputMode(GLFWContext::window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        if (glfwRawMouseMotionSupported())
            glfwSetInputMode(GLFWContext::window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        break;
    default:
        break;
    }
}

CursorType GLFWPlatform::cursorType() { return GLFWContext::cursorType; }

void GLFWPlatform::setCursorType(CursorType cursorType)
{
    CHRZONE_PLATFORM;

    auto* oldCursor = GLFWContext::cursor;

    switch (cursorType) {
    case chronicle::CursorType::arrow:
        GLFWContext::cursor = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
        break;
    case chronicle::CursorType::ibeam:
        GLFWContext::cursor = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
        break;
    case chronicle::CursorType::crosshair:
        GLFWContext::cursor = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
        break;
    case chronicle::CursorType::hand:
        GLFWContext::cursor = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
        break;
    case chronicle::CursorType::hresize:
        GLFWContext::cursor = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
        break;
    case chronicle::CursorType::vresize:
        GLFWContext::cursor = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
        break;
    default:
        GLFWContext::cursor = nullptr;
        break;
    }

    glfwSetCursor(GLFWContext::window, GLFWContext::cursor);
    if (oldCursor != nullptr) {
        glfwDestroyCursor(oldCursor);
    }

    GLFWContext::cursorType = cursorType;
}

std::pair<double, double> GLFWPlatform::cursorPosition()
{
    double xpos;
    double ypos;
    glfwGetCursorPos(GLFWContext::window, &xpos, &ypos);
    return std::pair<double, double>(xpos, ypos);
}

ButtonAction GLFWPlatform::mouseButton(MouseButton button)
{
    int state = glfwGetMouseButton(GLFWContext::window, static_cast<int>(button));
    return static_cast<ButtonAction>(state);
}

ButtonAction GLFWPlatform::key(Key key)
{
    int state = glfwGetKey(GLFWContext::window, static_cast<int>(key));
    return static_cast<ButtonAction>(state);
}

bool GLFWPlatform::gamepadAvailable(Gamepad gamepad) { return glfwJoystickIsGamepad(static_cast<int>(gamepad)); }

GamepadData GLFWPlatform::gamepadInput(Gamepad gamepad)
{
    GamepadData data {};

    if (GLFWgamepadstate state; glfwGetGamepadState(static_cast<int>(gamepad), &state)) {
        memcpy(data.buttons.data(), state.buttons, data.buttons.size());
        memcpy(data.axes.data(), state.axes, data.axes.size());
    }

    return data;
}

std::optional<std::string> GLFWPlatform::gamepadName(Gamepad gamepad)
{
    auto* name = glfwGetGamepadName(static_cast<int>(gamepad));
    if (name == nullptr)
        return {};
    return name;
}

std::optional<std::string> GLFWPlatform::clipboardString()
{
    auto* value = glfwGetClipboardString(nullptr);
    if (value == nullptr)
        return {};
    return value;
}

void GLFWPlatform::setClipboardString(const std::string& value) { glfwSetClipboardString(nullptr, value.c_str()); }

std::pair<uint32_t, uint32_t> GLFWPlatform::windowSize()
{
    return std::pair<uint32_t, uint32_t>(GLFWContext::width, GLFWContext::height);
}

void GLFWPlatform::framebufferSizeCallback([[maybe_unused]] GLFWwindow* window, int width, int height)
{
    GLFWContext::width = width;
    GLFWContext::height = height;

    Renderer::invalidateSwapChain();
}

void GLFWPlatform::cursorPositionCallback([[maybe_unused]] GLFWwindow* window, double xpos, double ypos)
{
    CursorPositionEvent evn = { .data = { .positionX = xpos, .positionY = ypos } };
    GLFWContext::dispatcher.enqueue<CursorPositionEvent>(evn);
}

void GLFWPlatform::mouseButtonCallback([[maybe_unused]] GLFWwindow* window, int button, int action, int mods)
{
    MouseButtonEvent evn = { .data = { .button = static_cast<MouseButton>(button),
                                 .action = static_cast<ButtonAction>(action),
                                 .modifier = static_cast<KeyModifier>(mods) } };
    GLFWContext::dispatcher.enqueue<MouseButtonEvent>(evn);
}

void GLFWPlatform::cursorEnterCallback([[maybe_unused]] GLFWwindow* window, int entered)
{
    MouseEnterEvent evn = { .data = { .entered = entered != 0 } };
    GLFWContext::dispatcher.enqueue<MouseEnterEvent>(evn);
}

void GLFWPlatform::scrollCallback([[maybe_unused]] GLFWwindow* window, double xoffset, double yoffset)
{
    MouseScrollEvent evn = { .data = { .offsetX = xoffset, .offsetY = yoffset } };
    GLFWContext::dispatcher.enqueue<MouseScrollEvent>(evn);
}

void GLFWPlatform::keyCallback([[maybe_unused]] GLFWwindow* window, int key, int scancode, int action, int mods)
{
    KeyEvent evn = { .data = { .key = static_cast<Key>(key),
                         .action = static_cast<ButtonAction>(action),
                         .modifier = static_cast<KeyModifier>(mods) } };
    GLFWContext::dispatcher.enqueue<KeyEvent>(evn);
}

void GLFWPlatform::joystickCallback(int joystickId, int connectedStatus)
{
    GamepadConnectedEvent evn
        = { .data = { .gamepad = static_cast<Gamepad>(joystickId), .connected = connectedStatus == GLFW_CONNECTED } };
    GLFWContext::dispatcher.enqueue<GamepadConnectedEvent>(evn);
}

void GLFWPlatform::characterCallback([[maybe_unused]] GLFWwindow* window, unsigned int codepoint)
{
    CharEvent evn = { .data = { .character = static_cast<wchar_t>(codepoint) } };
    GLFWContext::dispatcher.enqueue<CharEvent>(evn);
}

void GLFWPlatform::dropCallback([[maybe_unused]] GLFWwindow* window, int count, const char** paths)
{
    PathDropEvent evn = {};
    evn.data.paths.reserve(count);
    for (int i = 0; i < count; i++) {
        evn.data.paths.emplace_back(paths[i]);
    }
    GLFWContext::dispatcher.enqueue<PathDropEvent>(evn);
}

} // namespace chronicle