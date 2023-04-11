#include "GLFWPlatform.h"

#include "GLFWCommon.h"
#include "Renderer/Renderer.h"

namespace chronicle {

void GLFWPlatform::init()
{
    CHRZONE_PLATFORM

    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    GLFWContext::window
        = glfwCreateWindow(GLFWContext::width, GLFWContext::height, GLFWContext::title.c_str(), nullptr, nullptr);

    glfwSetFramebufferSizeCallback(GLFWContext::window, framebufferSizeCallback);
}

void GLFWPlatform::deinit()
{
    CHRZONE_PLATFORM

    glfwDestroyWindow(GLFWContext::window);
    glfwTerminate();
}

bool GLFWPlatform::poll()
{
    CHRZONE_PLATFORM

    if (glfwWindowShouldClose(GLFWContext::window))
        return false;

    glfwPollEvents();
    return true;
}

void GLFWPlatform::setWindowTitle(const std::string_view& title) { GLFWContext::title = title; }

void GLFWPlatform::setWindowSize(uint32_t width, uint32_t height)
{
    GLFWContext::width = width;
    GLFWContext::height = height;
}

std::pair<uint32_t, uint32_t> GLFWPlatform::windowSize()
{
    return std::pair<uint32_t, uint32_t>(GLFWContext::width, GLFWContext::height);
}

void GLFWPlatform::framebufferSizeCallback([[maybe_unused]] GLFWwindow* window, int width, int height)
{
    GLFWContext::width = width;
    GLFWContext::height = height;

    // TODO: invalidate renderer swapchain
}

} // namespace chronicle