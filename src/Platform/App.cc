#include "App.h"

#include "Locator.h"

// #include "Renderer/Renderer.h"
#include "Renderer/Renderer.h"
#include "Systems/MeshRenderSystem.h"
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

    Renderer::init(this);
    Locator::systems = std::make_unique<Systems>();

    _systems = Locator::systems.get();
    _systems->add<MeshRenderSystem>();
}

void App::MainLoop()
{
    CHRZONE_PLATFORM

    entt::registry registry {};

    while (!glfwWindowShouldClose(_window)) {
        glfwPollEvents();
        _systems->run(registry);
    }

    Renderer::waitIdle();
}

void App::Destroy()
{
    CHRZONE_PLATFORM

    Locator::systems.reset();
    Renderer::deinit();

    glfwDestroyWindow(_window);
    glfwTerminate();
}

} // namespace chronicle