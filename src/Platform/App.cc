#include "App.h"

#include "Renderer/Renderer.h"
#include "Systems/MeshRendererSystem.h"
#include "Systems/Systems.h"

namespace chronicle {

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

void App::Run(const std::string_view& appName)
{
    CHRZONE_PLATFORM

    _appName = appName;

    Init();
    MainLoop();
    Destroy();
}

void App::FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    auto app = std::bit_cast<App*>(glfwGetWindowUserPointer(window));
    app->_width = width;
    app->_height = height;
}

void App::Init()
{
    CHRZONE_PLATFORM

    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    _width = WIDTH;
    _height = HEIGHT;
    _window = glfwCreateWindow(WIDTH, HEIGHT, _appName.c_str(), nullptr, nullptr);

    glfwSetWindowUserPointer(_window, this);
    glfwSetFramebufferSizeCallback(_window, FramebufferSizeCallback);

    _renderer = &entt::locator<Renderer>::emplace<Renderer>(this);

    _systems = &entt::locator<Systems>::emplace<Systems>();
    _systems->Register<MeshRendererSystem>();
}

void App::MainLoop()
{
    CHRZONE_PLATFORM

    entt::registry registry {};

    while (!glfwWindowShouldClose(_window)) {
        glfwPollEvents();
        _systems->Run(registry);
    }

    _renderer->waitIdle();
}

void App::Destroy()
{
    CHRZONE_PLATFORM

    entt::locator<Systems>::reset();
    entt::locator<Renderer>::reset();

    glfwDestroyWindow(_window);
    glfwTerminate();
}

} // namespace chronicle