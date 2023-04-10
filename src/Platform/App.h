#pragma once

#include "pch.h"

#include "Renderer/Common.h"

namespace chronicle {

class Systems;

class App {
public:
    void Run(const std::string_view& appName);

    [[nodiscard]] std::string AppName() const { return _appName; }
    [[nodiscard]] int Width() const { return _width; }
    [[nodiscard]] int Height() const { return _height; }
    [[nodiscard]] GLFWwindow* Window() const { return _window; }

private:
    GLFWwindow* _window = nullptr;
    std::string _appName;
    int _width;
    int _height;
    Systems* _systems = nullptr;

    static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);

    void Init();
    void MainLoop();
    void Destroy();
};

} // namespace chronicle