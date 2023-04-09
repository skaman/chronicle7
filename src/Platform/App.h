#pragma once

#include "pch.h"

#include "Renderer/Common.h"
#include "Renderer/Vulkan/Vulkan.h"

namespace chronicle {

class Systems;

class App {
public:
    void Run(const std::string_view& appName);

    [[nodiscard]] inline std::string AppName() const { return _appName; }
    [[nodiscard]] inline int Width() const { return _width; }
    [[nodiscard]] inline int Height() const { return _height; }
    [[nodiscard]] inline GLFWwindow* Window() const { return _window; }

private:
    GLFWwindow* _window = nullptr;
    std::string _appName;
    int _width;
    int _height;
    Renderer* _renderer = nullptr;
    Systems* _systems = nullptr;

    static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);

    void Init();
    void MainLoop();
    void Destroy();
};

} // namespace chronicle